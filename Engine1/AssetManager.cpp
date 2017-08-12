#include "AssetManager.h"

#include <climits>
#include <thread>
#include <d3d11_3.h>

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
	m_executeThreads = false;
	m_assetsToReadFromDiskNotEmpty.notify_all();
	m_basicAssetsToParseNotEmpty.notify_all();
    m_complexAssetsToParseNotEmpty.notify_all();

	m_readingFromDiskThread.join();

	for ( auto& thread : m_parsingBasicAssetsThreads )
		thread.join();

    for ( auto& thread : m_parsingComplexAssetsThreads )
        thread.join();
}

void AssetManager::initialize( int parsingBasicAssetsThreadCount, int parsingComplexAssetsThreadCount, ComPtr< ID3D11Device3 > device )
{
    if ( parsingBasicAssetsThreadCount <= 0 ) 
        throw std::exception( "AssetManager::AssetManager - Number of loading threads has to be greater than 0." );

    m_device = device;

	m_executeThreads = true;

	// Create thread to load assets from disk.
	m_readingFromDiskThread = std::thread( &AssetManager::readAssetsFromDisk, this );

	// Create threads to parse basic assets.
	for ( int i = 0; i < parsingBasicAssetsThreadCount; ++i ) {
		m_parsingBasicAssetsThreads.push_back( std::thread( &AssetManager::parseBasicAssets, this ) );
	}

    // Create threads to parse complex assets.
    for ( int i = 0; i < parsingComplexAssetsThreadCount; ++i ) {
        m_parsingComplexAssetsThreads.push_back( std::thread( &AssetManager::parseComplexAssets, this ) );
    }
}

void AssetManager::load( const FileInfo& fileInfo )
{
    std::string id = getId( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() );

	{ // Check if asset was loaded already or is in the course of loading.
		std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
		if ( m_assets.count( id ) != 0 ) 
			throw std::exception( "AssetManager::load - Asset is already loaded or in the course of loading." );
		
		m_assets.insert( id );
	}

	try {
		OutputDebugStringW( StringUtil::widen( 
            "AssetManager::load - reading and parsing \"" 
            + fileInfo.getPath( ) + "\" [" 
            + std::to_string( fileInfo.getIndexInFile() ) + "]\n" 
        ).c_str( ) );

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
			std::lock_guard<std::mutex> loadedAssetsLock( m_loadedAssetsMutex );
			m_loadedAssets.insert( std::make_pair( id, asset ) );
		}

		OutputDebugStringW( StringUtil::widen( 
            "AssetManager::load - read and parsed \"" 
            + fileInfo.getPath( ) + "\" [" 
            + std::to_string( fileInfo.getIndexInFile() ) + "]\n" 
        ).c_str( ) );

	} catch ( .../*std::exception& ex*/ ) {
		// If asset failed to load - remove it from assets.
		std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
		m_assets.erase( id );

		OutputDebugStringW( StringUtil::widen( 
            "AssetManager::load - failed to read or parse \"" 
            + fileInfo.getPath( ) + "\" [" 
            + std::to_string( fileInfo.getIndexInFile() ) + "]\n"  
        ).c_str( ) );

		// Re-throw the exception.
		throw;
	}
}

void AssetManager::loadAsync( const FileInfo& fileInfo, const bool highestPriority )
{
    std::string id = getId( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() );

	{ // Check if this asset was loaded already or is in the course of loading.
		std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
		if ( m_assets.count( id ) != 0 ) 
			return; // Asset is already loading or loaded.

		m_assets.insert( id );
	}

	{ // Add the asset to the list of assets to load from disk - lock mutex.
		std::unique_lock<std::mutex> assetsToLoadFromDiskLock( m_assetsToReadFromDiskMutex );

        if ( fileInfo.canHaveSubAssets() ) 
        {
            if ( highestPriority )
                m_complexAssetsToReadFromDisk.insert( m_complexAssetsToReadFromDisk.begin(), fileInfo.clone() );
            else
                m_complexAssetsToReadFromDisk.push_back( fileInfo.clone() );
        } 
        else 
        {
            if ( highestPriority )
                m_basicAssetsToReadFromDisk.insert( m_basicAssetsToReadFromDisk.begin(), fileInfo.clone() );
            else
		        m_basicAssetsToReadFromDisk.push_back( fileInfo.clone() );
        }
	}

	// Resume thread which loads assets from disk.
	m_assetsToReadFromDiskNotEmpty.notify_one();
}

