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

struct ID3D11Device3;

namespace Engine1
{
    class AssetManager
    {

        public:
        AssetManager();
        ~AssetManager();

        // parsingBasicAssetsThreadCount and parsingComplexAssetsThreadCount - should be around the number of threads the CPU can run in parallel.
        // Having separate threads for complex assets is needed, because they can all freeze waiting for basic assets (sub-assets) to be loaded.
        // Thus we need other threads to parse basic assets at all times.
        void initialize( int parsingBasicAssetsThreadCount, int parsingComplexAssetsThreadCount, Microsoft::WRL::ComPtr< ID3D11Device3 > device );

        void                   load( const FileInfo& fileInfo );
        void                   loadAsync( const FileInfo& fileInfo, const bool highestPriority = false );
        bool                   isLoaded( Asset::Type type, std::string path, const int indexInFile = 0 );
        bool                   isLoadedOrLoading( Asset::Type type, std::string path, const int indexInFile = 0 );
        std::shared_ptr<Asset> get( Asset::Type type, std::string path, const int indexInFile = 0 );
        std::shared_ptr<Asset> getOrLoad( const FileInfo& fileInfo );
        std::shared_ptr<Asset> getWhenLoaded( Asset::Type type, std::string path, const int indexInFile = 0, const float timeout = 10.0f );

        void unloadAll();

        private:

        Microsoft::WRL::ComPtr< ID3D11Device3 > m_device;

        bool m_executeThreads;

        void readAssetsFromDisk();
        void parseBasicAssets();
        void parseComplexAssets();

        std::shared_ptr<Asset> createFromFile( const FileInfo& fileInfo );
        std::shared_ptr<Asset> createFromMemory( const FileInfo& fileInfo, const std::vector<char>& fileData );

        std::string getId( Asset::Type type, const std::string path, const int indexInFile );

        std::thread              m_readingFromDiskThread;
        std::vector<std::thread> m_parsingBasicAssetsThreads;
        std::vector<std::thread> m_parsingComplexAssetsThreads;

        // List of all assets which are in the course of loading or were loaded already.
        std::mutex                      m_assetsMutex;
        std::unordered_set<std::string> m_assets;

        std::mutex                                     m_assetsToReadFromDiskMutex;
        std::condition_variable                        m_assetsToReadFromDiskNotEmpty;
        std::list< std::shared_ptr< const FileInfo > > m_basicAssetsToReadFromDisk;
        std::list< std::shared_ptr< const FileInfo > > m_complexAssetsToReadFromDisk;

        struct AssetToParse
        {
            std::shared_ptr< const FileInfo > fileInfo;
            std::shared_ptr< std::vector<char> > fileData;

            AssetToParse() :
                fileInfo( nullptr ),
                fileData( nullptr )
            {}

            AssetToParse( std::shared_ptr< const FileInfo > fileInfo, std::shared_ptr< std::vector<char> > fileData ) :
                fileInfo( fileInfo ),
                fileData( fileData )
            {}

            AssetToParse( const AssetToParse& other ) :
                fileInfo( other.fileInfo ),
                fileData( other.fileData )
            {}
        };

        std::mutex               m_basicAssetsToParseMutex;
        std::mutex               m_complexAssetsToParseMutex;
        std::condition_variable  m_basicAssetsToParseNotEmpty;
        std::condition_variable  m_complexAssetsToParseNotEmpty;
        std::list< AssetToParse > m_basicAssetsToParse;
        std::list< AssetToParse > m_complexAssetsToParse;

        std::mutex                                                m_loadedAssetsMutex;
        std::unordered_map< std::string, std::shared_ptr<Asset> > m_loadedAssets;

        // Used to notify 'getWhenLoaded' method that a new asset has just finished loading.
        std::condition_variable m_assetLoadedOrError;
    };
}

