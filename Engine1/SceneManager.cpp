#include "SceneManager.h"

#include <algorithm>
#include <functional>

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
#include "MeshUtil.h"
#include "ModelUtil.h"
#include "StringUtil.h"
#include "FileUtil.h"

#include "PointLight.h"
#include "SpotLight.h"

#include "BlockModelImporter.h"

#include "AssetPathManager.h"

using namespace Engine1;
using Microsoft::WRL::ComPtr;

SceneManager::SceneManager( AssetManager& assetManager ) :
    m_assetManager( assetManager ),
    m_scenePath( "Assets/Scenes/new.scene" ),
    m_cameraPath( "Assets/Scenes/new.camera" ),
    m_camera( std::make_shared< FreeCamera >() ),
    m_scene( std::make_shared< Scene >() )
{
    // Setup the camera.
    m_camera->setUp( float3( 0.0f, 1.0f, 0.0f ) );
    m_camera->setPosition( float3( 30.0f, 4.0f, -53.0f ) );
    m_camera->rotate( float3( 0.0f, MathUtil::piHalf, 0.0f ) );
}

void SceneManager::initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    m_device        = device;
    m_deviceContext = deviceContext;
}

std::shared_ptr< FreeCamera >& SceneManager::getCamera()
{
    return m_camera;
}

std::shared_ptr< Scene >& SceneManager::getScene()
{
    return m_scene;
}

const Selection& SceneManager::getSelection()
{
    return m_selection;
}

const std::vector< std::shared_ptr< Light > >& SceneManager::getSelectedLights()
{
    return m_selection.getLights();
}

const std::vector< std::shared_ptr< BlockActor > >& SceneManager::getSelectedBlockActors()
{
    return m_selection.getBlockActors();
}

const std::vector< std::shared_ptr< SkeletonActor > >& SceneManager::getSelectedSkeletonActors()
{
    return m_selection.getSkeletonActors();
}

std::shared_ptr< BlockMesh > SceneManager::getSelectionVolumeMesh()
{
    return m_selectionVolumeMesh;
}

void SceneManager::loadCamera( std::string path )
{
    m_camera = FreeCamera::createFromFile( path );
}

void SceneManager::saveCamera( std::string path )
{
    m_camera->saveToFile( path );
}

void SceneManager::loadScene( std::string path )
{
    std::shared_ptr< std::vector < std::shared_ptr< FileInfo > > > fileInfos;

    std::tie( m_scene, fileInfos ) = Scene::createFromFile( path );

    // Load all assets.
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        m_assetManager.loadAsync( *fileInfo );

    int loadedAssetsCount = 0;

    // Wait for all assets to be loaded.
    const Timer loadingStartTime;
    const float maxLoadingTime = 60.0f;
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos ) {
        const Timer currTime;
        const float loadingTime = (float)Timer::getElapsedTime( currTime, loadingStartTime ) / 1000.0f;
        const float timeout = std::max( 0.0f, maxLoadingTime - loadingTime );

        const auto asset = m_assetManager.getWhenLoaded( fileInfo->getAssetType(), fileInfo->getPath(), fileInfo->getIndexInFile(), timeout );

        if ( asset ) {
            ++loadedAssetsCount;
        }
    }

    OutputDebugStringW( StringUtil::widen( 
        "\n\nSceneManager::loadScene - loaded " 
        + std::to_string( loadedAssetsCount ) 
        + "\\" + std::to_string( fileInfos->size() ) + " assets.\n\n" ).c_str() 
    );

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
                    //throw std::exception( "SceneManager::loadScene - failed to load one of the scene's models." );
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
                    //throw std::exception( "SceneManager::loadScene - failed to load one of the scene's models." );
                }
            }
        }
    }
}

void SceneManager::saveScene( std::string path )
{
    m_scene->saveToFile( path );
}

