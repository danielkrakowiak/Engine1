#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "BasicAsset.h"

class BlockMesh;
class Texture2D;

class AssetManager {

public:
	AssetManager( int loadingThreadCount );
	~AssetManager();

	void loadAsset( std::shared_ptr<BasicAsset>& );
	void loadAssetAsync( std::shared_ptr<BasicAsset>& );
	bool isAssetLoaded( std::string path );
	std::shared_ptr<BasicAsset>& getLoadedAsset( std::string path );
	bool isAssetLoadedOrLoading( std::string path );

private:

	bool executeThreads;

	void loadAssetsFromDisk( );
	void loadAssets( );

	std::thread loadingFromDiskThread;
	std::vector<std::thread> loadingThreads;

	//list of all assets which are in the course of loading or were loaded already
	std::mutex assetsMutex;
	std::unordered_set<std::string> assets;

	std::mutex assetsToLoadFromDiskMutex;
	std::condition_variable assetsToLoadFromDiskNotEmpty;
	std::list<std::shared_ptr<BasicAsset>> assetsToLoadFromDisk;

	std::mutex assetsToLoadMutex;
	std::condition_variable assetsToLoadNotEmpty;
	std::list<std::shared_ptr<BasicAsset>> assetsToLoad;

	std::mutex loadedAssetsMutex;
	std::unordered_map<std::string, std::shared_ptr<BasicAsset>> loadedAssets;
	
};

