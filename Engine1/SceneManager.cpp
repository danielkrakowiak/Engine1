#include "SceneManager.h"

#include <algorithm>

#include "AssetManager.h"

#include "Scene.h"
#include "BlockActor.h"
#include "SkeletonActor.h"
#include "BlockModel.h"
#include "SkeletonModel.h"
#include "BlockMesh.h"
#include "SkeletonMesh.h"

#include "SkeletonAnimationFileInfo.h"
#include "SkeletonAnimation.h"

#include "Timer.h"
#include "FileInfo.h"
#include "MathUtil.h"
#include "ModelUtil.h"
#include "StringUtil.h"

#include "PointLight.h"
#include "SpotLight.h"

using namespace Engine1;
using Microsoft::WRL::ComPtr;

SceneManager::SceneManager( AssetManager& assetManager ) :
    m_assetManager( assetManager ),
    m_scenePath( "Assets/Scenes/new.scene" ),
    m_cameraPath( "Assets/Scenes/new.camera" ),
    m_scene( std::make_shared< Scene >() )
{
    // Setup the camera.
    m_camera.setUp( float3( 0.0f, 1.0f, 0.0f ) );
    m_camera.setPosition( float3( 30.0f, 4.0f, -53.0f ) );
    m_camera.rotate( float3( 0.0f, MathUtil::piHalf, 0.0f ) );
}

void SceneManager::initialize( Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device        = device;
    m_deviceContext = deviceContext;
}

FreeCamera& SceneManager::getCamera()
{
    return m_camera;
}

Scene& SceneManager::getScene()
{
    return *m_scene;
}

std::vector< std::shared_ptr< Light > >& SceneManager::getSelectedLights()
{
    return m_selectedLights;
}

std::vector< std::shared_ptr< BlockActor > >& SceneManager::getSelectedBlockActors()
{
    return m_selectedBlockActors;
}

std::vector< std::shared_ptr< SkeletonActor > >& SceneManager::getSelectedSkeletonActors()
{
    return m_selectedSkeletonActors;
}

std::shared_ptr< BlockMesh > SceneManager::getSelectionVolumeMesh()
{
    return m_selectionVolumeMesh;
}

void SceneManager::loadCamera( std::string path )
{
    m_camera = *FreeCamera::createFromFile( path );
}

void SceneManager::saveCamera( std::string path )
{
    m_camera.saveToFile( path );
}

void SceneManager::loadScene( std::string path )
{
    std::shared_ptr< std::vector < std::shared_ptr< FileInfo > > > fileInfos;

    std::tie( m_scene, fileInfos ) = Scene::createFromFile( path );

    // Load all assets.
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        m_assetManager.loadAsync( *fileInfo );

    // Wait for all assets to be loaded.
    const Timer loadingStartTime;
    const float maxLoadingTime = 60.0f;
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos ) {
        const Timer currTime;
        const float loadingTime = (float)Timer::getElapsedTime( currTime, loadingStartTime ) / 1000.0f;
        const float timeout = std::max( 0.0f, maxLoadingTime - loadingTime );

        m_assetManager.getWhenLoaded( fileInfo->getAssetType(), fileInfo->getPath(), fileInfo->getIndexInFile(), timeout );
    }

    // Swap actors' empty models with the loaded models. Create BVH trees. Load models to GPU.
    const std::unordered_set< std::shared_ptr< Actor > > sceneActors = m_scene->getActors();
    for ( const std::shared_ptr< Actor >& actor : sceneActors ) {
        if ( actor->getType() == Actor::Type::BlockActor ) {

            const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>( actor );
            if ( blockActor->getModel() ) {
                const BlockModelFileInfo& fileInfo = blockActor->getModel()->getFileInfo();
                std::shared_ptr<BlockModel> blockModel = std::static_pointer_cast<BlockModel>( m_assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ) );
                if ( blockModel ) {
                    // Build BVH tree and load it to GPU.
                    if ( blockModel->getMesh() ) {
                        if ( !blockModel->getMesh()->getBvhTree() )
                            blockModel->getMesh()->buildBvhTree();

                        blockModel->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
                    }

                    blockModel->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );
                    blockActor->setModel( blockModel ); // Swap an empty model with a loaded model.
                } else {
                    throw std::exception( "SceneTools::loadScene - failed to load one of the scene's models." );
                }
            }
        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const std::shared_ptr<SkeletonActor>& skeletonActor = std::static_pointer_cast<SkeletonActor>( actor );
            if ( skeletonActor->getModel() ) {
                const SkeletonModelFileInfo& fileInfo = skeletonActor->getModel()->getFileInfo();
                std::shared_ptr<SkeletonModel> skeletonModel = std::static_pointer_cast<SkeletonModel>( m_assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ) );
                if ( skeletonModel ) {
                    skeletonModel->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );
                    skeletonActor->setModel( skeletonModel ); // Swap an empty model with a loaded model.
                } else {
                    throw std::exception( "SceneTools::loadScene - failed to load one of the scene's models." );
                }
            }
        }
    }
}

