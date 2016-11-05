#include "ModelUtil.h"

#include <tuple>
#include <algorithm>
#include <array>

#include "BlockModel.h"

#include "TextureUtil.h"
#include "MeshUtil.h"

using namespace Engine1;

//std::shared_ptr< BlockModel > ModelUtil::mergeModels( const std::vector< std::shared_ptr< BlockModel > >& models, ID3D11Device& device )
//{
//    // #TODO: BIG PROBLEM: When merging models - we create a texture atlas for albedo textures. But roughness textures 
//    // or emissive may have completely different dimensions and be placed differently in the atlas.
//    // So we somehow have to enforce layout withing the atlas based on the first created atlas.How to deal with big textures?
//
//    // #TODO: PROBLEM: What if all alpha textures are the same among models, but roughness textures are different. 
//    // It should be supported by duplicating alpha texture the same way roughness is placed within the merged texture.
//    // It gets more complex when there are some repeated textures in metalness, rougnees, albedo etc at the same time.
//    // : idea: start by merging the set of textures which has the most unique textures. 
//    // Then merge other sets using the given placement. If something goeas wrong - interrupt.
//
//    // #TODO: How to improve code by getting rid of these distinction between emissive/albedo/metalness etc and treat them as texture sets.
//    // Store texture sets in a map? <type, vector> ? and iterate over that map...?
//
//    // #TODO: Support for multiple UV sets/multitexturing.
//
//    // #TODO: How to deal with color multipliers per texture when merging them?
//
//    // We assume that there is one texture of each type (albedo, roughness etc) for each texcoord in each model. Or there may be zero texture of given type for all models.
//
//    std::array< std::vector< std::shared_ptr< Asset > >, (int)Model::TextureType::COUNT > uniqueTexturesSets;
//    std::array< std::vector< std::shared_ptr< Asset > >, (int)Model::TextureType::COUNT > nonUniqueTexturesSets;
//
//    const int currTexcoordSetIndex = 0;
//
//    // Gather unique textures which use the same texcoord set.
//    for ( auto& model : models )
//    {
//        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
//        {
//            for ( const auto& modelTexture : model->getTextures( (Model::TextureType)textureType ) )
//            {
//                std::shared_ptr< Asset > texture;
//                int                      texcoordSetIndex;
//
//                std::tie( texture, texcoordSetIndex ) = modelTexture;
//
//                if ( texcoordSetIndex == currTexcoordSetIndex )
//                {
//                    const bool alreadyListed = std::find( uniqueTexturesSets[ textureType ].begin(), uniqueTexturesSets[ textureType ].end(), texture ) != uniqueTexturesSets[ textureType ].end();
//                    
//                    nonUniqueTexturesSets[ textureType ].push_back( texture );
//
//                    if ( !alreadyListed )
//                        uniqueTexturesSets[ textureType ].push_back( texture );
//                }
//            }
//        }
//    }
//
//    // Check if there is one texture of each type per model for the current texcoord set.
//    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
//    {
//        if ( !nonUniqueTexturesSets[ textureType ].empty() && nonUniqueTexturesSets[ textureType ].size() != models.size() )
//        {
//            std::string description;
//            for ( auto& model : models )
//                description += getDescription( *model );
//
//            //#TODO: Should use default textures in that case. Default textures should be in model class maybe? And they should be a bit larger than 1x1 to avoid filtering issues...
//            throw std::exception( ( "ModelUtil::mergeModels - cannot merge models, because some types of textures are present only in some of the models. (ex: only model 0 and 2 have roughness texture). Details: \n\n" + description ).c_str() );
//        }
//    }
//
//    // Decide whether to use unique or non-unique texture set when merging models.
//    // Use unique set if all models share the same textures. Use non-unique textures otherwise.
//    bool modelsShareAllTextures = true;
//    {
//        // Check if there is one unique texture for each type.
//        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
//        {
//            if ( uniqueTexturesSets[ textureType ].size() > 1 )
//                modelsShareAllTextures = false;
//        }
//    }
//
//    std::array< std::vector< std::shared_ptr< Asset > >, (int)Model::TextureType::COUNT >& textureSets = modelsShareAllTextures ? uniqueTexturesSets : nonUniqueTexturesSets;
//
//
//    std::array< std::shared_ptr< Asset >, (int)Model::TextureType::COUNT >                     mergedTextures;
//    std::array< std::vector< TextureUtil::TexturePlacement >, (int)Model::TextureType::COUNT > mergedTexturePlacements;
//    std::vector< TextureUtil::TexturePlacement >*                                              mergedTexturePlacement = nullptr;
//    
//    // Merge textures.
//    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
//    {
//        if ( !textureSets[ textureType ].empty() ) 
//        {
//            Model::TextureType texType = (Model::TextureType)textureType;
//            
//            if ( texType == Model::TextureType::Alpha ||
//                 texType == Model::TextureType::Metalness || 
//                 texType == Model::TextureType::Roughness ||
//                 texType == Model::TextureType::RefractiveIndex )
//            {
//                auto textureSet = reinterpret_cast< std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > >& >( textureSets[ textureType ] );
//
//                std::tie( mergedTextures[ textureType ], mergedTexturePlacements[ textureType ] )
//                    = TextureUtil::mergeTextures( textureSet, device, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
//            }
//            else
//            {
//                auto textureSet = reinterpret_cast<std::vector< std::shared_ptr< Texture2DGeneric< uchar4 > > >&>( textureSets[ textureType ] );
//
//                std::tie( mergedTextures[ textureType ], mergedTexturePlacements[ textureType ] )
//                    = TextureUtil::mergeTextures( textureSet, device, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
//            }
//
//            mergedTexturePlacement = &mergedTexturePlacements[ textureType ];
//        }
//    }
//
//    // Check if texture placements for all texture types are the same 
//    // - that textures were placed at the same texcoords within the merged texture.
//    for ( int textureIdx = 0; textureIdx < models.size(); ++textureIdx ) 
//    {
//        TextureUtil::TexturePlacement& texturePlacement = mergedTexturePlacement->at( textureIdx );
//
//        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) 
//        {
//            if ( !mergedTexturePlacements[ textureType ].empty() && !TextureUtil::TexturePlacement::areTexcoordsEqual( texturePlacement, mergedTexturePlacements[ textureType ][ textureIdx ] ) ) 
//            {
//                std::string description;
//
//                for ( auto& model : models )
//                    description += getDescription( *model, true, true );
//
//                throw std::exception( ( "ModelUtil::mergeModels - cannot merge models, because of dfferences in size for textures of various types (albedo, roughness etc) - each type of textures requirs a different set of textures after merge." + description ).c_str() );
//            }
//        }
//    }
//
//    // Gather meshes to be merged.
//    std::vector< std::shared_ptr< BlockMesh > > meshes;
//    meshes.reserve( models.size() );
//    for ( auto& model : models ) 
//        meshes.push_back( model->getMesh() );
//
//    // Merge the meshes.
//    std::shared_ptr< BlockMesh > mergedMesh = MeshUtil::mergeMeshes( meshes );
//
//    { // Recalculate merged mesh texcoords to match the merged textures.
//        std::vector< float2 >& mergedMeshTexcoords = mergedMesh->getTexcoords( 0 );
//        int vertexStartIndex = 0, vertexEndIndex = 0;
//        for ( int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex ) 
//        {
//            const std::shared_ptr< BlockMesh >& mesh             = meshes[ meshIndex ];
//            const TextureUtil::TexturePlacement texturePlacement = mergedTexturePlacement->at( meshIndex );
//
//            vertexEndIndex += (int)mesh->getVertices().size();
//            // Recalculate texcoords.
//            for ( int vertexIndex = vertexStartIndex; vertexIndex < vertexEndIndex; ++vertexIndex )
//                mergedMeshTexcoords[ vertexIndex ] = texturePlacement.getTopLeftTexcoords() + mergedMeshTexcoords[ vertexIndex ] * texturePlacement.getDimensionsInTexcoords();
//
//            vertexStartIndex = vertexEndIndex;
//        }
//    }
//
//    std::shared_ptr< BlockModel > mergedModel = std::make_shared< BlockModel >();
//    { // Create merged model.
//        mergedModel->setMesh( mergedMesh );
//
//        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
//        {
//            if ( mergedTextures[ textureType ] )
//                mergedModel->addTexture( (Model::TextureType)textureType, mergedTextures[ textureType ], currTexcoordSetIndex );
//        }
//    }
//
//    return mergedModel;
//}

