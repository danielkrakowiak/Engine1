#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <wrl.h>

#include "Asset.h"
#include "FileInfo.h"

struct ID3D11Device;

namespace Engine1
{
    class AssetManager
    {

        public:
        AssetManager();
        ~AssetManager();

        // loadingThreadCount - should be around twice the number of threads the CPU can run in parallel.
        // Higher number is needed because some threads may be waiting (sleeping) for sub-assets to be loaded and thus not doing any work.
        void initialize( int loadingThreadCount, Microsoft::WRL::ComPtr< ID3D11Device > device );

        void                   load( const FileInfo& fileInfo );
        void                   loadAsync( const FileInfo& fileInfo );
        bool                   isLoaded( Asset::Type type, std::string path, const int indexInFile = 0 );
        bool                   isLoadedOrLoading( Asset::Type type, std::string path, const int indexInFile = 0 );
        std::shared_ptr<Asset> get( Asset::Type type, std::string path, const int indexInFile = 0 );
        std::shared_ptr<Asset> getOrLoad( const FileInfo& fileInfo );
        std::shared_ptr<Asset> getWhenLoaded( Asset::Type type, std::string path, const int indexInFile = 0, const float timeout = 10.0f );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device > m_device;

        bool executeThreads;

        void loadAssetsFromDisk();
        void loadAssets();

        std::shared_ptr<Asset> createFromFile( const FileInfo& fileInfo );
        std::shared_ptr<Asset> createFromMemory( const FileInfo& fileInfo, const std::vector<char>& fileData );

        std::string getId( Asset::Type type, const std::string path, const int indexInFile );

        std::thread              readingFromDiskThread;
        std::vector<std::thread> parsingThreads;

        // List of all assets which are in the course of loading or were loaded already.
        std::mutex                      assetsMutex;
        std::unordered_set<std::string> assets;

        std::mutex                                     assetsToReadFromDiskMutex;
        std::condition_variable                        assetsToReadFromDiskNotEmpty;
        std::list< std::shared_ptr< const FileInfo > > basicAssetsToReadFromDisk;
        std::list< std::shared_ptr< const FileInfo > > complexAssetsToReadFromDisk;

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

        std::mutex               assetsToParseMutex;
        std::condition_variable  assetsToParseNotEmpty;
        std::list< AssetToLoad > basicAssetsToParse;
        std::list< AssetToLoad > complexAssetsToParse;

        std::mutex                                                loadedAssetsMutex;
        std::unordered_map< std::string, std::shared_ptr<Asset> > loadedAssets;

        // Used to notify 'getWhenLoaded' method that a new asset has just finished loading.
        std::condition_variable assetLoaded;
    };
}