bool AssetManager::isLoaded( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	//TODO: should I lock loadedAssets before searching through it?
    return (m_loadedAssets.find( id ) != m_loadedAssets.end( ));
}

bool AssetManager::isLoadedOrLoading( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
    return (m_assets.count( id ) != 0);
}

std::shared_ptr<Asset> AssetManager::get( Asset::Type type, std::string path, const int indexInFile )
{
    std::string id = getId( type, path, indexInFile );

	std::lock_guard<std::mutex> loadedAssetsLock( m_loadedAssetsMutex );

    std::unordered_map< std::string, std::shared_ptr<Asset> >::iterator it = m_loadedAssets.find( id );

	if ( it == m_loadedAssets.end() )
		return nullptr;

	return it->second;
}

std::shared_ptr<Asset> AssetManager::getOrLoad( const FileInfo& fileInfo )
{
    const float timeout = 660.0f;

    if ( !isLoadedOrLoading( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ) )
        load( fileInfo );

    return getWhenLoaded( fileInfo.getAssetType(), fileInfo.getPath( ), fileInfo.getIndexInFile( ), timeout );
}

std::shared_ptr<Asset> AssetManager::getWhenLoaded( Asset::Type type, std::string path, const int indexInFile, const float timeout )
{
    std::string id = getId( type, path, indexInFile );

	std::unordered_map< std::string, std::shared_ptr<Asset> >::iterator it;

	const std::chrono::steady_clock::time_point timoutTime = std::chrono::steady_clock::now( ) + std::chrono::microseconds( (long long)( timeout / 0.000001f ) );

	std::unique_lock<std::mutex> loadedAssetsLock( m_loadedAssetsMutex );

	for (;;) {
		// Check if the asset is loaded.
        it = m_loadedAssets.find( id );
		if ( it != m_loadedAssets.end() )
			return it->second;

        { // Check if that asset failed to load (because it's not loading).
            std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
            if ( m_assets.count( id ) == 0 )
                return nullptr; //#TODO: Should it throw exception?
        }

		// Wait until some asset finish loading or timeout.
		std::_Cv_status status = m_assetLoadedOrError.wait_until( loadedAssetsLock, timoutTime );

		// Exit method on timeout.
		if ( status == std::cv_status::timeout )
			return nullptr;
	}
}