std::shared_ptr< BlockModel > ModelUtil::mergeModels( const std::vector< std::shared_ptr< BlockModel > >& models, ID3D11Device& device )
{
    // #TODO: BIG PROBLEM: When merging models - we create a texture atlas for albedo textures. But roughness textures 
    // or emissive may have completely different dimensions and be placed differently in the atlas.
    // So we somehow have to enforce layout withing the atlas based on the first created atlas.How to deal with big textures?

    // #TODO: PROBLEM: What if all alpha textures are the same among models, but roughness textures are different. 
    // It should be supported by duplicating alpha texture the same way roughness is placed within the merged texture.
    // It gets more complex when there are some repeated textures in metalness, rougnees, albedo etc at the same time.
    // : idea: start by merging the set of textures which has the most unique textures. 
    // Then merge other sets using the given placement. If something goeas wrong - interrupt.

    // #TODO: How to improve code by getting rid of these distinction between emissive/albedo/metalness etc and treat them as texture sets.
    // Store texture sets in a map? <type, vector> ? and iterate over that map...?

    // #TODO: Support for multiple UV sets/multitexturing.

    // #TODO: How to deal with color multipliers per texture when merging them?

    // We assume that there is one texture of each type (albedo, roughness etc) for each texcoord in each model. Or there may be zero texture of given type for all models.

    // Stores unique textures for all models.
    std::array< std::vector< std::shared_ptr< Asset > >, (int)Model::TextureType::COUNT > uniqueTexturesSets;

    // Stores one texture (or none) for each model in the same order in which models are.
    std::array< std::vector< std::shared_ptr< Asset > >, (int)Model::TextureType::COUNT > modelsTexturesSets;

    const int currTexcoordSetIndex = 0;

    // Gather unique textures which use the same texcoord set.
    for ( int modelIdx = 0; modelIdx < models.size(); ++modelIdx ) 
    {
        const auto& model = models[ modelIdx ];
        
        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) 
        {
            modelsTexturesSets[ textureType ].resize( models.size(), nullptr );

            for ( const auto& modelTexture : model->getTextures( ( Model::TextureType )textureType ) ) 
            {
                std::shared_ptr< Asset > texture;
                int                      texcoordSetIndex;

                std::tie( texture, texcoordSetIndex ) = modelTexture;

                if ( texcoordSetIndex == currTexcoordSetIndex ) 
                {
                    const bool alreadyListed = std::find( uniqueTexturesSets[ textureType ].begin(), uniqueTexturesSets[ textureType ].end(), texture ) != uniqueTexturesSets[ textureType ].end();

                    modelsTexturesSets[ textureType ][ modelIdx ] = texture;

                    if ( !alreadyListed )
                        uniqueTexturesSets[ textureType ].push_back( texture );

                    break; // Allow for only one texture per texcoord set per model.
                }
            }
        }
    }

    // Check if there is one texture of each type per model for the current texcoord set.
    //for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) {
    //    if ( !nonUniqueTexturesSets[ textureType ].empty() && nonUniqueTexturesSets[ textureType ].size() != models.size() ) {
    //        std::string description;
    //        for ( auto& model : models )
    //            description += getDescription( *model );

    //        //#TODO: Should use default textures in that case. Default textures should be in model class maybe? And they should be a bit larger than 1x1 to avoid filtering issues...
    //        throw std::exception( ( "ModelUtil::mergeModels - cannot merge models, because some types of textures are present only in some of the models. (ex: only model 0 and 2 have roughness texture). Details: \n\n" + description ).c_str() );
    //    }
    //}

    // Find a type of texture for which there is the most unique textures.
    Model::TextureType mainTextureType = Model::TextureType::Alpha;
    int maxUniqueTexturesCount = 0;

    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) 
    {
        if ( uniqueTexturesSets[ textureType ].size() > maxUniqueTexturesCount ) 
        {
            maxUniqueTexturesCount = (int)uniqueTexturesSets[ textureType ].size();
            mainTextureType        = (Model::TextureType)textureType;
        }
    }

    std::array< std::shared_ptr< Asset >, (int)Model::TextureType::COUNT > mergedTextures;
    std::vector< TextureUtil::TexturePlacement >                           mergedTexturePlacement;

    // Merge main texture set (which has the most unique textures).
    if ( mainTextureType == Model::TextureType::Alpha || mainTextureType == Model::TextureType::Metalness ||
         mainTextureType == Model::TextureType::Roughness || mainTextureType == Model::TextureType::RefractiveIndex ) 
    {
        auto textureSet = reinterpret_cast<std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > >&>( uniqueTexturesSets[ (int)mainTextureType ] );

        std::tie( mergedTextures[ (int)mainTextureType ], mergedTexturePlacement )
            = TextureUtil::mergeTextures( textureSet, device, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
    } 
    else 
    {
        auto textureSet = reinterpret_cast<std::vector< std::shared_ptr< Texture2DGeneric< uchar4 > > >&>( uniqueTexturesSets[ (int)mainTextureType ] );

        std::tie( mergedTextures[ (int)mainTextureType ], mergedTexturePlacement )
            = TextureUtil::mergeTextures( textureSet, device, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
    }

    // Merge remaining texture sets.
    for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) 
    {
        if ( textureType == (int)mainTextureType )
            continue; // Main type is already merged.

        if ( uniqueTexturesSets[ textureType ].empty() )
            continue;
        
        Model::TextureType texType = ( Model::TextureType )textureType;

        std::shared_ptr< Asset > mergedTexture;

        if ( texType == Model::TextureType::Alpha || texType == Model::TextureType::Metalness ||
             texType == Model::TextureType::Roughness || texType == Model::TextureType::RefractiveIndex ) 
        {
            auto textureSet = reinterpret_cast<std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > >&>( uniqueTexturesSets[ textureType ] );
            const bool duplicateTextures = textureSet.size() == 1;
            mergedTexture   = TextureUtil::mergeTextures( textureSet, mergedTexturePlacement, duplicateTextures, device, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
        } 
        else 
        {
            auto textureSet = reinterpret_cast<std::vector< std::shared_ptr< Texture2DGeneric< uchar4 > > >&>( uniqueTexturesSets[ textureType ] );
            const bool duplicateTextures = textureSet.size() == 1;
            mergedTexture   = TextureUtil::mergeTextures( textureSet, mergedTexturePlacement, duplicateTextures, device, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
        }

        if ( mergedTexture ) {
            mergedTextures[ textureType ] = mergedTexture;
        } else {
            throw std::exception( ("ModelUtil::mergeModels - merging was not possible, because " + Model::textureTypeToString( texType ) +
                                   " textures cannot be merged using the same texcoords as " + Model::textureTypeToString( mainTextureType ) + " textures").c_str() );
        }
    }

    // Gather meshes to be merged.
    std::vector< std::shared_ptr< BlockMesh > > meshes;
    meshes.reserve( models.size() );
    for ( auto& model : models )
        meshes.push_back( model->getMesh() );

    // Merge the meshes.
    std::shared_ptr< BlockMesh > mergedMesh = MeshUtil::mergeMeshes( meshes );

    { // Recalculate merged mesh texcoords to match the merged textures.
        std::vector< float2 >& mergedMeshTexcoords = mergedMesh->getTexcoords( 0 );

        // Check if all texcoords are zeros.
        bool allTexcoordsAreZeros = true;
        for ( const float2& texcoord : mergedMeshTexcoords )
        {
            if ( !MathUtil::areEqual( texcoord, float2::ZERO, 0.0f, 0.0001f ) ) {
                allTexcoordsAreZeros = false;
                break;
            }
        }

        // Optionally apply texcoords offset if all texcoords are zeros.
        // Without such offset texcoords after merge would end up in the top-left corner 
        // of the input textures in the atlas. That would cause unwanted blending between them when using bilinear sampling.
        const float2 texcoordOffset = allTexcoordsAreZeros ? float2( 0.5f, 0.5f ) : float2::ZERO;

        int vertexStartIndex = 0, vertexEndIndex = 0;

        for ( int modelIndex = 0; modelIndex < models.size(); ++modelIndex ) 
        {
            const auto& mesh         = models[ modelIndex ]->getMesh();
            const auto& modelTexture = modelsTexturesSets[ (int)mainTextureType ][ modelIndex ];

            const auto& uniqueTextureIt = std::find( uniqueTexturesSets[ (int)mainTextureType ].begin(), uniqueTexturesSets[ (int)mainTextureType ].end(), modelTexture );
            const int   indexWithinUniqueTextures = (int)(uniqueTextureIt - uniqueTexturesSets[ (int)mainTextureType ].begin());

            const TextureUtil::TexturePlacement texturePlacement = mergedTexturePlacement[ indexWithinUniqueTextures ];

            vertexEndIndex += (int)mesh->getVertices().size();
            // Recalculate texcoords.
            for ( int vertexIndex = vertexStartIndex; vertexIndex < vertexEndIndex; ++vertexIndex )
                mergedMeshTexcoords[ vertexIndex ] = texturePlacement.getTopLeftTexcoords() + ( mergedMeshTexcoords[ vertexIndex ] + texcoordOffset ) * texturePlacement.getDimensionsInTexcoords();

            vertexStartIndex = vertexEndIndex;
        }
    }

    std::shared_ptr< BlockModel > mergedModel = std::make_shared< BlockModel >();
    { // Create merged model.
        mergedModel->setMesh( mergedMesh );

        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType ) {
            if ( mergedTextures[ textureType ] )
                mergedModel->addTexture( ( Model::TextureType )textureType, mergedTextures[ textureType ], currTexcoordSetIndex );
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

    if ( model.getAlphaTexturesCount() > 0 )
         text += "alpha: " + std::to_string( model.getAlphaTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getAlphaTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getEmissiveTexturesCount() > 0 )
        text += "emissive: " + std::to_string( model.getEmissiveTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getEmissiveTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getAlbedoTexturesCount() > 0 )
        text += "albedo: " + std::to_string( model.getAlbedoTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getAlbedoTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getMetalnessTexturesCount() > 0 )
        text += "metalness: " + std::to_string( model.getMetalnessTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getMetalnessTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getRoughnessTexturesCount() > 0 )
        text += "roughness: " + std::to_string( model.getRoughnessTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getRoughnessTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getNormalTexturesCount() > 0 )
        text += "normal: " + std::to_string( model.getNormalTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getNormalTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    if ( model.getRefractiveIndexTexturesCount() > 0 )
        text += "normal: " + std::to_string( model.getRefractiveIndexTexturesCount() ) + "\n";

    for ( auto& modelTexture : model.getRefractiveIndexTextures() )
        text += TextureUtil::getDescription( *modelTexture.getTexture(), printPath, printDimensions );

    text += "\n";

    return text;
}
