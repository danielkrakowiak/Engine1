#pragma once

#include <string>
#include <vector>

namespace Engine1
{
    class FileSystem
    {
        public:

        static long long getFileSize( const std::string& path );

        static std::vector< std::string > getAllFilesFromDirectory( std::string directoryPath );
    };
};