void SceneManager::saveScene( std::string path )
{
    m_scene->saveToFile( path );
}

void SceneManager::loadAsset( std::string filePath, const bool replaceSelected )
{
    const size_t dotIndex = filePath.rfind( "." );
    if ( dotIndex == std::string::npos )
        return;

    std::string filePathWithoutExtension = StringUtil::toLowercase( filePath.substr( 0, dotIndex ) );
    std::string extension = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

    std::array< const std::string, 4 > blockMeshExtensions     = { "obj", "dae", "fbx", "blockmesh" };
    std::array< const std::string, 1 > skeletonkMeshExtensions = { "dae" };
    std::array< const std::string, 9 > textureExtensions       = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff", "tif" };
    std::array< const std::string, 1 > blockModelExtensions    = { "blockmodel" };
    std::array< const std::string, 1 > skeletonModelExtensions = { "skeletonmodel" };
    std::array< const std::string, 1 > animationExtensions     = { "xaf" };
    std::array< const std::string, 1 > sceneExtensions         = { "scene" };
    std::array< const std::string, 1 > cameraExtensions        = { "camera" };

    bool isBlockMesh     = false;
    bool isSkeletonMesh  = false;
    bool isTexture       = false;
    bool isBlockModel    = false;
    bool isSkeletonModel = false;
    bool isAnimation     = false;
    bool isScene         = false;
    bool isCamera        = false;

    for ( const std::string& blockMeshExtension : blockMeshExtensions ) {
        if ( extension.compare( blockMeshExtension ) == 0 )
            isBlockMesh = true;
    }

    for ( const std::string& skeletonkMeshExtension : skeletonkMeshExtensions ) {
        if ( extension.compare( skeletonkMeshExtension ) == 0 )
            isSkeletonMesh = true;
    }

    for ( const std::string& textureExtension : textureExtensions ) {
        if ( extension.compare( textureExtension ) == 0 )
            isTexture = true;
    }

    for ( const std::string& blockModelExtension : blockModelExtensions ) {
        if ( extension.compare( blockModelExtension ) == 0 )
            isBlockModel = true;
    }

    for ( const std::string& skeletonModelExtension : skeletonModelExtensions ) {
        if ( extension.compare( skeletonModelExtension ) == 0 )
            isSkeletonModel = true;
    }

    for ( const std::string& animationExtension : animationExtensions ) {
        if ( extension.compare( animationExtension ) == 0 )
            isAnimation = true;
    }

    for ( const std::string& sceneExtension : sceneExtensions ) {
        if ( extension.compare( sceneExtension ) == 0 )
            isScene = true;
    }

    for ( const std::string& cameraExtension : cameraExtensions ) {
        if ( extension.compare( cameraExtension ) == 0 )
            isCamera = true;
    }

    float43 pose = float43::IDENTITY;
    pose.setTranslation( m_camera.getPosition() + m_camera.getDirection() );

    if ( isBlockMesh ) {
        BlockMeshFileInfo::Format format = BlockMeshFileInfo::Format::OBJ;

        if ( extension.compare( "obj" ) == 0 )            format = BlockMeshFileInfo::Format::OBJ;
        else if ( extension.compare( "dae" ) == 0 )       format = BlockMeshFileInfo::Format::DAE;
        else if ( extension.compare( "fbx" ) == 0 )       format = BlockMeshFileInfo::Format::FBX;
        else if ( extension.compare( "blockmesh" ) == 0 ) format = BlockMeshFileInfo::Format::BLOCKMESH;

        // Note: Bad support for multiple meshes in a single file. Because of how AssetManager works, they have to be parsed one by one from the same file.
        // So the file has to be re-parsed for each mesh it contains. Not a big problem normally, because only one mesh should be in a single file.

        // We try to parse meshes with increasing index in file until it fails.
        for ( int indexInFile = 0; true; ++indexInFile ) {
            try {
                BlockMeshFileInfo fileInfo( filePath, format, indexInFile, true, true, true ); // Note: Convert to right hand coord system.
                std::shared_ptr< BlockMesh > mesh = std::static_pointer_cast<BlockMesh>( m_assetManager.getOrLoad( fileInfo ) );

                if ( !mesh->getBvhTree() )
                    mesh->buildBvhTree();

                if ( !mesh->isInGpuMemory() ) {
                    mesh->loadCpuToGpu( *m_device.Get() );
                    mesh->loadBvhTreeToGpu( *m_device.Get() );
                }

                if ( replaceSelected ) {
                    // Replace a mesh of an existing model.
                    if ( m_selectedBlockActors.size() == 1 && m_selectedBlockActors[ 0 ]->getModel() ) {
                        m_selectedBlockActors[ 0 ]->getModel()->setMesh( mesh );

                        break; // If replacing a mesh - read only the one with index 0 in file.
                    }
                } else {
                    // Add new actor to the scene.
                    m_selectedBlockActors.clear();
                    m_selectedBlockActors.push_back( std::make_shared< BlockActor >( std::make_shared< BlockModel >(), pose ) );
                    m_selectedBlockActors[ 0 ]->getModel()->setMesh( mesh );
                    m_scene->addActor( m_selectedBlockActors[ 0 ] );
                }

                // Save mesh to .blockmesh format for future use.
                if ( format != BlockMeshFileInfo::Format::BLOCKMESH )
                    mesh->saveToFile( filePathWithoutExtension + ( indexInFile != 0 ? "_" + std::to_string( indexInFile ) : "" ) + ".blockmesh", BlockMeshFileInfo::Format::BLOCKMESH );
            } catch ( ... ) {
                break;
            }
        }
    }

    if ( isSkeletonMesh ) {
        SkeletonMeshFileInfo::Format format = SkeletonMeshFileInfo::Format::DAE;

        if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

        // Note: Bad support for multiple meshes in a single file. Because of how AssetManager works, they have to be parsed one by one from the same file.
        // So the file has to be re-parsed for each mesh it contains. Not a big problem normally, because only one mesh should be in a single file.

        // We try to parse meshes with increasing index in file until it fails.
        for ( int indexInFile = 0; true; ++indexInFile ) {
            try {
                SkeletonMeshFileInfo fileInfo( filePath, format, 0, true, true, true ); // Note: Convert to right hand coord system.
                std::shared_ptr<SkeletonMesh> mesh = std::static_pointer_cast<SkeletonMesh>( m_assetManager.getOrLoad( fileInfo ) );
                if ( !mesh->isInGpuMemory() )
                    mesh->loadCpuToGpu( *m_device.Get() );

                // #TODO: add replacing mesh? How to deal with non-matching animation?
                // Add new actor to the scene.
                m_selectedSkeletonActors.clear();
                m_selectedSkeletonActors.push_back( std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>(), pose ) );
                m_selectedSkeletonActors[ 0 ]->getModel()->setMesh( mesh );
                m_selectedSkeletonActors[ 0 ]->resetSkeletonPose();
                m_scene->addActor( m_selectedSkeletonActors[ 0 ] );
            } catch ( ... ) {
                break;
            }
        }
    }

    if ( isTexture ) {
        Texture2DFileInfo::Format format = Texture2DFileInfo::Format::BMP;

        if ( extension.compare( "bmp" ) == 0 )       format = Texture2DFileInfo::Format::BMP;
        else if ( extension.compare( "dds" ) == 0 )  format = Texture2DFileInfo::Format::DDS;
        else if ( extension.compare( "jpg" ) == 0 )  format = Texture2DFileInfo::Format::JPEG;
        else if ( extension.compare( "jpeg" ) == 0 ) format = Texture2DFileInfo::Format::JPEG;
        else if ( extension.compare( "png" ) == 0 )  format = Texture2DFileInfo::Format::PNG;
        else if ( extension.compare( "raw" ) == 0 )  format = Texture2DFileInfo::Format::RAW;
        else if ( extension.compare( "tga" ) == 0 )  format = Texture2DFileInfo::Format::TGA;
        else if ( extension.compare( "tiff" ) == 0 ) format = Texture2DFileInfo::Format::TIFF;
        else if ( extension.compare( "tif" ) == 0 )  format = Texture2DFileInfo::Format::TIFF;

        Texture2DFileInfo::PixelType pixelType;
        if ( filePath.find( "_A." ) != std::string::npos ||
             filePath.find( "_N." ) != std::string::npos ||
             filePath.find( "_E." ) != std::string::npos ) {
            pixelType = Texture2DFileInfo::PixelType::UCHAR4;
        } else if ( filePath.find( "_AL." ) != std::string::npos ||
                    filePath.find( "_M." ) != std::string::npos ||
                    filePath.find( "_R." ) != std::string::npos ||
                    filePath.find( "_I." ) != std::string::npos ) {
            pixelType = Texture2DFileInfo::PixelType::UCHAR;
        } else
            return; // Unrecognized texture type.

        Texture2DFileInfo fileInfo( filePath, format, pixelType );
        std::shared_ptr< Asset > textureAsset = m_assetManager.getOrLoad( fileInfo );

        if ( filePath.find( "_A." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 >>( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllAlbedoTextures();

                actor->getModel()->addAlbedoTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllAlbedoTextures();

                actor->getModel()->addAlbedoTexture( modelTexture );
            }
        } else if ( filePath.find( "_AL." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char >>( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllAlphaTextures();

                actor->getModel()->addAlphaTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllAlphaTextures();

                actor->getModel()->addAlphaTexture( modelTexture );
            }
        } else if ( filePath.find( "_M." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char >>( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllMetalnessTextures();

                actor->getModel()->addMetalnessTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllMetalnessTextures();

                actor->getModel()->addMetalnessTexture( modelTexture );
            }
        } else if ( filePath.find( "_N." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 >>( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllNormalTextures();

                actor->getModel()->addNormalTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllNormalTextures();

                actor->getModel()->addNormalTexture( modelTexture );
            }
        } else if ( filePath.find( "_R." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char >>( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllRoughnessTextures();

                actor->getModel()->addRoughnessTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllRoughnessTextures();

                actor->getModel()->addRoughnessTexture( modelTexture );
            }
        } else if ( filePath.find( "_E." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 >>( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllEmissiveTextures();

                actor->getModel()->addEmissiveTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllEmissiveTextures();

                actor->getModel()->addEmissiveTexture( modelTexture );
            }
        } else if ( filePath.find( "_I." ) != std::string::npos ) {
            auto texture = std::dynamic_pointer_cast<Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char >>( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllRefractiveIndexTextures();

                actor->getModel()->addRefractiveIndexTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) {
                if ( replaceSelected )
                    actor->getModel()->removeAllRefractiveIndexTextures();

                actor->getModel()->addRefractiveIndexTexture( modelTexture );
            }
        }
    }

    if ( isBlockModel ) {

        BlockModelFileInfo fileInfo( filePath, BlockModelFileInfo::Format::BLOCKMODEL, 0 );
        std::shared_ptr<BlockModel> model = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( fileInfo ) );

        if ( model->getMesh() && !model->getMesh()->getBvhTree() ) {
            model->getMesh()->buildBvhTree();
            model->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
        }

        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

        // Add new actor to the scene.
        m_selectedBlockActors.clear();
        m_selectedBlockActors.push_back( std::make_shared<BlockActor>( model, pose ) );
        m_scene->addActor( m_selectedBlockActors[ 0 ] );
    }

    if ( isSkeletonModel ) {
        SkeletonModelFileInfo fileInfo( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, 0 );
        std::shared_ptr<SkeletonModel> model = std::static_pointer_cast<SkeletonModel>( m_assetManager.getOrLoad( fileInfo ) );
        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

        // Add new actor to the scene.
        m_selectedSkeletonActors.clear();
        m_selectedSkeletonActors.push_back( std::make_shared<SkeletonActor>( model, pose ) );
        m_scene->addActor( m_selectedSkeletonActors[ 0 ] );
    }

    if ( isAnimation ) {
        for ( auto& actor : m_selectedSkeletonActors ) {
            if ( actor->getModel() && actor->getModel()->getMesh() ) {
                SkeletonMeshFileInfo&     referenceMeshFileInfo = actor->getModel()->getMesh()->getFileInfo();
                SkeletonAnimationFileInfo fileInfo( filePath, SkeletonAnimationFileInfo::Format::XAF, referenceMeshFileInfo, false );

                std::shared_ptr< SkeletonAnimation > animation = std::static_pointer_cast<SkeletonAnimation>( m_assetManager.getOrLoad( fileInfo ) );

                if ( animation )
                    actor->startAnimation( animation );
            }
        }
    }

    if ( ( isBlockMesh || isTexture ) && m_selectedBlockActors.size() == 1 && m_selectedBlockActors[ 0 ]->getModel() && m_selectedBlockActors[ 0 ]->getModel()->isInCpuMemory() )
        m_selectedBlockActors[ 0 ]->getModel()->saveToFile( "Assets/Models/new.blockmodel" );

    if ( ( isSkeletonMesh || isTexture ) && m_selectedSkeletonActors.size() == 1 && m_selectedSkeletonActors[ 0 ]->getModel() && m_selectedSkeletonActors[ 0 ]->getModel()->isInCpuMemory() )
        m_selectedSkeletonActors[ 0 ]->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );

    if ( isScene ) {
        loadScene( filePath );
        m_scenePath = filePath;
    }

    if ( isCamera ) {
        loadCamera( filePath );
        m_cameraPath = filePath;
    }
}

std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light > >
SceneManager::pickActorOrLight( const float2& targetPixel, const float screenWidth, const float screenHeight, const float fieldOfView )
{
    float43 cameraPose;
    cameraPose.setRow1( m_camera.getRight() );
    cameraPose.setRow2( m_camera.getUp() );
    cameraPose.setRow3( m_camera.getDirection() );
    cameraPose.setTranslation( m_camera.getPosition() );

    const float2 screenDimensions( screenWidth, screenHeight );

    const float3 rayOriginWorld = m_camera.getPosition();
    const float3 rayDirWorld = MathUtil::getRayDirectionAtPixel( cameraPose, targetPixel, screenDimensions, fieldOfView );

    float                    minHitDistance = FLT_MAX;
    std::shared_ptr< Actor > hitActor;
    std::shared_ptr< Light > hitLight;

    bool  hitOccurred = false;
    float hitDistance = FLT_MAX;

    for ( const std::shared_ptr< Light >& light : m_scene->getLights() ) 
    {
        BoundingBox bbBoxLocal( float3( -0.25f, -0.25f, -0.25f ), float3( 0.25f, 0.25f, 0.25f ) );

        float43 pose = float43::IDENTITY;
        pose.setTranslation( light->getPosition() );

        std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBoundingBox( rayOriginWorld, rayDirWorld, pose, bbBoxLocal );

        if ( hitOccurred && hitDistance < minHitDistance ) {
            minHitDistance = hitDistance;
            hitLight = light;
        }
    }

    for ( const std::shared_ptr< Actor >& actor : m_scene->getActors() ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) {
            const std::shared_ptr< BlockActor >& blockActor = std::static_pointer_cast<BlockActor>( actor );
            if ( !blockActor->getModel() || !blockActor->getModel()->getMesh() )
                continue;

            std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBlockActor( rayOriginWorld, rayDirWorld, *blockActor, minHitDistance );

            if ( hitOccurred && hitDistance < minHitDistance ) {
                minHitDistance = hitDistance;
                hitActor = blockActor;
            }
        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const std::shared_ptr< SkeletonActor >& skeletonActor = std::static_pointer_cast<SkeletonActor>( actor );
            if ( !skeletonActor->getModel() || !skeletonActor->getModel()->getMesh() )
                continue;

            const BoundingBox bbBoxLocal = skeletonActor->getModel()->getMesh()->getBoundingBox();

            std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBoundingBox( rayOriginWorld, rayDirWorld, skeletonActor->getPose(), bbBoxLocal );

            if ( hitOccurred && hitDistance < minHitDistance ) {
                minHitDistance = hitDistance;
                hitActor = skeletonActor;
            }
        }
    }

    if ( hitActor )
        return std::make_tuple( hitActor, nullptr );
    else
        return std::make_tuple( nullptr, hitLight );
}

void SceneManager::mergeSelectedActors()
{
    try {
        std::vector< std::shared_ptr< BlockModel > > models;
        models.reserve( m_selectedBlockActors.size() );

        for ( auto& actor : m_selectedBlockActors )
            models.push_back( actor->getModel() );

        std::shared_ptr< BlockModel > mergedModel = ModelUtil::mergeModels( models, *m_device.Get() );

        if ( mergedModel->getMesh() ) {
            mergedModel->getMesh()->recalculateBoundingBox();

            if ( !mergedModel->getMesh()->getBvhTree() ) {
                mergedModel->getMesh()->buildBvhTree();
                mergedModel->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
            }
        }

        if ( !mergedModel->isInGpuMemory() )
            mergedModel->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

        { // Save merged model, mesh and textures to files.
            std::string meshPath = "Assets/Meshes/merged.obj";
            std::string meshPath2 = "Assets/Meshes/merged.blockmesh";
            std::string texturePath = "Assets/Textures/merged";

            if ( mergedModel->getMesh() ) {
                mergedModel->getMesh()->saveToFile( meshPath, BlockMeshFileInfo::Format::OBJ );
                mergedModel->getMesh()->saveToFile( meshPath2, BlockMeshFileInfo::Format::BLOCKMESH );

                mergedModel->getMesh()->getFileInfo().setPath( meshPath );
                mergedModel->getMesh()->getFileInfo().setFormat( BlockMeshFileInfo::Format::OBJ );
                mergedModel->getMesh()->getFileInfo().setIndexInFile( 0 );
            }

            int textureIndex = 0;
            for ( auto& texture : mergedModel->getAlphaTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_AL.tiff";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getEmissiveTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_E.png";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getAlbedoTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_A.png";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getMetalnessTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_M.tiff";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getRoughnessTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_R.tiff";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getNormalTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_N.png";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getRefractiveIndexTextures() ) {
                std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_I.tiff";
                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
            }

            mergedModel->saveToFile( "Assets/Models/merged.blockmodel" );
        }

        float43 pose( float43::IDENTITY );
        pose.setTranslation( m_camera.getPosition() );

        // Add new actor to the scene.
        m_selectedBlockActors.clear();
        m_selectedBlockActors.push_back( std::make_shared< BlockActor >( mergedModel, pose ) );
        m_scene->addActor( m_selectedBlockActors[ 0 ] );
    } catch ( std::exception& e ) {
        OutputDebugStringW( StringUtil::widen( e.what() + std::string( "\n" ) ).c_str() );
    }
}

void SceneManager::saveSelectedModels()
{
    for ( auto& actor : m_selectedBlockActors ) {
        const std::shared_ptr< BlockModel > model = actor->getModel();
        if ( model ) {
            std::string path = model->getFileInfo().getPath();

            // If there is no existing path for model - use mesh's path.
            if ( path.empty() && model->getMesh() ) {
                path = model->getMesh()->getFileInfo().getPath();
                const int dotPos = (int)path.rfind( "." );
                if ( dotPos != std::string::npos )
                    path = path.replace( path.begin() + dotPos, path.end(), ".blockmodel" );
            }

            if ( !path.empty() )
                model->saveToFile( path );
        }
    }
}

std::tuple< int, int > SceneManager::getSelectedActorsVertexAndTriangleCount()
{
    int vertexCount = 0;
    int triangleCount = 0;

    for ( const auto& actor : m_selectedBlockActors ) {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        vertexCount += (int)actor->getModel()->getMesh()->getVertices().size();
        triangleCount += (int)actor->getModel()->getMesh()->getTriangles().size();
    }

    for ( const auto& actor : m_selectedSkeletonActors ) {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        vertexCount += (int)actor->getModel()->getMesh()->getVertices().size();
        triangleCount += (int)actor->getModel()->getMesh()->getTriangles().size();
    }

    return std::make_tuple( vertexCount, triangleCount );
}

std::tuple< int, int > SceneManager::getSceneVertexAndTriangleCount()
{
    int vertexCount = 0;
    int triangleCount = 0;

    const auto& actors = m_scene->getActors();

    for ( const auto& actor : actors ) {
        if ( actor->getType() == Actor::Type::BlockActor ) {
            const auto& blockActor = std::dynamic_pointer_cast<const BlockActor>( actor );

            if ( !blockActor->getModel() || !blockActor->getModel()->getMesh() )
                continue;

            vertexCount += (int)blockActor->getModel()->getMesh()->getVertices().size();
            triangleCount += (int)blockActor->getModel()->getMesh()->getTriangles().size();
        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const auto& skeletonActor = std::dynamic_pointer_cast<const SkeletonActor>( actor );

            if ( !skeletonActor->getModel() || !skeletonActor->getModel()->getMesh() )
                continue;

            vertexCount += (int)skeletonActor->getModel()->getMesh()->getVertices().size();
            triangleCount += (int)skeletonActor->getModel()->getMesh()->getTriangles().size();
        }
    }

    return std::make_tuple( vertexCount, triangleCount );
}

void SceneManager::addPointLight()
{
    float3 lightPosition = m_camera.getPosition() + m_camera.getDirection();

    std::shared_ptr< Light > light = std::make_shared< PointLight >( lightPosition );

    m_scene->addLight( light );

    clearSelection();
    selectLight( light );
}

void SceneManager::addSpotLight()
{
    float3 lightPosition = m_camera.getPosition() + m_camera.getDirection();

    std::shared_ptr< Light > light = std::make_shared< SpotLight >( lightPosition, m_camera.getDirection(), MathUtil::pi / 8.0f );

    m_scene->addLight( light );

    clearSelection();
    selectLight( light );
}

void SceneManager::modifySelectedLightsColor( float3 colorChange )
{
    for ( auto& light : m_selectedLights )
        light->setColor( max( float3::ZERO, light->getColor() + colorChange ) );
}

void SceneManager::enableDisableSelectedLights()
{
    if ( m_selectedLights.empty() ) 
        return;

    const bool enable = !m_selectedLights[ 0 ]->isEnabled();

    for ( auto& light : m_selectedLights )
        light->setEnabled( enable );
}

void SceneManager::saveSceneOrSelectedModels()
{
    if ( m_selectedBlockActors.empty() && m_selectedSkeletonActors.empty() ) 
    {
        if ( m_scene && !m_scenePath.empty() )
            saveScene( m_scenePath );

        if ( !m_cameraPath.empty() )
            saveCamera( m_cameraPath );
    } 
    else 
    {
        saveSelectedModels();
    }
}

bool SceneManager::isSelectionEmpty()
{
    return m_selectedBlockActors.empty() && m_selectedSkeletonActors.empty() && m_selectedLights.empty();
}

void SceneManager::selectAll()
{
    m_selectedBlockActors.clear();
    m_selectedSkeletonActors.clear();
    m_selectedLights.clear();

    for ( auto& actor : m_scene->getActors() ) {
        if ( actor->getType() == Actor::Type::BlockActor )
            m_selectedBlockActors.push_back( std::dynamic_pointer_cast<BlockActor>( actor ) );
        else if ( actor->getType() == Actor::Type::SkeletonActor )
            m_selectedSkeletonActors.push_back( std::dynamic_pointer_cast<SkeletonActor>( actor ) );
    }

    for ( auto& light : m_scene->getLights() )
        m_selectedLights.push_back( light );
}

void SceneManager::clearSelection()
{
    m_selectedBlockActors.clear();
    m_selectedSkeletonActors.clear();
    m_selectedLights.clear();
}

bool SceneManager::isActorSelected( std::shared_ptr< Actor > actor )
{
    if ( actor->getType() == Actor::Type::BlockActor )
        return std::find( m_selectedBlockActors.begin(), m_selectedBlockActors.end(), actor ) != m_selectedBlockActors.end();
    else
        return std::find( m_selectedSkeletonActors.begin(), m_selectedSkeletonActors.end(), actor ) != m_selectedSkeletonActors.end();
}

void SceneManager::selectActor( std::shared_ptr< Actor > actor )
{
    if ( !isActorSelected( actor ) )
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            m_selectedBlockActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
        else
            m_selectedSkeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( actor ) );
    }

    recalculateSelectionVolume();
}

