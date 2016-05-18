#include "AssetManager.h"

#include <climits>
#include <thread>
#include <d3d11.h>

#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

#include "StringUtil.h"

#include "TextFile.h"
#include "BinaryFile.h"

#include "SkeletonAnimationFileInfo.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

AssetManager::AssetManager() 
{}

AssetManager::~AssetManager() 
{
	executeThreads = false;
	assetsToLoadFromDiskNotEmpty.notify_all();
	assetsToLoadNotEmpty.notify_all();

	loadingFromDiskThread.join();

	for ( std::vector<std::thread>::iterator thread = loadingThreads.begin(); thread != loadingThreads.end(); ++thread ) {
		( *thread ).join();
	}
}

void AssetManager::initialize( int loadingThreadCount, ComPtr< ID3D11Device > device )
{
    if ( loadingThreadCount <= 0 ) 
        throw std::exception( "AssetManager::AssetManager - Number of loading threads has to be greater than 0." );

    m_device = device;

	executeThreads = true;

	// Create thread to load assets from disk.
	loadingFromDiskThread = std::thread( &AssetManager::loadAssetsFromDisk, this );

	// Create threads to load assets.
	for ( int i = 0; i < loadingThreadCount; ++i ) {
		loadingThreads.push_back( std::thread( &AssetManager::loadAssets, this ) );
	}
}

void AssetManager::load( const FileInfo& fileInfo )
{
    std::string id = getId( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() );

	{ // Check if asset was loaded already or is in the course of loading.
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		if ( assets.count( id ) != 0 ) 
			throw std::exception( "AssetManager::load - Asset is already loaded or in the course of loading." );
		
		assets.insert( id );
	}

	try {
		std::shared_ptr<Asset> asset = createFromFile( fileInfo );

        { // Load sub-assets or wait for the sub-assets to be loaded and swap empty sub-assets with loaded sub-assets.
            std::vector<std::shared_ptr<Asset>> subAssets = asset->getSubAssets( );
            for ( std::shared_ptr<Asset>& subAsset : subAssets ) {
                if ( !subAsset->getFileInfo().getPath().empty() ) {
                    std::shared_ptr<Asset> newSubAsset = getOrLoad( subAsset->getFileInfo( ) );

                    asset->swapSubAsset( subAsset, newSubAsset );
                }
            }
        }

		{
			std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );
			loadedAssets.insert( std::make_pair( id, asset ) );
		}
	} catch ( .../*std::exception& ex*/ ) {
		// If asset failed to load - remove it from assets.
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		assets.erase( id );
		// Re-throw the exception.
		throw;
	}
}

void AssetManager::loadAsync( const FileInfo& fileInfo )
{
    std::string id = getId( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() );

	{ // Check if this asset was loaded already or is in the course of loading.
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		if ( assets.count( id ) != 0 ) 
			throw std::exception( "AssetManager::loadAsync - Asset is already loaded or in the course of loading." );

		assets.insert( id );
	}

	{ // Add the asset to the list of assets to load from disk - lock mutex.
		std::unique_lock<std::mutex> assetsToLoadFromDiskLock( assetsToLoadFromDiskMutex );

		assetsToLoadFromDisk.push_back( fileInfo.clone() );
	}

	// Resume thread which loads assets from disk.
	assetsToLoadFromDiskNotEmpty.notify_one();
}

bool AssetManager::isLoaded( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	//TODO: should I lock loadedAssets before searching through it?
    return (loadedAssets.find( id ) != loadedAssets.end( ));
}

bool AssetManager::isLoadedOrLoading( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	std::lock_guard<std::mutex> assetsLock( assetsMutex );
    return (assets.count( id ) != 0);
}

std::shared_ptr<Asset> AssetManager::get( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );

    std::unordered_map< std::string, std::shared_ptr<Asset> >::iterator it = loadedAssets.find( id );

	if ( it == loadedAssets.end() )
		return nullptr;

	return it->second;
}

std::shared_ptr<Asset> AssetManager::getOrLoad( const FileInfo& fileInfo )
{
    const float timeout = 60.0f;

    if ( !isLoadedOrLoading( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ) )
        load( fileInfo );

    return getWhenLoaded( fileInfo.getAssetType(), fileInfo.getPath( ), fileInfo.getIndexInFile( ), timeout );
}

