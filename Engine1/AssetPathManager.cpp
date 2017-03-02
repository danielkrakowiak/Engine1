#include "AssetPathManager.h"

#include "FileSystem.h"
#include "FileUtil.h"

#include <utility>

using namespace Engine1;

std::unordered_map< std::string, std::string > AssetPathManager::s_paths;

bool s_initialized = AssetPathManager::initialize();

bool AssetPathManager::initialize()
{
    std::vector< std::string > filePaths = FileSystem::getAllFilesFromDirectory( "Assets" );

    for ( const auto& path : filePaths )
    {
        const size_t slashPos = path.rfind('\\');

        const std::string fileName = slashPos != std::string::npos 
            ? path.substr( slashPos + 1 ) : path;

        s_paths.insert( std::make_pair( fileName, path ) );
    }

    return true;
}

std::string AssetPathManager::getPathForFileName( const std::string& fileName )
{
    // Make sure the given file name is really a name, not a path - convert if needed.
    const auto name = FileUtil::getFileNameFromPath( fileName );

    const auto it = s_paths.find( name );

    if ( it == s_paths.end() )
        return "";
    else
        return it->second;
}

