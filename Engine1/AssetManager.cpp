#include "AssetManager.h"

#include <climits>
#include <thread>

#include "BlockMesh.h"
#include "Texture2D.h"

#include "StringUtil.h"

AssetManager::AssetManager( int loadingThreadCount = INT_MAX ) {
	if ( loadingThreadCount <= 0 ) throw std::exception( "AssetManager::AssetManager - Number of loading threads cannot be less or equal to 0" );

	executeThreads = true;

	//create thread to load assets from disk
	loadingFromDiskThread = std::thread( &AssetManager::loadAssetsFromDisk, this );

	//create threads to load assets
	for ( int i = 0; i < loadingThreadCount; ++i ) {
		loadingThreads.push_back( std::thread( &AssetManager::loadAssets, this ) );
	}
}

AssetManager::~AssetManager() {

	executeThreads = false;
	assetsToLoadFromDiskNotEmpty.notify_all();
	assetsToLoadNotEmpty.notify_all();

	loadingFromDiskThread.join();

	for ( std::vector<std::thread>::iterator thread = loadingThreads.begin(); thread != loadingThreads.end(); thread++ ) {
		( *thread ).join();
	}
}


void AssetManager::loadAsset( std::shared_ptr<BasicAsset>& asset ) {
	std::string path = StringUtil::toLowercase( asset->getPath() );

	{ //check if asset was loaded already or is in the course of loading
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		if ( assets.count( path ) != 0 ) throw std::exception( "AssetManager::loadAsset - Asset is already loaded or in the course of loading" );
		assets.insert( path );
	}

	try {
		asset->loadFile( );
		asset->load( );
		asset->unloadFile();

		{
			std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );
			loadedAssets.insert( std::make_pair( path, asset ) );
		}
	} catch ( std::exception& ex ) {
		//if asset failed to load - remove it from assets
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		assets.erase( path );
		//rethrow exception
		throw ex;
	}
}

void AssetManager::loadAssetAsync( std::shared_ptr<BasicAsset>& asset ) {
	std::string path = StringUtil::toLowercase( asset->getPath() );

	{ //check if assets was loaded already or is in the course of loading
		std::lock_guard<std::mutex> assetsLock( assetsMutex );
		if ( assets.count( path ) != 0 ) throw std::exception( "AssetManager::loadAssetAsync - Asset is already loaded or in the course of loading" );
		assets.insert( path );
	}

	{ // add asset to the list of assets to load from disk - lock mutex
		std::unique_lock<std::mutex> assetsToLoadFromDiskLock( assetsToLoadFromDiskMutex );
		assetsToLoadFromDisk.push_back( asset );
	}


	//resume thread which loads assets from disk
	assetsToLoadFromDiskNotEmpty.notify_one();
}

bool AssetManager::isAssetLoaded( std::string path ) {
	path = StringUtil::toLowercase( path );

	//TODO: should I lock loadedAssets before searching through it?
	return ( loadedAssets.find( path ) != loadedAssets.end( ) );
}

std::shared_ptr<BasicAsset>& AssetManager::getLoadedAsset( std::string path ) {
	path = StringUtil::toLowercase( path );

	std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );

	std::unordered_map<std::string, std::shared_ptr<BasicAsset>>::iterator it = loadedAssets.find( path );

	if ( it == loadedAssets.end() ) throw std::exception( "AssetManager::getLoadedAsset - No BlockMesh found for that path" );

	return it->second;
}

bool AssetManager::isAssetLoadedOrLoading( std::string path ) {
	path = StringUtil::toLowercase( path );

	std::lock_guard<std::mutex> assetsLock( assetsMutex );
	return ( assets.count( path ) != 0 );
}

void AssetManager::loadAssetsFromDisk() {
	std::shared_ptr<BasicAsset> asset;

	while ( true ) {
		{ //check if there is any asset to load and if so, get it - hold lock
			std::unique_lock<std::mutex> assetsToLoadFromDiskLock( assetsToLoadFromDiskMutex );

			//wait until there are some assets to load from disk
			assetsToLoadFromDiskNotEmpty.wait( assetsToLoadFromDiskLock, [this]() { return !assetsToLoadFromDisk.empty() || !executeThreads; } );

			//terminate thread if requested
			if ( !executeThreads ) return;

			//get first asset from the list
			asset = assetsToLoadFromDisk.front();
			//remove asset from the list
			assetsToLoadFromDisk.pop_front();
		}

		//load file from disk
		try {
			asset->loadFile();
		} catch ( std::exception& ex ) {
			//if asset failed to load - remove it from assets
			std::lock_guard<std::mutex> assetsLock( assetsMutex );
			assets.erase( asset->getPath() );

			//TODO: handle this error - do some callback for ex.
		}

		{ //add asset to list of assets to load - hold mutex
			std::lock_guard<std::mutex> assetsToLoadLock( assetsToLoadMutex );
			//add asset to list of assets to load
			assetsToLoad.push_back( asset );
		}

		//resume one of threads which load assets
		assetsToLoadNotEmpty.notify_one();
	}
}

void AssetManager::loadAssets() {

	std::shared_ptr<BasicAsset> asset;

	while ( true ) {
		{ //check if there is any asset to load and if so, get it - hold lock
			std::unique_lock<std::mutex> assetsToLoadLock( assetsToLoadMutex );

			//wait until there are some assets to load
			assetsToLoadNotEmpty.wait( assetsToLoadLock, [this]() { return !assetsToLoad.empty() || !executeThreads; } );

			//terminate thread if requested
			if ( !executeThreads ) return;

			//get first asset from the list
			asset = assetsToLoad.front();
			//remove asset from the list
			assetsToLoad.pop_front();
		}

		//load file
		try {
			asset->load();
		} catch ( std::exception& ex ) {
			//if asset failed to load - remove it from assets
			std::lock_guard<std::mutex> assetsLock( assetsMutex );
			assets.erase( asset->getPath() );

			//TODO: handle this error - do some callback for ex.
		}

		{ //add asset to a list of assets - hold lock
			std::lock_guard<std::mutex> loadedAssetsLock( loadedAssetsMutex );
			//add asset to a list of assets
			loadedAssets.insert( std::pair<std::string, std::shared_ptr<BasicAsset>>( asset->getPath(), asset ) );
		}
	}
}