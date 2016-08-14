#include "BlockModelFileInfo.h"

#include "BlockModelFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<BlockModelFileInfo> BlockModelFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return BlockModelFileInfoParser::parseBinary( dataIt );
}

BlockModelFileInfo::BlockModelFileInfo( ) :
    m_path( "" ),
    m_format( Format::BLOCKMODEL )
{}

BlockModelFileInfo::BlockModelFileInfo( std::string path, Format format, int indexInFile ) :
    m_path( path ),
    m_format( format )
{
    indexInFile; // Unused.
}

BlockModelFileInfo::BlockModelFileInfo( const BlockModelFileInfo& obj ) :
    m_path( obj.m_path ),
    m_format( obj.m_format )
{}

BlockModelFileInfo::~BlockModelFileInfo( )
{}

std::shared_ptr<FileInfo> BlockModelFileInfo::clone( ) const
{
	return std::make_shared<BlockModelFileInfo>( *this );
}

void BlockModelFileInfo::saveToMemory( std::vector<char>& data ) const
{
	BlockModelFileInfoParser::writeBinary( data, *this );
}

void BlockModelFileInfo::setPath( std::string path )
{
	this->m_path = path;
}

void BlockModelFileInfo::setFormat( Format format )
{
	this->m_format = format;
}

Asset::Type BlockModelFileInfo::getAssetType( ) const
{
	return Asset::Type::BlockModel;
}

FileInfo::FileType BlockModelFileInfo::getFileType( ) const
{
	return FileInfo::FileType::Binary;
}

bool BlockModelFileInfo::canHaveSubAssets() const
{
    return true;
}

std::string BlockModelFileInfo::getPath( ) const
{
	return m_path;
}

BlockModelFileInfo::Format BlockModelFileInfo::getFormat( ) const
{
	return m_format;
}

int BlockModelFileInfo::getIndexInFile( ) const
{
	return 0;
}