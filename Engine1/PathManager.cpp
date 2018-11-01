#include "AssetPathManager.h"

#include "FileSystem.h"
#include "FileUtil.h"

#include <utility>

#include "StringUtil.h"

#include <windows.h> // Needed only for OutputDebugStringW method.

using namespace Engine1;

void PathManager::scanDirectory( const std::string& directory )
{
    m_paths.clear();

    bool duplicatesFound = false;

    std::vector< std::string > filePaths = FileSystem::getAllFilesFromDirectory( directory );

    for ( const auto& path : filePaths )
    {
        const size_t slashPos = path.rfind('\\');

        const std::string fileName = slashPos != std::string::npos 
            ? path.substr( slashPos + 1 ) : path;

        const auto insertResult = m_paths.insert( std::make_pair( fileName, path ) );
        
        const auto inserted = insertResult.second;
        if ( !inserted )
        {
            duplicatesFound = true;

            const auto& existingPath = insertResult.first->second;

            OutputDebugStringW( StringUtil::widen( "PathManager::initialize - duplicated asset: \n\"" + path + "\"\n\"" + existingPath + "\"\n" ).c_str( ) );
        }
    }

    if ( duplicatesFound )
        throw std::exception( "PathManager::initialize - duplicate assets found." );
}

std::string PathManager::getPathForFileName( const std::string& fileName ) const
{
    if ( fileName.empty() )
        throw std::exception( "PathManager::getPathForFileName - empty file name passed." );

    // Make sure the given file name is really a name, not a path - convert if needed.
    const auto name = FileUtil::getFileNameFromPath( fileName );

    const auto it = m_paths.find( name );

    if ( it == m_paths.end() ) 
    {
        throw std::exception(
            ( std::string( "PathManager::getPathForFileName - filename \"" ) 
            +  fileName + "\" not found." ).c_str()
        );
    }
    else
    {
        return it->second;
    }
}

const std::unordered_map< std::string, std::string >& PathManager::getAllPaths() const
{
    return m_paths;
}