void SceneManager::loadAsset( std::string filePath, const bool replaceSelected, const bool invertZ, const bool invertVertexWindingOrder, const bool invertUVs )
{
    if ( filePath.empty() )
        throw std::exception( "SceneManager::loadAsset - file of given name not found." );

    const size_t dotIndex = filePath.rfind( "." );
    if ( dotIndex == std::string::npos )
        return;

    std::string filePathWithoutExtension = StringUtil::toLowercase( filePath.substr( 0, dotIndex ) );
    std::string extension = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

    std::array< const std::string, 3 > blockMeshExtensions             = { "dae", "fbx", "blockmesh" };
    std::array< const std::string, 1 > skeletonkMeshExtensions         = { "dae" };
    std::array< const std::string, 9 > textureExtensions               = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff", "tif" };
    std::array< const std::string, 2 > blockModelExtensions            = { "blockmodel", "obj" };
    std::array< const std::string, 1 > skeletonModelExtensions         = { "skeletonmodel" };
    std::array< const std::string, 1 > skeletonAnimationExtensions     = { "xaf" };
    std::array< const std::string, 1 > sceneExtensions                 = { "scene" };
    std::array< const std::string, 1 > cameraExtensions                = { "camera" };
    std::array< const std::string, 1 > cameraAnimationExtensions       = { "cameraanim" };
    std::array< const std::string, 1 > spotlightAnimationExtensions    = { "spotlightanim" };

    bool isBlockMesh          = false;
    bool isSkeletonMesh       = false;
    bool isTexture            = false;
    bool isBlockModel         = false;
    bool isSkeletonModel      = false;
    bool isSkeletonAnimation  = false;
    bool isScene              = false;
    bool isCamera             = false;
    bool isCameraAnimation    = false;
    bool isSpotlightAnimation = false;

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

    for ( const std::string& skeletonAnimationExtension : skeletonAnimationExtensions ) {
        if ( extension.compare( skeletonAnimationExtension ) == 0 )
            isSkeletonAnimation = true;
    }

    for ( const std::string& sceneExtension : sceneExtensions ) {
        if ( extension.compare( sceneExtension ) == 0 )
            isScene = true;
    }

    for ( const std::string& cameraExtension : cameraExtensions ) {
        if ( extension.compare( cameraExtension ) == 0 )
            isCamera = true;
    }

    for ( const std::string& cameraAnimationExtension : cameraAnimationExtensions ) {
        if ( extension.compare( cameraAnimationExtension ) == 0 )
            isCameraAnimation = true;
    }

    for ( const std::string& spotlightAnimationExtension : spotlightAnimationExtensions ) {
        if ( extension.compare( spotlightAnimationExtension ) == 0 )
            isSpotlightAnimation = true;
    }

    float43 pose = float43::IDENTITY;
    pose.setTranslation( m_camera->getPosition() + m_camera->getDirection() );

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
                BlockMeshFileInfo fileInfo( filePath, format, indexInFile, invertZ, invertVertexWindingOrder, invertUVs ); // Note: Convert to right hand coord system.
                std::shared_ptr< BlockMesh > mesh = std::static_pointer_cast<BlockMesh>( m_assetManager.getOrLoad( fileInfo ) );

                if ( !mesh->getBvhTree() )
                    mesh->buildBvhTree();

                if ( !mesh->isInGpuMemory() ) {
                    mesh->loadCpuToGpu( *m_device.Get() );
                    mesh->loadBvhTreeToGpu( *m_device.Get() );
                }

                if ( replaceSelected && !m_selection.isEmpty() ) {
                    // Replace a mesh of an existing model.
                    for ( auto& actor : m_selection.getBlockActors() )
                    {
                        if ( actor->getModel() ) 
                            actor->getModel()->setMesh( mesh );
                    }

                    break; // If replacing a mesh - read only the one with index 0 in file.
                } 
                else 
                {
                    // Add new actor to the scene.
                    auto newActor = std::make_shared< BlockActor >( std::make_shared< BlockModel >(), pose );
                    newActor->getModel()->setMesh( mesh );

                    m_scene->addActor( newActor );

                    m_selection.clear();
                    m_selection.add( newActor );
                }

                // Save mesh to .blockmesh format for future use.
                if ( format != BlockMeshFileInfo::Format::BLOCKMESH )
                {
                    mesh->saveToFile( filePathWithoutExtension + ( indexInFile != 0 ? "_" + std::to_string( indexInFile ) : "" ) + ".blockmesh", BlockMeshFileInfo::Format::BLOCKMESH );

                    // Re-scan all paths to detect newly exported files.
                    AssetPathManager::scanAllPaths();
                }
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
        for ( int indexInFile = 0; true; ++indexInFile ) 
        {
            try {
                SkeletonMeshFileInfo fileInfo( filePath, format, 0, invertZ, invertZ, invertZ ); // Note: Convert to right hand coord system.
                std::shared_ptr<SkeletonMesh> mesh = std::static_pointer_cast<SkeletonMesh>( m_assetManager.getOrLoad( fileInfo ) );
                if ( !mesh->isInGpuMemory() )
                    mesh->loadCpuToGpu( *m_device.Get() );

                // #TODO: add replacing mesh? How to deal with non-matching animation?
                // Add new actor to the scene.
                auto newActor = std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>(), pose );
                newActor->getModel()->setMesh( mesh );
                newActor->resetSkeletonPose();

                m_scene->addActor( newActor );
                m_selection.clear();
            } catch ( ... ) {
                break;
            }
        }
    }

    if ( isTexture ) {
        Texture2DFileInfo::Format format = Texture2DFileInfo::fromExtension( extension );
        Model::TextureType        textureType = Model::textureFileNameToType( filePath );

        // #TODO: THis should be replaced by a call to an utility method - but PixelType is in the wrong class
        // (should be a part of the texture), not texture-file-info.
        Texture2DFileInfo::PixelType pixelType = Texture2DFileInfo::PixelType::UCHAR;
        if ( textureType == Model::TextureType::Albedo ||
             textureType == Model::TextureType::Normal ||
             textureType == Model::TextureType::Emissive ) 
        {
            pixelType = Texture2DFileInfo::PixelType::UCHAR4;
        } 
        else if ( 
            textureType == Model::TextureType::Alpha ||
            textureType == Model::TextureType::Metalness ||
            textureType == Model::TextureType::Roughness ||
            textureType == Model::TextureType::RefractiveIndex ) 
        {
            pixelType = Texture2DFileInfo::PixelType::UCHAR;
        }

        Texture2DFileInfo fileInfo( filePath, format, pixelType );
        std::shared_ptr< Asset > textureAsset = m_assetManager.getOrLoad( fileInfo );

        for ( auto& actor : m_selection.getActors() ) 
        {
            auto      model        = actor->getBaseModel();
            const int textureCount = model->getTextureCount( textureType );

            if ( !replaceSelected && textureCount > 0 && model->getTexture( textureType ) )
                continue;

            // Read first texture color multipliers if available.
            float4 colorMultiplier = float4::ONE;
            if ( textureCount > 0 )
                colorMultiplier = model->getTextureColorMultiplier( textureType );

            model->removeAllTextures( textureType );
            model->addTexture( textureType, textureAsset, 0, colorMultiplier );
        }
    }

    if ( isBlockModel ) 
    {
        BlockModelFileInfo::Format format = BlockModelFileInfo::Format::BLOCKMODEL;

        if ( extension.compare( "obj" ) == 0 ) 
            format = BlockModelFileInfo::Format::OBJ;

        if ( format == BlockModelFileInfo::Format::BLOCKMODEL )
        {
            BlockModelFileInfo fileInfo( filePath, format, 0 );
            std::shared_ptr<BlockModel> model = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( fileInfo ) );

            if ( !model->isInGpuMemory() )
                model->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

            // Add new actor to the scene.
            auto newActor = std::make_shared< BlockActor >( model, pose );
            m_scene->addActor( newActor );
                
            m_selection.clear();
            m_selection.add( newActor );
        }
        else
        { // Import block models from an external file format.

            auto models = BlockModelImporter::import( filePath, invertZ, invertVertexWindingOrder, invertUVs );

            for ( int modelIdx = 0; modelIdx < models.size(); ++modelIdx )
            {
                auto& model = models[ modelIdx ];

                if ( !model->getMesh() ) 
                    continue;

                model->getMesh()->recalculateBoundingBox();
                model->getMesh()->buildBvhTree();

                // Save model and mesh to .blockmodel/.blockmesh format for future use.
                std::string meshPath = filePathWithoutExtension + ( modelIdx != 0 ? "_" + std::to_string( modelIdx ) : "" ) + ".blockmesh";
                std::string modelPath = filePathWithoutExtension + ( modelIdx != 0 ? "_" + std::to_string( modelIdx ) : "" ) + ".blockmodel";
                
                model->getMesh()->saveToFile( meshPath, BlockMeshFileInfo::Format::BLOCKMESH );

                model->getMesh()->getFileInfo().setPath( meshPath );
                model->getMesh()->getFileInfo().setFormat( BlockMeshFileInfo::Format::BLOCKMESH );
                model->getMesh()->getFileInfo().setIndexInFile( 0 );

                model->saveToFile( modelPath );

                OutputDebugStringW( StringUtil::widen( 
                    "SceneManager::loadAsset - imported a model and re-saved as \"" 
                    + modelPath + "\n"  
                ).c_str( ) );
            }

            // Re-scan all paths to detect newly exported files.
            AssetPathManager::scanAllPaths();
        }
    }

    if ( isSkeletonModel ) {
        SkeletonModelFileInfo fileInfo( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, 0 );
        std::shared_ptr<SkeletonModel> model = std::static_pointer_cast<SkeletonModel>( m_assetManager.getOrLoad( fileInfo ) );
        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

        // Add new actor to the scene.
        auto newActor = std::make_shared< SkeletonActor >( model, pose );
        m_scene->addActor( newActor );

        m_selection.clear();
        m_selection.add( newActor );
    }

    if ( isSkeletonAnimation ) {
        for ( auto& actor : m_selection.getSkeletonActors() ) {
            if ( actor->getModel() && actor->getModel()->getMesh() ) {
                SkeletonMeshFileInfo&     referenceMeshFileInfo = actor->getModel()->getMesh()->getFileInfo();
                SkeletonAnimationFileInfo fileInfo( filePath, SkeletonAnimationFileInfo::Format::XAF, referenceMeshFileInfo, false );

                std::shared_ptr< SkeletonAnimation > animation = std::static_pointer_cast<SkeletonAnimation>( m_assetManager.getOrLoad( fileInfo ) );

                if ( animation )
                    actor->startAnimation( animation );
            }
        }
    }

    if ( ( isBlockMesh || isTexture ) && m_selection.containsOnlyOneBlockActor() )
    {
        auto& actor = m_selection.getBlockActors()[ 0 ];
        
        if ( actor->getModel() && actor->getModel()->isInCpuMemory() )
            actor->getModel()->saveToFile( "Assets/Models/new.blockmodel" );
    }

    if ( ( isSkeletonMesh || isTexture ) && m_selection.containsOnlyOneSkeletonActor() )
    {
        auto& actor = m_selection.getBlockActors()[ 0 ];

        if ( actor->getModel() && actor->getModel()->isInCpuMemory() )
            actor->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );
    }

    if ( isScene ) {
        loadScene( filePath );
        m_scenePath = filePath;
    }

    if ( isCamera ) {
        loadCamera( filePath );
        m_cameraPath = filePath;
    }

    if ( isCameraAnimation ) {
        m_cameraAnimator.loadAnimationFromFile( m_camera, filePath );
    }

    if ( isSpotlightAnimation && getSelection().containsOnlyOneSpotLight() ) {
        m_spotlightAnimator.loadAnimationFromFile( getSelection().getSpotLights().front(), filePath );
    }
}

