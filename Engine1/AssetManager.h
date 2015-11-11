#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "Asset.h"
#include "FileInfo.h"

class BlockMesh;
class Texture2D;

class AssetManager {

public:
	AssetManager( int loadingThreadCount );
	~AssetManager();

	void                   load( const FileInfo& fileInfo );
	void                   loadAsync( const FileInfo& fileInfo );
	bool                   isAvailable( std::string path );
	std::shared_ptr<Asset> get( std::string path );
	bool                   isAvailableOrLoading( std::string path );

private:

	bool executeThreads;

	void loadAssetsFromDisk( );
	void loadAssets( );

	std::shared_ptr<Asset> createFromFile( const FileInfo& fileInfo );
	std::shared_ptr<Asset> createFromMemory( const FileInfo& fileInfo, const std::vector<char>& fileData );

	std::thread              loadingFromDiskThread;
	std::vector<std::thread> loadingThreads;

	// List of all assets which are in the course of loading or were loaded already.
	std::mutex                      assetsMutex;
	std::unordered_set<std::string> assets;

	std::mutex                                     assetsToLoadFromDiskMutex;
	std::condition_variable                        assetsToLoadFromDiskNotEmpty;
	std::list< std::shared_ptr< const FileInfo > > assetsToLoadFromDisk;

	struct AssetToLoad
	{
		std::shared_ptr< const FileInfo > fileInfo;
		std::shared_ptr< std::vector<char> > fileData;

		AssetToLoad() :
			fileInfo( nullptr ),
			fileData( nullptr )
		{}

		AssetToLoad( std::shared_ptr< const FileInfo > fileInfo, std::shared_ptr< std::vector<char> > fileData ) :
			fileInfo( fileInfo ),
			fileData( fileData )
		{}

		AssetToLoad( const AssetToLoad& other ) :
			fileInfo( other.fileInfo ),
			fileData( other.fileData )
		{}
	};

	std::mutex                                     assetsToLoadMutex;
	std::condition_variable                        assetsToLoadNotEmpty;
	std::list< AssetToLoad > assetsToLoad;

	std::mutex                                                loadedAssetsMutex;
	std::unordered_map< std::string, std::shared_ptr<Asset> > loadedAssets;
};