std::shared_ptr<Asset> AssetManager::getWhenLoaded( Asset::Type type, std::string path, const int indexInFile, const float timeout )
{
    std::string id = getId( type, path, indexInFile );

	std::unordered_map< std::string, std::shared_ptr<Asset> >::iterator it;

	const std::chrono::steady_clock::time_point timoutTime = std::chrono::steady_clock::now( ) + std::chrono::microseconds( (long long)( timeout / 0.000001f ) );

	std::unique_lock<std::mutex> loadedAssetsLock( loadedAssetsMutex );

	for (;;) {
		// Check if the asset is loaded.
        it = loadedAssets.find( id );
		if ( it != loadedAssets.end() )
			return it->second;

		// Wait until some asset finish loading or timeout.
		std::_Cv_status status = assetLoaded.wait_until( loadedAssetsLock, timoutTime );

		// Exit method on timeout.
		if ( status == std::cv_status::timeout )
			return nullptr;
	}
}

void AssetManager::loadAssetsFromDisk() {
	std::shared_ptr< const FileInfo > fileInfo = nullptr;
	std::shared_ptr< std::vector<char> > fileData = nullptr;

	for (;;) {
		fileInfo = nullptr;
		fileData = nullptr;

		{ // Check if there is any asset to load and if so, get it - hold lock.
			std::unique_lock<std::mutex> assetsToLoadFromDiskLock( assetsToLoadFromDiskMutex );

			// Wait until there are some assets to load from disk.
			assetsToLoadFromDiskNotEmpty.wait( assetsToLoadFromDiskLock, [this]() { return !assetsToLoadFromDisk.empty() || !executeThreads; } );

			// Terminate thread if requested.
			if ( !executeThreads ) 
				return;

			// Get first asset from the list
			fileInfo = assetsToLoadFromDisk.front( );
			// Remove asset from the list.
			assetsToLoadFromDisk.pop_front();
		}

		OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssetsFromDisk - loading \"" + fileInfo->getPath( ) + "\"\n" ).c_str( ) );

		// Load file from disk.
		try {
			if ( fileInfo->getFileType() == FileInfo::FileType::Textual )
				fileData = TextFile::load( fileInfo->getPath() );
			else if ( fileInfo->getFileType() == FileInfo::FileType::Binary )
				fileData = BinaryFile::load( fileInfo->getPath() );

			OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssetsFromDisk - loaded \"" + fileInfo->getPath( ) + "\"\n" ).c_str( ) );

		} catch ( .../*std::exception& ex*/ ) {
            std::string id = getId( fileInfo->getAssetType( ), fileInfo->getPath( ), fileInfo->getIndexInFile( ) );

			// If asset failed to load - remove it from assets.
			std::lock_guard<std::mutex> assetsLock( assetsMutex );
			assets.erase( id );

			OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssetsFromDisk - failed to load \"" + fileInfo->getPath( ) + "\"\n" ).c_str( ) );

			//TODO: handle this error - do some callback for ex.
            continue;
		}

		{ // Add asset to list of assets to load - hold mutex.
			std::lock_guard<std::mutex> assetsToLoadLock( assetsToLoadMutex );
			// Add asset to list of assets to load.
			assetsToLoad.push_back( AssetToLoad( fileInfo, fileData ) );
		}

		// Resume one of threads which load assets.
		assetsToLoadNotEmpty.notify_one();
	}
}

void AssetManager::loadAssets() 
{
	for (;;) {
		AssetToLoad assetToLoad;

		{ // Check if there is any asset to load and if so, get it - hold lock.
			std::unique_lock<std::mutex> assetsToLoadLock( assetsToLoadMutex );

			// Wait until there are some assets to load.
			assetsToLoadNotEmpty.wait( assetsToLoadLock, [this]() { return !assetsToLoad.empty() || !executeThreads; } );

			// Terminate thread if requested.
			if ( !executeThreads ) return;

			// Get first asset from the list.
			assetToLoad = assetsToLoad.front( );
			// Remove asset from the list.
			assetsToLoad.pop_front();
		}

		OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssets - loading \"" + assetToLoad.fileInfo->getPath( ) + "\"\n" ).c_str( ) );

		std::shared_ptr<Asset> asset = nullptr;

		// Load asset and sub-assets.
		try {
			asset = createFromMemory( *assetToLoad.fileInfo, *assetToLoad.fileData );

            { // Load sub-assets if needed.
                std::vector<std::shared_ptr<Asset>> subAssets = asset->getSubAssets();
                for ( std::shared_ptr<Asset>& subAsset : subAssets ) {
                    if ( !subAsset->getFileInfo().getPath().empty() )
                        loadAsync( subAsset->getFileInfo() ); //#TODO: Should load with the highest priority.
                }

                // Wait for the sub-assets to be loaded and swap empty sub-assets with loaded sub-assets.
                const float subAssetLoadTimeout = 60.0f;
                for ( std::shared_ptr<Asset>& subAsset : subAssets ) {
                    if ( !subAsset->getFileInfo().getPath().empty() ) {
                        std::shared_ptr<Asset> newSubAsset = getWhenLoaded( subAsset->getFileInfo( ).getAssetType(), subAsset->getFileInfo( ).getPath( ), subAsset->getFileInfo( ).getIndexInFile( ), subAssetLoadTimeout );

                        asset->swapSubAsset( subAsset, newSubAsset );
                    }
                }
            }

			OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssets - loaded \"" + assetToLoad.fileInfo->getPath( ) + "\"\n" ).c_str( ) );
		} catch ( .../*std::exception& ex*/ ) {
			// Asset failed to load - remove it from assets.
			std::lock_guard<std::mutex> assetsLock( assetsMutex );
			assets.erase( assetToLoad.fileInfo->getPath() );

			OutputDebugStringW( StringUtil::widen( "AssetManager::loadAssets - failed to load \"" + assetToLoad.fileInfo->getPath( ) + "\"\n" ).c_str( ) );

			//TODO: handle this error - do some callback for ex.
            continue;
		}

		{ // Add asset to a list of assets - hold lock.
			std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );
			// Add asset to a list of assets.

			//#TODO: handle paths for files with multiple meshes inside.
            std::string id = getId( assetToLoad.fileInfo->getAssetType( ), assetToLoad.fileInfo->getPath( ), assetToLoad.fileInfo->getIndexInFile() );
			loadedAssets.insert( std::make_pair( id, asset ) );
		}

		// Notify 'getWhenLoaded' method that an asset has just been loaded.
		assetLoaded.notify_all();
	}
}