std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light > >
SceneManager::pickActorOrLight( const float2& targetPixel, const float screenWidth, const float screenHeight, const float fieldOfView )
{
    float43 cameraPose;
    cameraPose.setRow1( m_camera->getRight() );
    cameraPose.setRow2( m_camera->getUp() );
    cameraPose.setRow3( m_camera->getDirection() );
    cameraPose.setTranslation( m_camera->getPosition() );

    const float2 screenDimensions( screenWidth, screenHeight );

    const float3 rayOriginWorld = m_camera->getPosition();
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
    if ( m_selection.getBlockActors().empty() )
        return;

    try {
        std::vector< std::shared_ptr< BlockModel > > models;
        std::vector< float43 >                       transforms;
        models.reserve( m_selection.getBlockActors().size() );
        transforms.reserve( m_selection.getBlockActors().size() );

        const float43 firstTransformInv = m_selection.getBlockActors().front()->getPose().getScaleOrientationTranslationInverse();

        for ( auto& actor : m_selection.getBlockActors() )
        {
            models.push_back( actor->getModel() );
            transforms.push_back( actor->getPose() * firstTransformInv );
        }

        std::shared_ptr< BlockModel > mergedModel = ModelUtil::mergeModels( models, transforms, *m_device.Get() );

        if ( mergedModel->getMesh() ) {
            mergedModel->getMesh()->recalculateBoundingBox();

            if ( !mergedModel->getMesh()->getBvhTree() ) {
                mergedModel->getMesh()->buildBvhTree();
                mergedModel->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
            }
        }

        if ( !mergedModel->isInGpuMemory() )
            mergedModel->loadCpuToGpu( *m_device.Get(), *m_deviceContext.Get() );

        // Create a name for the merged model.
        std::string mergedFullName = "merged";
        for (auto& model : models )
        {
            auto modelName = FileUtil::getFileNameFromPath( model->getFileInfo().getPath() );

            // Remove file extension from name.
            modelName = modelName.substr( 0, modelName.rfind(".blockmodel") );

            mergedFullName += "_" + modelName;
        }

        std::string mergedHashedName = "merged_" + std::to_string( std::hash< std::string >{}( mergedFullName ) );

        auto firstModelPath = models.front()->getFileInfo().getPath();
        auto firstModelName = FileUtil::getFileNameFromPath( firstModelPath );
        auto folderName     = StringUtil::replaceSubstring( firstModelPath, firstModelName, "" );
        folderName = StringUtil::replaceSubstring( folderName, "Assets\\Models\\", "" );
        folderName = StringUtil::replaceAllSubstrings( folderName, "\\", "" );

        { // Save merged model, mesh and textures to files.
            std::string mergedMeshPath    = "Assets\\Meshes\\" + folderName + "\\" + mergedHashedName + ".obj";
            std::string mergedMeshPath2   = "Assets\\Meshes\\" + folderName + "\\" + mergedHashedName + ".blockmesh";
            std::string mergedTexturePath = "Assets\\Textures\\" + folderName + "\\" + mergedHashedName;
            std::string mergedModelPath   = "Assets\\Models\\" + folderName + "\\" + mergedHashedName + ".blockmodel";

            if ( mergedModel->getMesh() ) 
            {
                mergedModel->getMesh()->getFileInfo().setPath( mergedMeshPath2 );
                mergedModel->getMesh()->getFileInfo().setFormat( BlockMeshFileInfo::Format::BLOCKMESH );
                mergedModel->getMesh()->getFileInfo().setIndexInFile( 0 );

                mergedModel->getMesh()->saveToFile( mergedMeshPath, BlockMeshFileInfo::Format::OBJ );
                mergedModel->getMesh()->saveToFile( mergedMeshPath2, BlockMeshFileInfo::Format::BLOCKMESH );
            }

            int textureIndex = 0;
            for ( auto& texture : mergedModel->getAlphaTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_AL.tiff";

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getEmissiveTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_E.png";
                
                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getAlbedoTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_A.png";

                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getMetalnessTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_M.tiff";
                
                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getRoughnessTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_R.tiff";
                
                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getNormalTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_N.png";
                
                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );
            }

            textureIndex = 0;
            for ( auto& texture : mergedModel->getRefractiveIndexTextures() ) 
            {
                std::string path = mergedTexturePath + "_" + std::to_string( textureIndex++ ) + "_I.tiff";
                
                texture.getTexture()->getFileInfo().setPath( path );
                texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
                texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );

                texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );
            }

            // Set file info for the merged model, so the scene knows in which file it's stored.
            BlockModelFileInfo mergedModelFileInfo( mergedModelPath, BlockModelFileInfo::Format::BLOCKMODEL, 0 );
            mergedModel->setFileInfo( mergedModelFileInfo );

            mergedModel->saveToFile( mergedModelPath );
        }

        const auto& pose = m_selection.getBlockActors().front()->getPose();

        // Add new actor to the scene.
        //m_selectedBlockActors.clear();
        auto newActor = std::make_shared< BlockActor >( mergedModel, pose );
        m_scene->addActor( newActor );
    } catch ( std::exception& e ) {
        OutputDebugStringW( StringUtil::widen( e.what() + std::string( "\n" ) ).c_str() );
    }
}

