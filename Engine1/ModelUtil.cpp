#include "ModelUtil.h"

#include <tuple>
#include <algorithm>
#include <array>
#include <memory>

#include "BlockModel.h"

#include "Texture2DGeneric.h"
#include "TextureUtil.h"
#include "MeshUtil.h"

using namespace Engine1;

std::shared_ptr< BlockModel > ModelUtil::mergeModels( 
    const std::vector< std::shared_ptr< BlockModel > >& models, 
    const std::vector< float43 >& transforms, 
    ID3D11Device3& device )
{
    // We assume that there is one texture of each type (albedo, roughness etc) for each texcoord in each model. Or there may be zero texture of given type for all models.

    // ------ NEW --------------------
    // 1. Check that all models have textures of the same types provided.
    // 2. Treat textures in sets - one set per model. Gather sets for all models. Each model references its set.
    // 3. Remove duplicated sets - re-index models to sets.
    // 4. For each set - find the proportions of the texture (max of all types), max dimensions etc. Save it as texture set properties.
    // 5. Decide on texture placements (texcoords) based on texture set properties (not just one type of textures - as they may have different dimensions).
    // 6. For each texture type, iterate over all sets and calculate what is the needed merged texture dimension (per type) to fit textures in their placements.
    // 7. Create a texture of needed dimensions for each type.
    // 8. Copy/re-size textures into their placements.

    // #TODO: Adding a border of a few pixels to each sub-texture. Otherwise, textures blend with each other because of bilinear filtering or higher level mipmap sampling during rendering.

    if ( !transforms.empty() && transforms.size() != models.size() )
        throw std::exception( "ModelUtil::mergeModels - different number of transforms passed compared to the number of models." );
    // -------------------------------

    // Check that all models have meshes and power-of-two textures.
    std::string error;

    for ( auto& model : models )
    {
        if ( !model->getMesh() )
            error += "ModelUtil::mergeModels - some models don't have a mesh.\n";

        // Check if all textures have power-of-two dimensions.
        // #TODO: This limitation should be removed in the future.
        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
        {
            const auto texType = (Model::TextureType)textureType;

            const auto textures = model->getTextures( texType );
            for ( const auto& texture : textures )
            {
                const auto& tex = std::get< 0 >( texture );

                if ( !tex )
                    continue;

                int2 dimensions( int2::ZERO );
                if ( texType == Model::TextureType::Alpha
                    || texType == Model::TextureType::Metalness
                    || texType == Model::TextureType::Roughness
                    || texType == Model::TextureType::RefractiveIndex )
                {
                    auto& texU = static_cast< Texture2DGeneric< unsigned char >& >( *std::get< 0 >( texture ) );

                    dimensions = texU.getDimensions();
                }
                else
                {
                    auto& texU4 = static_cast< Texture2DGeneric< uchar4 >& >( *std::get< 0 >( texture ) );

                    dimensions = texU4.getDimensions();
                }

                if ( !MathUtil::isPowerOfTwo( dimensions.x ) || !MathUtil::isPowerOfTwo( dimensions.y ) )
                {
                    std::string path = std::get< 0 >( texture )->getFileInfo().getPath();

                    error += "ModelUtil::mergeModels - the texture does not have power-of-two dimensions: " + path + "\n";
                }
            }
        }
    }

    if ( !error.empty() )
        throw std::exception( error.c_str() );

    // Each set stores textures of all types for a single model (or many - if they share the same set).
    std::vector< TextureSet > textureSets;
    textureSets.reserve( models.size() );

    // Texture-set index for each model. If two models share the same set, they have the same index.
    std::vector< int > modelToTextureSetMapping;
    modelToTextureSetMapping.reserve( models.size() );

    // Check that all models have textures of the same types provided.
    std::array< int, (int)Model::TextureType::COUNT > textureCountPerType = { 0 };
    for ( auto& model : models )
    {
        modelToTextureSetMapping.push_back( (int)textureSets.size() );
        textureSets.emplace_back();
        auto& textureSet = textureSets.back();

        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
        {
            const int textureCount = model->getTextureCount( (Model::TextureType)textureType );

            if ( textureCount > 0 )
            {
                textureCountPerType[ textureType ] += 1;

                // Add a texture and it's color multipliers to the model's set.
                textureSet.m_textures[ textureType ]         = model->getTexture( (Model::TextureType)textureType );
                textureSet.m_colorMultipliers[ textureType ] = model->getTextureColorMultiplier( (Model::TextureType)textureType );
            }
        }
    }

    // Check if count of each texture type is the same (no texture is missing).
    std::string errorDescription;
    const int maxTextureCount = *std::max_element( textureCountPerType.begin(), textureCountPerType.end() );
    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
    {
        if ( textureCountPerType[ textureType ] > 0 && textureCountPerType[ textureType ] < maxTextureCount )
        {
            errorDescription = "Some models are missing " 
                + Model::textureTypeToString( (Model::TextureType)textureType ) + " textures:";

            // Find models which are missing textures of given type.
            for ( auto& model : models )
            {
                if ( model->getTextureCount( (Model::TextureType)textureType ) == 0 )
                {
                    // Get model or mesh file path.
                    auto path = model->getFileInfo().getPath();
                    if ( path.empty() ) 
                        path = model->getMesh()->getFileInfo().getPath();

                    errorDescription += "\n" + path;
                }
            }

            errorDescription += "\n\n";
        }
    }

    if ( !errorDescription.empty() )
        throw std::exception( "ModelUtil::mergeModels - errors during merge: \n" );

    // Remove duplicated sets - re-index model-to-texture-set mapping if needed.
    for ( int setIdx1 = 0; setIdx1 < textureSets.size(); ++setIdx1 )
    {
        for ( int setIdx2 = setIdx1 + 1; setIdx2 < textureSets.size(); )
        {
            if ( textureSets[ setIdx1 ] == textureSets[ setIdx2 ] )
            {
                // Erase the duplicate set.
                textureSets.erase( textureSets.begin() + setIdx2 );

                // Re-index all models referencing the removed set to the remaining copy of it.
                for ( int mapIdx = setIdx1 + 1; mapIdx < modelToTextureSetMapping.size(); ++mapIdx )
                {
                    if ( modelToTextureSetMapping[ mapIdx ] == setIdx2 )
                        modelToTextureSetMapping[ mapIdx ] = setIdx1;
                    else if ( modelToTextureSetMapping[ mapIdx ] > setIdx2 )
                        modelToTextureSetMapping[ mapIdx ] -= 1;
                }
            }
            else
                ++setIdx2;
        }
    }

    // Calculate dimensions of each set, which are required to store 
    // the set without loosing resolution for any type of texture.
    std::vector< int2 > textureSetDimensions;
    textureSetDimensions.reserve( textureSets.size() );
    for ( auto& textureSet : textureSets )
    {
        textureSet.calculateDimensions();

        textureSetDimensions.push_back( textureSet.m_dimensions );
    }

    int2                                       mergedTextureDimensions;
    std::vector< std::pair< float2, float2 > > mergedTexturesTexcoords;

    // Calculate where textures need to be placed within the merged texture.
    std::tie( mergedTextureDimensions, mergedTexturesTexcoords ) 
        = TextureUtil::prepareTextureMerge( textureSetDimensions );

    std::array< std::shared_ptr< Asset >, (int)Model::TextureType::COUNT > mergedTextures;

    // For each texture type, iterate over all sets and calculate 
    // what are the needed merged texture dimensions (per type) 
    // to fit textures in their texcoord range.
    // Then, merge the textures of given type.
    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
    {
        const auto texType = (Model::TextureType)textureType;

        int2 neededDimensions( int2::ZERO );
        for ( int textureSetIdx = 0; textureSetIdx < textureSets.size(); ++textureSetIdx )
        {
            const auto& textureSet    = textureSets[ textureSetIdx ];
            const int2  dimensions    = textureSet.getTextureDimensions( texType );
            const auto& texcoords     = mergedTexturesTexcoords[ textureSetIdx ];
            const auto  texcoordsSpan = texcoords.second - texcoords.first;

            neededDimensions.x = std::max( (int)((float)dimensions.x * ( 1.0f / texcoordsSpan.x )), neededDimensions.x );
            neededDimensions.y = std::max( (int)((float)dimensions.y * ( 1.0f / texcoordsSpan.y )), neededDimensions.y );
        }

        // Gather textures to be merged.
        std::vector< std::shared_ptr< Asset > > texturesToMerge;
        std::vector< float4 >                   colorMultipliersToMerge;
        texturesToMerge.reserve( textureSets.size() );
        colorMultipliersToMerge.reserve( textureSets.size() );
        for ( auto& textureSet : textureSets )
        {
            auto& texture = textureSet.m_textures[ textureType ];

            if ( texture )
            {
                texturesToMerge.push_back( texture );
                colorMultipliersToMerge.push_back( textureSet.m_colorMultipliers[ textureType ] );
            }
        }

        if ( texturesToMerge.empty() )
            continue;

        // Merge textures.
        if ( texType == Model::TextureType::Alpha || texType == Model::TextureType::Metalness ||
             texType == Model::TextureType::Roughness || texType == Model::TextureType::RefractiveIndex ) 
        {
            auto texturesToMergeU 
                = reinterpret_cast< std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > >& >( texturesToMerge );

            mergedTextures[ textureType ] = TextureUtil::mergeTextures( 
                texturesToMergeU, 
                colorMultipliersToMerge,
                mergedTexturesTexcoords, 
                neededDimensions,
                device, 
                DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM 
            );

            // Save temporary merged result - to help in debugging.
            auto tex = std::dynamic_pointer_cast< Texture2DGeneric< unsigned char > >( mergedTextures[ textureType ] );
            assert(tex);
            tex->saveToFile( "Temp/merged_temp_" + Model::textureTypeToString( texType ) + ".tif", Texture2DFileInfo::Format::TIFF );
        } 
        else 
        {
            auto texturesToMergeU4 
                = reinterpret_cast< std::vector< std::shared_ptr< Texture2DGeneric< uchar4 > > >& >( texturesToMerge );

            mergedTextures[ textureType ] = TextureUtil::mergeTextures( 
                texturesToMergeU4, 
                colorMultipliersToMerge,
                mergedTexturesTexcoords, 
                neededDimensions,
                device, 
                DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM 
            );

            // Save temporary merged result - to help in debugging.
            auto tex = std::dynamic_pointer_cast< Texture2DGeneric< uchar4 > >( mergedTextures[ textureType ] );
            assert(tex);
            tex->saveToFile( "Temp/merged_temp_" + Model::textureTypeToString( texType ) + ".png", Texture2DFileInfo::Format::PNG );
        }
    }

    // Gather meshes to be merged.
    std::vector< std::shared_ptr< BlockMesh > > meshes;
    meshes.reserve( models.size() );
    for ( auto& model : models )
        meshes.push_back( model->getMesh() );

    // Merge the meshes.
    std::shared_ptr< BlockMesh > mergedMesh = MeshUtil::mergeMeshes( meshes, transforms );

    { // Recalculate merged mesh texcoords to match the merged textures.
        std::vector< float2 > meshTexcoordOffsets;
        meshTexcoordOffsets.reserve( meshes.size() );

        // Check if all texcoords for a given source mesh are zeros -
        // apply (0.5, 0.5) offset then.
        // Without such offset texcoords after merge would end up in the top-left corner 
        // of the input textures in the atlas. That would cause unwanted blending between them when using bilinear sampling.
        for ( const auto& mesh : meshes )
        {
            bool allTexcoordsAreZeros = true;

            for ( const auto& texcoord : mesh->getTexcoords() )
            {
                if ( !MathUtil::areEqual( texcoord, float2::ZERO, 0.0f, 0.0001f ) ) 
                {
                    allTexcoordsAreZeros = false;
                    break;
                }
            }

            meshTexcoordOffsets.push_back( 
                allTexcoordsAreZeros ? float2::HALF : float2::ZERO
            );
        }

        std::vector< float2 >& mergedMeshTexcoords = mergedMesh->getTexcoords( 0 );

        int vertexStartIndex = 0, vertexEndIndex = 0;

        for ( int modelIndex = 0; modelIndex < models.size(); ++modelIndex ) 
        {
            const auto& mesh                = models[ modelIndex ]->getMesh();
            const auto& extraTexcoordOffset = meshTexcoordOffsets[ modelIndex ];
            const auto& mergedTexcoords     = mergedTexturesTexcoords[ modelToTextureSetMapping[ modelIndex ] ];

            vertexEndIndex += (int)mesh->getVertices().size();
            // Recalculate texcoords.
            for ( int vertexIndex = vertexStartIndex; vertexIndex < vertexEndIndex; ++vertexIndex )
            {
                float2 texcoord = mergedMeshTexcoords[ vertexIndex ];

                // Wrap UVs to 0-1 range (original ones could be negative, or greater than 1, less then -1).
                texcoord.x = fmod( texcoord.x, 1.0f );
                texcoord.y = fmod( texcoord.y, 1.0f );

                if (texcoord.x < 0.0f)
                    texcoord.x += 1.0f;

                if (texcoord.y < 0.0f)
                    texcoord.y += 1.0f;

                mergedMeshTexcoords[ vertexIndex ] 
                    = mergedTexcoords.first + ( texcoord + extraTexcoordOffset ) * (mergedTexcoords.second - mergedTexcoords.first);

                // Debug test.
                //static_cast< Texture2DGeneric< uchar4 >& >( *mergedTextures[ (int)Model::TextureType::Albedo ] ).setDataPixel( 
                //    mergedMeshTexcoords[ vertexIndex ], uchar4( 255, 0, 0, 255 ), 0 );
            }

            vertexStartIndex = vertexEndIndex;
        }
    }

    std::shared_ptr< BlockModel > mergedModel = std::make_shared< BlockModel >();
    { // Create merged model.
        mergedModel->setMesh( mergedMesh );

        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) 
        {
            if ( mergedTextures[ textureType ] )
                mergedModel->addTexture( ( Model::TextureType )textureType, mergedTextures[ textureType ], 0 );
        }
    }

    return mergedModel;
}

