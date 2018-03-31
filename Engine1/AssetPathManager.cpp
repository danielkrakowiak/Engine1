#include "AssetPathManager.h"

#include "FileSystem.h"
#include "FileUtil.h"

#include <utility>

#include "StringUtil.h"

#include <windows.h> // Needed only for OutputDebugStringW method.

using namespace Engine1;

std::unordered_map< std::string, std::string > AssetPathManager::s_paths;

bool s_initialized = AssetPathManager::initialize();

bool AssetPathManager::initialize()
{
    scanAllPaths();

    return true;
}

std::string AssetPathManager::getPathForFileName( const std::string& fileName )
{
    if ( fileName.empty() )
        throw std::exception( "AssetPathManager::getPathForFileName - empty file name passed." );

    // Make sure the given file name is really a name, not a path - convert if needed.
    const auto name = FileUtil::getFileNameFromPath( fileName );

    const auto it = s_paths.find( name );

    if ( it == s_paths.end() ) 
    {
        throw std::exception(
            ( std::string( "AssetPathManager::getPathForFileName - filename \"" ) 
            +  fileName + "\" not found." ).c_str()
        );
    }
    else
    {
        return it->second;
    }
}

void AssetPathManager::scanAllPaths()
{
    s_paths.clear();

    bool duplicatesFound = false;

    std::vector< std::string > filePaths = FileSystem::getAllFilesFromDirectory( "Assets" );

    for ( const auto& path : filePaths )
    {
        const size_t slashPos = path.rfind('\\');

        const std::string fileName = slashPos != std::string::npos 
            ? path.substr( slashPos + 1 ) : path;

        const auto insertResult = s_paths.insert( std::make_pair( fileName, path ) );
        
        const auto inserted = insertResult.second;
        if ( !inserted )
        {
            duplicatesFound = true;

            const auto& existingPath = insertResult.first->second;

            OutputDebugStringW( StringUtil::widen( "AssetPathManager::initialize - duplicated asset: \n\"" + path + "\"\n\"" + existingPath + "\"\n" ).c_str( ) );
        }
    }

    if ( duplicatesFound )
        throw std::exception( "AssetPathManager::initialize - duplicate assets found." );
}

