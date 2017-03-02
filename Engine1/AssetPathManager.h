#pragma once

#include <unordered_map>
#include <vector>

namespace Engine1
{
    class AssetPathManager
    {
        public:

        static bool initialize();

        static std::string getPathForFileName( const std::string& fileName );

        private:

        // Key - file name (with extension), value - path to the file.
        static std::unordered_map< std::string, std::string > s_paths;

        static bool s_initialized;
    };
}