void SceneManager::unselectActor( std::shared_ptr< Actor > actor )
{
    if ( actor->getType() == Actor::Type::BlockActor )
    {
        const auto& it = std::find( m_selectedBlockActors.begin(), m_selectedBlockActors.end(), actor );

        if ( it != m_selectedBlockActors.end() )
            m_selectedBlockActors.erase( it );
    }
    else
    {
        const auto& it = std::find( m_selectedSkeletonActors.begin(), m_selectedSkeletonActors.end(), actor );

        if ( it != m_selectedSkeletonActors.end() )
            m_selectedSkeletonActors.erase( it );
    }

    recalculateSelectionVolume();
}

bool SceneManager::isLightSelected( std::shared_ptr< Light > light )
{
    return std::find( m_selectedLights.begin(), m_selectedLights.end(), light ) != m_selectedLights.end();
}

void SceneManager::selectLight( std::shared_ptr< Light > light )
{
    if ( !isLightSelected( light ) )
        m_selectedLights.push_back( light );
}

void SceneManager::unselectLight( std::shared_ptr< Light > light )
{
    const auto& it = std::find( m_selectedLights.begin(), m_selectedLights.end(), light );

    if ( it != m_selectedLights.end() )
        m_selectedLights.erase( it );
}

void SceneManager::selectNext()
{
    std::shared_ptr< Actor > selectedActor;
    std::shared_ptr< Light > selectedLight;

    if ( m_selectedBlockActors.size() == 1 && m_selectedSkeletonActors.empty() && m_selectedLights.empty() )
        selectedActor = m_selectedBlockActors[ 0 ];
    else if ( m_selectedSkeletonActors.size() == 1 && m_selectedBlockActors.empty() && m_selectedLights.empty() )
        selectedActor = m_selectedSkeletonActors[ 0 ];
    else if ( m_selectedLights.size() == 1 && m_selectedBlockActors.empty() && m_selectedSkeletonActors.empty() )
        selectedLight = m_selectedLights[ 0 ];

    if ( selectedActor )
    {
        auto it = m_scene->getActors().find( selectedActor );

        if ( it == m_scene->getActors().end() ) 
            return;

        if ( ++it == m_scene->getActors().end() )
            it = m_scene->getActors().begin();

        clearSelection();

        if ( selectedActor->getType() == Actor::Type::BlockActor )
            m_selectedBlockActors.push_back( std::static_pointer_cast< BlockActor >( *it ) );
        else if ( selectedActor->getType() == Actor::Type::SkeletonActor )
            m_selectedSkeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( *it ) );
    }
    else if ( selectedLight )
    {
        auto it = m_scene->getLights().find( selectedLight );

        if ( it == m_scene->getLights().end() )
            return;

        if ( ++it == m_scene->getLights().end() )
            it = m_scene->getLights().begin();

        clearSelection();

        m_selectedLights.push_back( *it );
    }
}