std::shared_ptr<Asset> AssetManager::createFromFile( const FileInfo& fileInfo )
{
	switch ( fileInfo.getAssetType() )  
	{
		case Asset::Type::BlockModel:
			return BlockModel::createFromFile( static_cast<const BlockModelFileInfo&>( fileInfo ), false, *m_device.Get() );
		case Asset::Type::BlockMesh:
			return BlockMesh::createFromFile( static_cast<const BlockMeshFileInfo&>( fileInfo ) );
		case Asset::Type::SkeletonMesh:
			return SkeletonMesh::createFromFile( static_cast<const SkeletonMeshFileInfo&>( fileInfo ) );
		case Asset::Type::SkeletonAnimation:
		{
			const SkeletonAnimationFileInfo& animFileInfo = static_cast<const SkeletonAnimationFileInfo&>( fileInfo );

			std::shared_ptr<const SkeletonMesh> referenceMesh;
			{ // Get or load the reference mesh.
                const bool meshLoadedOrLoading = isLoadedOrLoading( animFileInfo.getMeshFileInfo( ).getAssetType(), animFileInfo.getMeshFileInfo( ).getPath( ), animFileInfo.getMeshFileInfo( ).getIndexInFile( ) );
				if ( !meshLoadedOrLoading )
					loadAsync( animFileInfo.getMeshFileInfo() );

				const float loadTimeout = 60.0f;
                std::shared_ptr<const Asset> mesh = getWhenLoaded( animFileInfo.getMeshFileInfo( ).getAssetType(), animFileInfo.getMeshFileInfo( ).getPath( ), animFileInfo.getMeshFileInfo( ).getIndexInFile( ), loadTimeout );
				referenceMesh = mesh->getType() == Asset::Type::SkeletonMesh ? std::static_pointer_cast<const SkeletonMesh>( mesh ) : nullptr;
			}

			if ( referenceMesh )
				return SkeletonAnimation::createFromFile( animFileInfo.getPath(), animFileInfo.getFormat(), *referenceMesh, animFileInfo.getInvertZCoordinate() );
			else
				throw std::exception( "AssetManager::createFromFile - failed to load reference mesh for the animation." );
		}
		case Asset::Type::Texture2D:
        {
            const Texture2DFileInfo& textureFileInfo = static_cast<const Texture2DFileInfo&>( fileInfo );
            if ( textureFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR4 )
            {
			    return std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >
                    ( *m_device.Get(), textureFileInfo, true, true, true, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
            }
            else if ( textureFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR )
            {
                return std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >
                    ( *m_device.Get(), textureFileInfo, true, true, true, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
            }
        }
		default:
			throw std::exception( "AssetManager::createFromFile - asset type not yet supported." );
	}
}

std::shared_ptr<Asset> AssetManager::createFromMemory( const FileInfo& fileInfo, const std::vector<char>& fileData )
{
	switch ( fileInfo.getAssetType() ) {
		case Asset::Type::BlockModel:
		{
			const BlockModelFileInfo& modelFileInfo = static_cast<const BlockModelFileInfo&>( fileInfo );
			std::shared_ptr<BlockModel> model = BlockModel::createFromMemory( fileData.cbegin(), modelFileInfo.getFormat( ), false, *m_device.Get() );
            model->setFileInfo( modelFileInfo );

            return model;
		}
		case Asset::Type::SkeletonModel:
		{
			const SkeletonModelFileInfo& modelFileInfo = static_cast<const SkeletonModelFileInfo&>( fileInfo );
            std::shared_ptr<SkeletonModel> model = SkeletonModel::createFromMemory( fileData.cbegin(), modelFileInfo.getFormat( ), false, *m_device.Get() );
            model->setFileInfo( modelFileInfo );

            return model;
		}
		case Asset::Type::BlockMesh:
		{
			const BlockMeshFileInfo& meshFileInfo = static_cast<const BlockMeshFileInfo&>( fileInfo );
            std::shared_ptr<BlockMesh> mesh = BlockMesh::createFromMemory( fileData.cbegin(), fileData.cend(), meshFileInfo.getFormat( ), meshFileInfo.getIndexInFile( ), meshFileInfo.getInvertZCoordinate( ), meshFileInfo.getInvertVertexWindingOrder( ), meshFileInfo.getFlipUVs( ) );
            mesh->setFileInfo( meshFileInfo );

            return mesh;
        }
		case Asset::Type::SkeletonMesh:
		{
			const SkeletonMeshFileInfo& meshFileInfo = static_cast<const SkeletonMeshFileInfo&>( fileInfo );
            std::shared_ptr<SkeletonMesh> mesh = SkeletonMesh::createFromMemory( fileData.cbegin(), fileData.cend(), meshFileInfo.getFormat( ), meshFileInfo.getIndexInFile( ), meshFileInfo.getInvertZCoordinate( ), meshFileInfo.getInvertVertexWindingOrder( ), meshFileInfo.getFlipUVs( ) );
            mesh->setFileInfo( meshFileInfo );

            return mesh;
        
        }
		case Asset::Type::SkeletonAnimation:
		{
			const SkeletonAnimationFileInfo& animFileInfo = static_cast<const SkeletonAnimationFileInfo&>( fileInfo );

			std::shared_ptr<const SkeletonMesh> referenceMesh;
			{ // Get or load the reference mesh.
                const bool meshLoadedOrLoading = isLoadedOrLoading( animFileInfo.getMeshFileInfo( ).getAssetType( ), animFileInfo.getMeshFileInfo( ).getPath( ), animFileInfo.getMeshFileInfo( ).getIndexInFile( ) );
				if ( !meshLoadedOrLoading )
					loadAsync( animFileInfo.getMeshFileInfo() );

				const float loadTimeout = 60.0f;
                std::shared_ptr<const Asset> mesh = getWhenLoaded( animFileInfo.getMeshFileInfo( ).getAssetType( ), animFileInfo.getMeshFileInfo( ).getPath( ), animFileInfo.getMeshFileInfo( ).getIndexInFile( ), loadTimeout );
				referenceMesh = mesh->getType() == Asset::Type::SkeletonMesh ? std::static_pointer_cast<const SkeletonMesh>( mesh ) : nullptr;
			}

            if ( referenceMesh ) {
                std::shared_ptr<SkeletonAnimation> anim = SkeletonAnimation::createFromMemory( fileData, animFileInfo.getFormat(), *referenceMesh, animFileInfo.getInvertZCoordinate() );
                anim->setFileInfo( animFileInfo );

                return anim;
            } else {
                throw std::exception( "AssetManager::createFromMemory - failed to load reference mesh for the animation." );
            }
		}
		case Asset::Type::Texture2D:
		{
			const Texture2DFileInfo& texFileInfo = static_cast<const Texture2DFileInfo&>( fileInfo );

            if ( texFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR4 )
            {
                auto texture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >
                    ( *m_device.Get(), fileData.cbegin(), fileData.cend(), texFileInfo.getFormat( ), true, true, true, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
                texture->setFileInfo( texFileInfo );

                return texture;
            }
            else if ( texFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR )
            {
                auto texture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >
                    ( *m_device.Get(), fileData.cbegin(), fileData.cend(), texFileInfo.getFormat( ), true, true, true, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
                texture->setFileInfo( texFileInfo );

                return texture;
            }
		}
		default:
			throw std::exception( "AssetManager::createFromMemory - asset type not yet supported." );
	}
}

std::string AssetManager::getId( Asset::Type type, const std::string path, const int indexInFile )
{
    return "(" + Asset::toString(type) + ") " + StringUtil::toLowercase( path ) + " [" + std::to_string( indexInFile ) + "]";
}