#include "FileUtil.h"

std::string FileUtil::getFileNameFromPath( const std::string& path, bool withExtension )
{
    const size_t slashIndex = path.rfind( "\\" );
    if ( slashIndex == std::string::npos )
    {
        // Path is the file name.
        return (withExtension 
            ? path
            : removeFileExtensionFromPath( path )); 
    }
    
    if ( slashIndex + 1 == path.size() )
        return ""; // No file name after the slash.

    auto filename = path.substr( slashIndex + 1 );

    // Normal case.
    return (withExtension 
        ? filename
        : removeFileExtensionFromPath( filename )); 
}

std::string FileUtil::getFileExtensionFromPath( const std::string& path )
{
    std::string fileName = getFileNameFromPath( path );

    const size_t dotIndex = fileName.rfind( "." );
    if ( dotIndex == std::string::npos )
        return ""; // File has no extension.

    return fileName.substr( dotIndex + 1 );
}

std::string FileUtil::removeFileExtensionFromPath( const std::string& path )
{
    const size_t dotIndex = path.rfind( "." );
    if ( dotIndex == std::string::npos )
        return path; // Path has no extension.
    else
        return path.substr( 0, dotIndex );
}