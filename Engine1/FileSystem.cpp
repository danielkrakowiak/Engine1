#include "FileSystem.h"

#include <experimental/filesystem>

#include "StringUtil.h"

using namespace Engine1;

long long FileSystem::getFileSize( const std::string& path )
{
    const int fileSize = (int)std::experimental::filesystem::file_size( path );

    return fileSize;
}

std::vector< std::string > FileSystem::getAllFilesFromDirectory( std::string directoryPath )
{
    if ( directoryPath.empty() )
        throw std::exception( "FileSystem::getAllFilesFromDirectory - received an empty path as argument." );

    std::vector< std::string > searchResults;

    // Reserve some memory to avoid too many memory allocations.
    // #TODO: Should check the number of found files instead (if possible).
    const int reserveMemoryFileCount = 50;
    searchResults.reserve( reserveMemoryFileCount );

    std::experimental::filesystem::recursive_directory_iterator fileIt( directoryPath );

    for ( const auto& file : fileIt )
    {
        if ( !std::experimental::filesystem::is_directory( file.path() ) )
            searchResults.push_back( file.path().string() );
    }

    return std::move( searchResults );
}