void SceneManager::selectPrev()
{
}

void SceneManager::deleteSelected()
{
    for ( auto& actor : m_selectedBlockActors )
        m_scene->removeActor( actor );

    for ( auto& actor : m_selectedSkeletonActors )
        m_scene->removeActor( actor );

    for ( auto& light : m_selectedLights ) {
        m_scene->removeLight( light );
    }

    m_selectedBlockActors.clear();
    m_selectedSkeletonActors.clear();
    m_selectedLights.clear();
}

void SceneManager::enableDisableCastingShadowsForSelected()
{
    bool castShadows = true;

    if ( !m_selectedBlockActors.empty() )
        castShadows = !m_selectedBlockActors[ 0 ]->isCastingShadows();
    else if ( !m_selectedSkeletonActors.empty() )
        castShadows = !m_selectedSkeletonActors[ 0 ]->isCastingShadows();
    else if ( !m_selectedLights.empty() )
        castShadows = !m_selectedLights[ 0 ]->isCastingShadows();

    for ( auto& actor : m_selectedBlockActors )
        actor->setCastingShadows( castShadows );

    for ( auto& actor : m_selectedSkeletonActors )
        actor->setCastingShadows( castShadows );

    for ( auto& light : m_selectedLights ) {
        light->setCastingShadows( castShadows );
    }
}