void SceneManager::saveSelectedModels()
{
    for ( auto& actor : m_selection.getBlockActors() ) {
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

    for ( const auto& actor : m_selection.getBlockActors() ) 
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        vertexCount += (int)actor->getModel()->getMesh()->getVertices().size();
        triangleCount += (int)actor->getModel()->getMesh()->getTriangles().size();
    }

    for ( const auto& actor : m_selection.getSkeletonActors() ) 
    {
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
    float3 lightPosition = m_camera->getPosition() + m_camera->getDirection();

    std::shared_ptr< Light > light = std::make_shared< PointLight >( lightPosition );

    m_scene->addLight( light );

    clearSelection();
    selectLight( light );
}

void SceneManager::addSpotLight()
{
    float3 lightPosition = m_camera->getPosition() + m_camera->getDirection();

    std::shared_ptr< Light > light = std::make_shared< SpotLight >( lightPosition, m_camera->getDirection(), MathUtil::pi / 8.0f );

    m_scene->addLight( light );

    clearSelection();
    selectLight( light );
}

void SceneManager::modifySelectedLightsColor( float3 colorChange )
{
    const auto& selectedLights = m_selection.getLights();
    for ( auto& light : selectedLights )
        light->setColor( max( float3::ZERO, light->getColor() + colorChange ) );
}

void SceneManager::enableDisableSelectedLights()
{
    const auto& selectedLights = m_selection.getLights();

    if ( selectedLights.empty() ) 
        return;

    const bool enable = !selectedLights[ 0 ]->isEnabled();

    for ( auto& light : selectedLights )
        light->setEnabled( enable );
}

void SceneManager::saveSceneOrSelectedModels()
{
    if ( m_selection.isEmpty() || m_selection.containsOnlyLights() ) 
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
    return m_selection.isEmpty();
}

void SceneManager::selectAll()
{
    m_selection.replace( m_scene->getActorsVec() );
    m_selection.add( m_scene->getLightsVec() );
}

void SceneManager::clearSelection()
{
    m_selection.clear();
}

bool SceneManager::isActorSelected( const std::shared_ptr< Actor >& actor )
{
    return m_selection.contains( actor );
}

void SceneManager::selectActor( const std::shared_ptr< Actor >& actor )
{
    const bool selected = m_selection.add( actor );

    if ( selected )
        recalculateSelectionVolume();
}

void SceneManager::unselectActor( const std::shared_ptr< Actor >& actor )
{
    const bool unselected = m_selection.remove( actor );

    if ( unselected )
        recalculateSelectionVolume();
}

bool SceneManager::isLightSelected( const std::shared_ptr< Light >& light )
{
    return m_selection.contains( light );
}

void SceneManager::selectLight( const std::shared_ptr< Light >& light )
{
    m_selection.add( light );
}

void SceneManager::unselectLight( const std::shared_ptr< Light >& light )
{
    m_selection.remove( light );
}

void SceneManager::selectNext()
{
    // #TODO: Should fix - it fails in many cases.

    std::shared_ptr< Actor > selectedActor;
    std::shared_ptr< Light > selectedLight;

    if ( m_selection.containsOnlyOneBlockActor() )
        selectedActor = m_selection.getBlockActors()[ 0 ];
    else if (  m_selection.containsOnlyOneSkeletonActor() )
        selectedActor = m_selection.getSkeletonActors()[ 0 ];
    else if ( m_selection.containsOnlyOneLight() )
        selectedLight = m_selection.getLights()[ 0 ];

    if ( selectedActor )
    {
        auto it = m_scene->getActors().find( selectedActor );

        if ( it == m_scene->getActors().end() ) 
            return;

        if ( ++it == m_scene->getActors().end() )
            it = m_scene->getActors().begin();

        clearSelection();

        m_selection.add( selectedActor );
    }
    else if ( selectedLight )
    {
        auto it = m_scene->getLights().find( selectedLight );

        if ( it == m_scene->getLights().end() )
            return;

        if ( ++it == m_scene->getLights().end() )
            it = m_scene->getLights().begin();

        clearSelection();

        m_selection.add( selectedLight );
    }
}

void SceneManager::selectPrev()
{
    // #TODO: Shoudl fix - it fails in many cases.

    std::shared_ptr< Actor > selectedActor;
    std::shared_ptr< Light > selectedLight;

    if ( m_selection.containsOnlyOneBlockActor() )
        selectedActor = m_selection.getBlockActors().front();
    else if ( m_selection.containsOnlyOneSkeletonActor() )
        selectedActor = m_selection.getSkeletonActors().front();
    else if ( m_selection.containsOnlyOneLight() )
        selectedLight = m_selection.getLights().front();

    if ( selectedActor ) 
    {
        auto it = m_scene->getActors().find( selectedActor );

        if ( it == m_scene->getActors().begin() )
            it = m_scene->getActors().end();

        --it;

        clearSelection();

        if ( selectedActor->getType() == Actor::Type::BlockActor )
            m_selection.add( std::static_pointer_cast<BlockActor>( *it ) );
        else if ( selectedActor->getType() == Actor::Type::SkeletonActor )
            m_selection.add( std::static_pointer_cast<SkeletonActor>( *it ) );
    } 
    else if ( selectedLight ) 
    {
        auto it = m_scene->getLights().find( selectedLight );

        if ( it == m_scene->getLights().begin() )
            it = m_scene->getLights().end();

        --it;

        clearSelection();

        m_selection.add( *it );
    }
}

void SceneManager::deleteSelected()
{
    for ( auto& actor : m_selection.getActors() )
        m_scene->removeActor( actor );

    for ( auto& light : m_selection.getLights() )
        m_scene->removeLight( light );

    m_selection.clear();
}

void SceneManager::enableDisableCastingShadowsForSelected()
{
    bool castShadows = true;

    if ( !m_selection.getBlockActors().empty() )
        castShadows = !m_selection.getBlockActors().front()->isCastingShadows();
    else if ( !m_selection.getSkeletonActors().empty() )
        castShadows = !m_selection.getSkeletonActors().front()->isCastingShadows();
    else if ( !m_selection.getLights().empty() )
        castShadows = !m_selection.getLights().front()->isCastingShadows();

    for ( auto& actor : m_selection.getActors() )
        actor->setCastingShadows( castShadows );

    for ( auto& light : m_selection.getLights() )
        light->setCastingShadows( castShadows );
}

void SceneManager::cloneInstancesOfSelectedActors()
{
    std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
    std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
    newBlockActors.reserve( m_selection.getBlockActors().size() );
    newSkeletonActors.reserve( m_selection.getSkeletonActors().size() );

    for ( auto& actor : m_selection.getBlockActors() ) 
    {
        // Clone the actor.
        newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); 
        m_scene->addActor( newBlockActors.back() );
    }

    for ( auto& actor : m_selection.getSkeletonActors() ) 
    {
        // Clone the actor.
        newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); 
        m_scene->addActor( newSkeletonActors.back() );
    }

    // Select new actors.
    m_selection.add( newBlockActors );
    m_selection.add( newSkeletonActors );
}