void AssetManager::readAssetsFromDisk() {
	std::shared_ptr< const FileInfo > fileInfo = nullptr;
	std::shared_ptr< std::vector<char> > fileData = nullptr;

	for (;;) {
		fileInfo = nullptr;
		fileData = nullptr;

		{ // Check if there is any asset to load and if so, get it - hold lock.
			std::unique_lock<std::mutex> assetsToReadFromDiskLock( m_assetsToReadFromDiskMutex );

			// Wait until there are some assets to load from disk.
			m_assetsToReadFromDiskNotEmpty.wait( assetsToReadFromDiskLock, [this]() { return ( !m_basicAssetsToReadFromDisk.empty() || !m_complexAssetsToReadFromDisk.empty() ) || !m_executeThreads; } );

			// Terminate thread if requested.
			if ( !m_executeThreads ) 
				return;

            // Get first asset from the list. Basic assets have priority over complex assets
            // to avoid locking parsing threads which wait for the sub-assets to be loaded.
            if ( !m_basicAssetsToReadFromDisk.empty() ) {
			    // Get first asset from the list
			    fileInfo = m_basicAssetsToReadFromDisk.front( );
			    // Remove asset from the list.
			    m_basicAssetsToReadFromDisk.pop_front();
            } else {
                // Get first asset from the list
			    fileInfo = m_complexAssetsToReadFromDisk.front( );
			    // Remove asset from the list.
			    m_complexAssetsToReadFromDisk.pop_front();
            }
		}

		OutputDebugStringW( StringUtil::widen( 
            "AssetManager::readAssetsFromDisk - reading \"" 
            + fileInfo->getPath( ) + "\" [" 
            + std::to_string( fileInfo->getIndexInFile() ) + "]\n"  
        ).c_str( ) );

		// Load file from disk.
		try {
			if ( fileInfo->getFileType() == FileInfo::FileType::Textual )
				fileData = TextFile::load( fileInfo->getPath() );
			else if ( fileInfo->getFileType() == FileInfo::FileType::Binary )
				fileData = BinaryFile::load( fileInfo->getPath() );

			OutputDebugStringW( StringUtil::widen( 
                "AssetManager::readAssetsFromDisk - read \"" 
                + fileInfo->getPath( ) + "\" [" 
                + std::to_string( fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str( ) );

		} catch ( .../*std::exception& ex*/ ) {
            std::string id = getId( fileInfo->getAssetType( ), fileInfo->getPath( ), fileInfo->getIndexInFile( ) );

			// If asset failed to load - remove it from assets.
			std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
			m_assets.erase( id );

			OutputDebugStringW( StringUtil::widen( 
                "AssetManager::readAssetsFromDisk - failed to read \"" 
                + fileInfo->getPath( ) + "\" [" 
                + std::to_string( fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str( ) );

            // Notify 'getWhenLoaded' method that an asset failed to load.
            m_assetLoadedOrError.notify_all();

			//TODO: handle this error - do some callback for ex.
            continue;
		}

		{ // Add asset to list of assets to load - hold mutex.
			
			// Add asset to list of assets to load.
            if ( fileInfo->canHaveSubAssets() ) 
            {
                std::lock_guard<std::mutex> complexAssetsToParseLock( m_complexAssetsToParseMutex );
                m_complexAssetsToParse.push_back( AssetToParse( fileInfo, fileData ) );

                // Resume one of threads which parses complex assets.
                m_complexAssetsToParseNotEmpty.notify_one();
            } 
            else 
            {
                std::lock_guard<std::mutex> basicAssetsToParseLock( m_basicAssetsToParseMutex );
			    m_basicAssetsToParse.push_back( AssetToParse( fileInfo, fileData ) );

                // Resume one of threads which parses basic assets.
                m_basicAssetsToParseNotEmpty.notify_one();
            }
		}
	}
}

void AssetManager::parseBasicAssets() 
{
	for (;;) {
		AssetToParse assetToParse;

		{ // Check if there is any asset to load and if so, get it - hold lock.
			std::unique_lock<std::mutex> basicAssetsToParseLock( m_basicAssetsToParseMutex );

			// Wait until there are some assets to load.
			m_basicAssetsToParseNotEmpty.wait( basicAssetsToParseLock, [this]() { return !m_basicAssetsToParse.empty() || !m_executeThreads; } );

			// Terminate thread if requested.
			if ( !m_executeThreads ) return;

			// Get first basic asset from the list.
			assetToParse = m_basicAssetsToParse.front( );

            // Remove basic asset from the list.
			m_basicAssetsToParse.pop_front();
		}

		OutputDebugStringW( StringUtil::widen( 
            "AssetManager::parseBasicAssets - parsing \"" 
            + assetToParse.fileInfo->getPath( ) + "\" [" 
            + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
        ).c_str( ) );

		std::shared_ptr<Asset> asset = nullptr;

		// Parse basic asset.
		try {
			asset = createFromMemory( *assetToParse.fileInfo, *assetToParse.fileData );

			OutputDebugStringW( StringUtil::widen( 
                "AssetManager::parseBasicAssets - parsed \"" 
                + assetToParse.fileInfo->getPath( ) + "\" [" 
                + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str( ) );
		} catch ( .../*std::exception& ex*/ ) {
			// Asset failed to load - remove it from assets.
			std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
			m_assets.erase( assetToParse.fileInfo->getPath() );

			OutputDebugStringW( StringUtil::widen( 
                "AssetManager::parseBasicAssets - failed to parse \"" 
                + assetToParse.fileInfo->getPath( ) + "\" [" 
                + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str( ) );

            // Notify 'getWhenLoaded' method that an asset failed to load.
            m_assetLoadedOrError.notify_all();

			//TODO: handle this error - do some callback for ex.
            continue;
		}

		{ // Add asset to a list of assets - hold lock.
			std::lock_guard<std::mutex> loadedAssetsLock( m_loadedAssetsMutex );
			// Add asset to a list of assets.

            std::string id = getId( assetToParse.fileInfo->getAssetType( ), assetToParse.fileInfo->getPath( ), assetToParse.fileInfo->getIndexInFile() );
			m_loadedAssets.insert( std::make_pair( id, asset ) );
		}

		// Notify 'getWhenLoaded' method that an asset has just been loaded.
		m_assetLoadedOrError.notify_all();
	}
}

void AssetManager::parseComplexAssets()
{
    for ( ;;) {
        AssetToParse assetToParse;

        { // Check if there is any asset to load and if so, get it - hold lock.
            std::unique_lock<std::mutex> complexAssetsToParseLock( m_complexAssetsToParseMutex );

            // Wait until there are some assets to load.
            m_complexAssetsToParseNotEmpty.wait( complexAssetsToParseLock, [ this ]() { return !m_complexAssetsToParse.empty() || !m_executeThreads; } );

            // Terminate thread if requested.
            if ( !m_executeThreads ) return;

            // Get first complex asset from the list.
            assetToParse = m_complexAssetsToParse.front();

            // Remove complex asset from the list.
            m_complexAssetsToParse.pop_front();
        }

        OutputDebugStringW( StringUtil::widen( 
            "AssetManager::parseComplexAssets - parsing \"" 
            + assetToParse.fileInfo->getPath() + "\" [" 
            + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
        ).c_str() );

        std::shared_ptr<Asset> asset = nullptr;

        // Parse complex asset and its sub-assets (basic assets).
        try {
            asset = createFromMemory( *assetToParse.fileInfo, *assetToParse.fileData );

            { // Load sub-assets if needed.
                std::vector<std::shared_ptr<Asset>> subAssets = asset->getSubAssets();
                for ( std::shared_ptr<Asset>& subAsset : subAssets ) {
                    if ( !subAsset->getFileInfo().getPath().empty() )
                        loadAsync( subAsset->getFileInfo(), true ); // Note: Load sub-assets with the highest priority.
                }

                // Wait for the sub-assets to be loaded and swap empty sub-assets with loaded sub-assets.
                const float subAssetLoadTimeout = 660.0f;
                for ( std::shared_ptr<Asset>& subAsset : subAssets ) {
                    if ( !subAsset->getFileInfo().getPath().empty() ) {
                        std::shared_ptr<Asset> newSubAsset = getWhenLoaded( subAsset->getFileInfo().getAssetType(), subAsset->getFileInfo().getPath(), subAsset->getFileInfo().getIndexInFile(), subAssetLoadTimeout );

                        asset->swapSubAsset( subAsset, newSubAsset );
                    }
                }
            }

            OutputDebugStringW( StringUtil::widen( 
                "AssetManager::parseComplexAssets - parsed \"" 
                + assetToParse.fileInfo->getPath() + "\" [" 
                + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str() );
        } catch ( .../*std::exception& ex*/ ) {
            // Asset failed to load - remove it from assets.
            std::lock_guard<std::mutex> assetsLock( m_assetsMutex );
            m_assets.erase( assetToParse.fileInfo->getPath() );

            OutputDebugStringW( StringUtil::widen( 
                "AssetManager::parseComplexAssets - failed to parse \"" 
                + assetToParse.fileInfo->getPath() + "\" [" 
                + std::to_string( assetToParse.fileInfo->getIndexInFile() ) + "]\n"  
            ).c_str() );

            // Notify 'getWhenLoaded' method that an asset failed to load.
            m_assetLoadedOrError.notify_all();

            //TODO: handle this error - do some callback for ex.
            continue;
        }

        { // Add asset to a list of assets - hold lock.
            std::lock_guard<std::mutex> loadedAssetsLock( m_loadedAssetsMutex );
            // Add asset to a list of assets.

            std::string id = getId( assetToParse.fileInfo->getAssetType(), assetToParse.fileInfo->getPath(), assetToParse.fileInfo->getIndexInFile() );
            m_loadedAssets.insert( std::make_pair( id, asset ) );
        }

        // Notify 'getWhenLoaded' method that an asset has just been loaded.
        m_assetLoadedOrError.notify_all();
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
        case Asset::Type::SkeletonModel:
			return SkeletonModel::createFromFile( static_cast<const SkeletonModelFileInfo&>( fileInfo ), false, *m_device.Get() );
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
			    return std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
                    ( *m_device.Get(), textureFileInfo, true, true, true, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
            }
            else if ( textureFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR )
            {
                return std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
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
			std::shared_ptr<BlockModel> model = BlockModel::createFromMemory( fileData.cbegin(), fileData.cend(), modelFileInfo.getFormat( ), false, *m_device.Get() );
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
                auto texture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
                    ( *m_device.Get(), fileData.cbegin(), fileData.cend(), texFileInfo.getFormat( ), true, true, true, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM );
                texture->setFileInfo( texFileInfo );

                return texture;
            }
            else if ( texFileInfo.getPixelType() == Texture2DFileInfo::PixelType::UCHAR )
            {
                auto texture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
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