void SceneManager::cloneInstancesOfSelectedActors()
{
    std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
    std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
    newBlockActors.reserve( m_selectedBlockActors.size() );
    newSkeletonActors.reserve( m_selectedSkeletonActors.size() );

    for ( auto& actor : m_selectedBlockActors ) {
        newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); // Clone the actor.
        m_scene->addActor( newBlockActors.back() );
    }

    for ( auto& actor : m_selectedSkeletonActors ) {
        newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); // Clone the actor.
        m_scene->addActor( newSkeletonActors.back() );
    }

    // Select new actors.
    m_selectedBlockActors = std::move( newBlockActors );
    m_selectedSkeletonActors = std::move( newSkeletonActors );
}

void SceneManager::cloneSelectedActors()
{
    std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
    std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
    newBlockActors.reserve( m_selectedBlockActors.size() );
    newSkeletonActors.reserve( m_selectedSkeletonActors.size() );

    for ( auto& actor : m_selectedBlockActors ) {
        newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); // Clone the actor.
        newBlockActors.back()->setModel( std::make_shared< BlockModel >( *actor->getModel() ) ); // Clone it's model.
        m_scene->addActor( newBlockActors.back() );
    }

    for ( auto& actor : m_selectedSkeletonActors ) {
        newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); // Clone the actor.
        newSkeletonActors.back()->setModel( std::make_shared< SkeletonModel >( *actor->getModel() ) ); // Clone it's model.
        m_scene->addActor( newSkeletonActors.back() );
    }
}