void SceneManager::cloneSelectedActorsAndLights()
{
    std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
    std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
    newBlockActors.reserve( m_selection.getBlockActors().size() );
    newSkeletonActors.reserve( m_selection.getSkeletonActors().size() );

    for ( auto& actor : m_selection.getBlockActors() ) 
    {
        newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); // Clone the actor.
        newBlockActors.back()->setModel( std::make_shared< BlockModel >( *actor->getModel() ) ); // Clone it's model.
        m_scene->addActor( newBlockActors.back() );
    }

    for ( auto& actor : m_selection.getSkeletonActors() ) 
    {
        newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); // Clone the actor.
        newSkeletonActors.back()->setModel( std::make_shared< SkeletonModel >( *actor->getModel() ) ); // Clone it's model.
        m_scene->addActor( newSkeletonActors.back() );
    }

    for ( auto& light : m_selection.getLights() )
    {
        if ( light->getType() == Light::Type::PointLight ) {
            m_scene->addLight( std::make_shared< PointLight >( static_cast< PointLight& >( *light ) ) );
        } else if ( light->getType() == Light::Type::SpotLight ) {
            m_scene->addLight( std::make_shared< SpotLight >( static_cast< SpotLight& >( *light ) ) );
        }
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

    for ( auto& actor : m_selection.getBlockActors() ) 
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
                m_selection.add( blockActor );
        }
        else if ( actor->getType() == Actor::Type::SkeletonActor )
        {
            const auto& skeletonActor = std::static_pointer_cast< SkeletonActor >( actor );

            if ( !skeletonActor->getModel() || !skeletonActor->getModel()->getMesh() )
                continue;

            const BoundingBox bbBoxLocal = skeletonActor->getModel()->getMesh()->getBoundingBox();
            const BoundingBox bbBoxWorld = MathUtil::boundingBoxLocalToWorld( bbBoxLocal, skeletonActor->getPose() );

            if ( MathUtil::intersectBoundingBoxes( m_selectionVolume, bbBoxWorld ) )
                m_selection.add( skeletonActor );
        }
    }
}

