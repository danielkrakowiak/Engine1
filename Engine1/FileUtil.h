#pragma once

#include <string>

class FileUtil
{
    public:

    static std::string getFileNameFromPath( const std::string& path );
    static std::string getFileExtensionFromPath( const std::string& path );
};

