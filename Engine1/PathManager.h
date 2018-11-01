#pragma once

#include <unordered_map>
#include <vector>

namespace Engine1
{
    class PathManager
    {
        public:

        void scanDirectory( const std::string& directory );

        std::string getPathForFileName( const std::string& fileName ) const;

        const std::unordered_map< std::string, std::string >& getAllPaths() const;

        private:

        // Key - file name (with extension), value - path to the file.
        std::unordered_map< std::string, std::string > m_paths;
    };
}