Animator< SpotLight >& SceneManager::getLightAnimator()
{
    return m_spotlightAnimator;
}

Animator< FreeCamera >& SceneManager::getCameraAnimator()
{
    return m_cameraAnimator;
}

Animator< BlockActor >& SceneManager::getActorAnimator()
{
    return m_actorAnimator;
}

Animator< BlockModel >& SceneManager::getModelAnimator()
{
    return m_modelAnimator;
}

void SceneManager::rebuildBoundingBoxAndBVH()
{
    for ( auto& actor : m_selection.getBlockActors() ) 
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        actor->getModel()->getMesh()->recalculateBoundingBox();
        actor->getModel()->getMesh()->buildBvhTree();

        actor->getModel()->getMesh()->unloadBvhTreeFromGpu();
        actor->getModel()->getMesh()->loadBvhTreeToGpu( *m_device.Get() );
    }
}

void SceneManager::flipTexcoordsVerticallyAndResaveMesh()
{
    for ( auto& actor : m_selection.getBlockActors() ) 
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        auto mesh = actor->getModel()->getMesh();

        MeshUtil::flipTexcoordsVertically( *mesh );

        mesh->loadCpuToGpu( *m_device.Get(), true );

        if ( !mesh->getFileInfo().getPath().empty() )
            mesh->saveToFile( mesh->getFileInfo().getPath(), mesh->getFileInfo().getFormat() );
    }
}

