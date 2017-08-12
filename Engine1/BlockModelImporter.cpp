#include "BlockModelImporter.h"

#include <array>

#include "BlockMesh.h"
#include "BlockModel.h"
#include "BinaryFile.h"
#include "FileUtil.h"
#include "StringUtil.h"

#include "ModelTexture2DParser.h"

#include "Assimp/Importer.hpp"
#include "Assimp/Exporter.hpp"
#include "Assimp/scene.h" 
#include "Assimp/postprocess.h"

#include "Settings.h"

using namespace Engine1;

std::vector< std::shared_ptr< BlockModel > > BlockModelImporter::import( 
    std::string filePath,
    const bool invertZCoordinate,
    const bool invertVertexWindingOrder,
    const bool flipUVs )
{
    std::vector< std::shared_ptr< BlockModel > > models;

    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

    if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
    if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
    if ( flipUVs )                  flags |= aiProcess_FlipUVs;

    const aiScene* aiscene = importer.ReadFile( filePath.c_str(), flags );

    if ( !aiscene ) 
        throw std::exception( ("BlockModelParser::parseObjFile - parsing failed - " + std::string( importer.GetErrorString() )).c_str() );

    models.reserve( aiscene->mNumMeshes );

    for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) 
    {
        aiMesh& aimesh = *aiscene->mMeshes[ meshIndex ];

        models.push_back( std::make_shared< BlockModel >() );
        auto& model = *models.back();

        const int texcoordSetCount = 1;

        model.setMesh( std::make_shared< BlockMesh >( 
            aimesh.mNumVertices, 
            aimesh.HasNormals() && aimesh.HasTangentsAndBitangents(), 
            texcoordSetCount, 
            aimesh.mNumFaces ) 
        );

        auto mesh = model.getMesh();

        auto& meshVertices = mesh->getVertices();
        for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) 
            meshVertices[ i ] = ( *(float3*)&aimesh.mVertices[ i ] );

        if ( aimesh.HasNormals() && aimesh.HasTangentsAndBitangents() ) 
        {
            auto& meshNormals = mesh->getNormals();
            for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) 
                meshNormals[ i ] = ( *(float3*)&aimesh.mNormals[ i ] );

            auto& meshTangents = mesh->getTangents();
            for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) 
                meshTangents[ i ] = ( *(float3*)&aimesh.mTangents[ i ] );
        }

        if ( aimesh.HasTextureCoords( 0 ) ) 
        {
            auto& meshTexcoords = mesh->getTexcoords( 0 );
            for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i )
                meshTexcoords[ i ] = ( float2( aimesh.mTextureCoords[ 0 ][ i ].x, aimesh.mTextureCoords[ 0 ][ i ].y ) );
        }

        auto& meshTriangles = mesh->getTriangles();
        for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
            meshTriangles[ i ] = ( *reinterpret_cast<uint3*>(aimesh.mFaces[ i ].mIndices) );
        }

        aiMaterial& aimaterial = *aiscene->mMaterials[ aimesh.mMaterialIndex ];

        // Read all textures from the material.
        const int textureTypeCount = 11; // Hard coded in Assimp.
        for ( int aiTextureType = 0; aiTextureType < textureTypeCount; ++aiTextureType )
        {
            try
            {
                aiString path;
                if ( aimaterial.Get( _AI_MATKEY_TEXTURE_BASE, aiTextureType, 0, path ) == aiReturn_SUCCESS )
                {
                    auto fileName      = FileUtil::getFileNameFromPath( path.C_Str() );
                    auto fileExtension = FileUtil::getFileExtensionFromPath( fileName );
                    auto fileFormat    = Texture2DFileInfo::fromExtension( fileExtension );

                    // Try to deduce texture type based on the file name's suffix (_A, _N etc).
                    Model::TextureType textureType = Model::textureFileNameToType( fileName );

                    // #TODO: Should be replaced by some utility method.
                    Texture2DFileInfo::PixelType pixelType = Texture2DFileInfo::PixelType::UCHAR;
                    if ( textureType == Model::TextureType::Emissive
                        || textureType == Model::TextureType::Albedo
                        || textureType == Model::TextureType::Normal )
                    {
                        pixelType = Texture2DFileInfo::PixelType::UCHAR4;
                    }

                    Texture2DFileInfo fileInfo;
                    fileInfo.setPath( fileName );
                    fileInfo.setFormat( fileFormat );
                    fileInfo.setPixelType( pixelType );

                    // Remove existing textures of the same type - only one is allowed during import.
                    model.removeAllTextures( textureType );

                    if ( pixelType == Texture2DFileInfo::PixelType::UCHAR )
                    {
                        auto emptyTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >();
                        emptyTexture->setFileInfo( fileInfo );

                        model.addTexture( textureType, emptyTexture );
                    }
                    else
                    {
                        auto emptyTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >();
                        emptyTexture->setFileInfo( fileInfo );

                        model.addTexture( textureType, emptyTexture );
                    }
                }
            }
            catch ( std::exception& e )
            {
                OutputDebugStringW( StringUtil::widen( 
                    std::string( "BlockModelImporter::import - error while parsing material textures: \"" ) 
                    + e.what() + "\"\n"  
                ).c_str( ) );
            }
        }

        // Read albedo color multipliers.
        aiColor3D albedoMul;
        if ( aimaterial.Get(AI_MATKEY_COLOR_DIFFUSE, albedoMul) == aiReturn_SUCCESS )
        {
            // Add an empty model texture if none exists.
            if ( model.getAlbedoTextures().empty() )
            {
                auto emptyTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >();
                model.addTexture( Model::TextureType::Albedo, emptyTexture );
            }

            auto& albedoModelTexture = model.getAlbedoTextures().front();

            albedoModelTexture.setColorMultiplier( float4(
                albedoMul.r,
                albedoMul.g,
                albedoMul.b,
                1.0f
            ) );
        }
    }

    return models;
}