void SceneManager::recreateSelectionVolumeMesh()
{
    const int  vertexCount      = 8;
    const int  texcoordSetCount = 1;
    const int  triangleCount    = 12; 
    const bool hasNormals       = false;

    auto mesh = std::make_shared< BlockMesh >( vertexCount, hasNormals, texcoordSetCount, triangleCount );

    auto& vertices  = mesh->getVertices();
    auto& texcoords = mesh->getTexcoords( 0 );
    texcoords.resize( vertexCount );
    auto& triangles = mesh->getTriangles();

    const float3& localMin = m_selectionVolume.getMin();
    const float3& localMax = m_selectionVolume.getMax();

    vertices[ 0 ] = localMin;
    vertices[ 1 ] = float3( localMax.x, localMin.y, localMin.z );
    vertices[ 2 ] = float3( localMax.x, localMin.y, localMax.z );
    vertices[ 3 ] = float3( localMin.x, localMin.y, localMax.z );
    vertices[ 4 ] = float3( localMin.x, localMax.y, localMin.z );
    vertices[ 5 ] = float3( localMax.x, localMax.y, localMin.z );
    vertices[ 6 ] = localMax;
    vertices[ 7 ] = float3( localMin.x, localMax.y, localMax.z );

    for ( auto& texcoord : texcoords)
        texcoord = float2( 0.5f, 0.5f );

    triangles[ 0 ]  = uint3( 0, 4, 5 );
    triangles[ 1 ]  = uint3( 0, 5, 1 );
    triangles[ 2 ]  = uint3( 3, 7, 4 );
    triangles[ 3 ]  = uint3( 3, 4, 0 );
    triangles[ 4 ]  = uint3( 2, 6, 7 );
    triangles[ 5 ]  = uint3( 2, 7, 3 );
    triangles[ 6 ]  = uint3( 1, 5, 6 );
    triangles[ 7 ]  = uint3( 1, 6, 2 );
    triangles[ 8 ]  = uint3( 4, 7, 6 );
    triangles[ 9 ]  = uint3( 4, 6, 5 );
    triangles[ 10 ] = uint3( 3, 0, 1 );
    triangles[ 11 ] = uint3( 3, 1, 2 );

    mesh->recalculateBoundingBox();
    mesh->loadCpuToGpu( *m_device.Get() );

    m_selectionVolumeMesh = mesh;
}