void SceneManager::flipTangentsAndResaveMesh()
{
    for ( auto& actor : m_selection.getBlockActors() ) {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        auto mesh = actor->getModel()->getMesh();

        MeshUtil::flipTangents( *mesh );

        mesh->loadCpuToGpu( *m_device.Get(), true );

        if ( !mesh->getFileInfo().getPath().empty() )
            mesh->saveToFile( mesh->getFileInfo().getPath(), mesh->getFileInfo().getFormat() );
    }
}

void SceneManager::flipNormalsAndResaveMesh()
{
    for ( auto& actor : m_selection.getBlockActors() ) {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        auto mesh = actor->getModel()->getMesh();

        MeshUtil::flipNormals( *mesh );

        mesh->loadCpuToGpu( *m_device.Get(), true );

        if ( !mesh->getFileInfo().getPath().empty() )
            mesh->saveToFile( mesh->getFileInfo().getPath(), mesh->getFileInfo().getFormat() );
    }
}

void SceneManager::invertVertexWindingOrderAndResaveMesh()
{
    for ( auto& actor : m_selection.getBlockActors() ) {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        auto mesh = actor->getModel()->getMesh();

        MeshUtil::invertVertexWindingOrder( *mesh );

        mesh->loadCpuToGpu( *m_device.Get(), true );

        if ( !mesh->getFileInfo().getPath().empty() )
            mesh->saveToFile( mesh->getFileInfo().getPath(), mesh->getFileInfo().getFormat() );
    }
}