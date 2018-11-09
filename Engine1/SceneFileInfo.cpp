#include "SceneFileInfo.h"

#include <memory>
#include <assert.h>

using namespace Engine1;

SceneFileInfo::SceneFileInfo( ) :
    m_path( "" )
{}

SceneFileInfo::SceneFileInfo( std::string path ) :
    m_path( path )
{}

SceneFileInfo::SceneFileInfo( const SceneFileInfo& obj ) :
    m_path( obj.m_path )
{}

SceneFileInfo::~SceneFileInfo( )
{}

std::shared_ptr<FileInfo> SceneFileInfo::clone( ) const
{
	return std::make_shared<SceneFileInfo>( *this );
}

void SceneFileInfo::saveToMemory( std::vector<char>& data ) const
{
    data;

	assert(false); // #TODO: Implement.
}

void SceneFileInfo::setPath( std::string path )
{
	this->m_path = path;
}

Asset::Type SceneFileInfo::getAssetType( ) const
{
	return Asset::Type::Scene;
}

FileInfo::FileType SceneFileInfo::getFileType( ) const
{
	return FileInfo::FileType::Binary;
}

bool SceneFileInfo::canHaveSubAssets() const
{
    return true;
}

std::string SceneFileInfo::getPath( ) const
{
	return m_path;
}

int SceneFileInfo::getIndexInFile() const
{
    return 0;
}