#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Engine1
{
    class Scene;
    class FileInfo;

    class SceneParser
    {
        friend class Scene;

        private:

        // Returns the parsed scene (with models containing only file info) and a vector of unique models (their file infos) in that scene.
        static std::tuple< std::shared_ptr<Scene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > parseBinary( const std::vector<char>& data );

        static void writeBinary( std::vector<char>& data, const Scene& scene );

        struct FileInfoHasher
        {
            std::size_t operator()( const std::shared_ptr<FileInfo>& fileInfo ) const;
        };

        struct FileInfoComparator
        {
            bool operator()( const std::shared_ptr<FileInfo>& fileInfo1, const std::shared_ptr<FileInfo>& fileInfo2 ) const;
        };
        
    };
}