void SceneManager::recalculateSelectionVolume()
{
    float3 worldMin( FLT_MAX, FLT_MAX, FLT_MAX );
    float3 worldMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    for ( auto& actor : m_selectedBlockActors ) 
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        const BoundingBox localBoundingBox = actor->getModel()->getMesh()->getBoundingBox();
        const BoundingBox worldBoundingBox = MathUtil::boundingBoxLocalToWorld( localBoundingBox, actor->getPose() );
            
        worldMin = min( worldMin, worldBoundingBox.getMin() );
        worldMax = max( worldMax, worldBoundingBox.getMax() );
    }

    m_selectionVolume.set( worldMin, worldMax );

    recreateSelectionVolumeMesh();
}

void SceneManager::modifySelectionVolume( const float3 minChange, const float3 maxChange )
{
    minChange;
    maxChange;
}

void SceneManager::selectAllInsideSelectionVolume()
{
    clearSelection();

    for ( auto& actor : m_scene->getActors() ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) 
        {
            const auto& blockActor = std::static_pointer_cast< BlockActor >( actor );

            if ( !blockActor->getModel() || !blockActor->getModel()->getMesh() )
                continue;

            const BoundingBox bbBoxLocal = blockActor->getModel()->getMesh()->getBoundingBox();
            const BoundingBox bbBoxWorld = MathUtil::boundingBoxLocalToWorld( bbBoxLocal, blockActor->getPose() );

            if ( MathUtil::intersectBoundingBoxes( m_selectionVolume, bbBoxWorld ) )
                m_selectedBlockActors.push_back( blockActor );
        }
        else if ( actor->getType() == Actor::Type::SkeletonActor )
        {
            const auto& skeletonActor = std::static_pointer_cast< SkeletonActor >( actor );

            if ( !skeletonActor->getModel() || !skeletonActor->getModel()->getMesh() )
                continue;

            const BoundingBox bbBoxLocal = skeletonActor->getModel()->getMesh()->getBoundingBox();
            const BoundingBox bbBoxWorld = MathUtil::boundingBoxLocalToWorld( bbBoxLocal, skeletonActor->getPose() );

            if ( MathUtil::intersectBoundingBoxes( m_selectionVolume, bbBoxWorld ) )
                m_selectedSkeletonActors.push_back( skeletonActor );
        }
    }
}

void SceneManager::rebuildBoundingBoxAndBVH()
{
    for ( auto& actor : m_selectedBlockActors ) 
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        actor->getModel()->getMesh()->recalculateBoundingBox();
        actor->getModel()->getMesh()->buildBvhTree();

        actor->getModel()->getMesh()->unloadBvhTreeFromGpu();
        actor->getModel()->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
    }
}