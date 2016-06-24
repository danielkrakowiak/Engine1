#include "BlockModelFileInfo.h"

#include "BlockModelFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<BlockModelFileInfo> BlockModelFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return BlockModelFileInfoParser::parseBinary( dataIt );
}

BlockModelFileInfo::BlockModelFileInfo( ) :
path( "" ),
format( Format::BLOCKMODEL )
{}

BlockModelFileInfo::BlockModelFileInfo( std::string path, Format format, int indexInFile ) :
path( path ),
format( format )
{
    indexInFile; // Unused.
}

BlockModelFileInfo::BlockModelFileInfo( const BlockModelFileInfo& obj ) :
path( obj.path ),
format( obj.format )
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
	this->path = path;
}

void BlockModelFileInfo::setFormat( Format format )
{
	this->format = format;
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
	return path;
}

BlockModelFileInfo::Format BlockModelFileInfo::getFormat( ) const
{
	return format;
}

int BlockModelFileInfo::getIndexInFile( ) const
{
	return 0;
}