std::string ModelUtil::getDescription( const BlockModel& model, const bool printPath, const bool printDimensions )
{
    std::string text;

    if ( !model.getFileInfo().getPath().empty() )
        text += "model: " + model.getFileInfo().getPath() + "\n";
    else
        text += "model: \n";

    if ( !model.getAlphaTextures().empty() )
         text += "alpha: " + std::to_string( model.getAlphaTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getAlphaTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getEmissiveTextures().empty() )
        text += "emissive: " + std::to_string( model.getEmissiveTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getEmissiveTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getAlbedoTextures().empty() )
        text += "albedo: " + std::to_string( model.getAlbedoTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getAlbedoTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getMetalnessTextures().empty() )
        text += "metalness: " + std::to_string( model.getMetalnessTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getMetalnessTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getRoughnessTextures().empty() )
        text += "roughness: " + std::to_string( model.getRoughnessTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getRoughnessTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getNormalTextures().empty() )
        text += "normal: " + std::to_string( model.getNormalTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getNormalTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( !model.getRefractiveIndexTextures().empty() )
        text += "normal: " + std::to_string( model.getRefractiveIndexTextures().size() ) + "\n";

    for ( auto& modelTexture : model.getRefractiveIndexTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    text += "\n";

    return text;
}

ModelUtil::TextureSet::TextureSet() :
    m_dimensions( float2::ZERO )
{
    m_textures         = {};
    m_colorMultipliers = {};
}

bool ModelUtil::TextureSet::operator == (TextureSet& other) const
{
    if ( m_dimensions != other.m_dimensions )
        return false;

    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
    {
        // Return false if textures or color multipliers are different.
        if ( m_textures[ textureType ] != other.m_textures[ textureType ] )
        {
            return false;
        }
        else if ( !MathUtil::areEqual( m_colorMultipliers[ textureType ], other.m_colorMultipliers[ textureType ], 0.0f, 0.001f ) )
        {
            return false;
        }
    }

    return true;
}

void ModelUtil::TextureSet::calculateDimensions()
{
    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
    {
        auto texType = (Model::TextureType)textureType;

        if ( !m_textures[ textureType ] )
            continue;

        if ( texType == Model::TextureType::Alpha
            || texType == Model::TextureType::Metalness
            || texType == Model::TextureType::Roughness
            || texType == Model::TextureType::RefractiveIndex )
        {
            auto& texture = static_cast< Texture2DGeneric< unsigned char >& >( *m_textures[ textureType ] );

            m_dimensions.x = std::max( texture.getWidth(), m_dimensions.x );
            m_dimensions.y = std::max( texture.getHeight(), m_dimensions.y );
        }
        else
        {
            auto& texture = static_cast< Texture2DGeneric< uchar4 >& >( *m_textures[ textureType ] );

            m_dimensions.x = std::max( texture.getWidth(), m_dimensions.x );
            m_dimensions.y = std::max( texture.getHeight(), m_dimensions.y );
        }
    }
}

int2 ModelUtil::TextureSet::getTextureDimensions( Model::TextureType textureType ) const
{
    if ( !m_textures[ (int)textureType ] )
            return int2::ZERO;

    if ( textureType == Model::TextureType::Alpha
        || textureType == Model::TextureType::Metalness
        || textureType == Model::TextureType::Roughness
        || textureType == Model::TextureType::RefractiveIndex )
    {
        auto& texture = static_cast< const Texture2DGeneric< unsigned char >& >( *m_textures[ (int)textureType ] );

        return texture.getDimensions();
    }
    else
    {
        auto& texture = static_cast< const Texture2DGeneric< uchar4 >& >( *m_textures[ (int)textureType ] );

        return texture.getDimensions();
    }
}
