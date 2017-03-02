#include "FileUtil.h"

std::string FileUtil::getFileNameFromPath( const std::string& path )
{
    const size_t slashIndex = path.rfind( "\\" );
    if ( slashIndex == std::string::npos )
        return path; // Path is the file name.

    
    if ( slashIndex + 1 == path.size() )
        return ""; // No file name after the slash.

    // Normal case.
    return path.substr( slashIndex + 1 );
}