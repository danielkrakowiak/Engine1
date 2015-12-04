#include "BlockMeshFileInfo.h"

#include "BlockMeshFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<BlockMeshFileInfo> BlockMeshFileInfo::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	return BlockMeshFileInfoParser::parseBinary( dataIt );
}

BlockMeshFileInfo::BlockMeshFileInfo() : 
	path( "" ),
	format( Format::OBJ ),
	indexInFile( 0 ),
	invertZCoordinate( false ),
	invertVertexWindingOrder( false ),
	flipUVs( false )
{}

BlockMeshFileInfo::BlockMeshFileInfo( std::string path, Format format, int indexInFile, bool invertZCoordinate, bool invertVertexWindingOrder, bool flipUVs ) :
	path( path ),
	format( format ),
	indexInFile( indexInFile ),
	invertZCoordinate( invertZCoordinate ),
	invertVertexWindingOrder( invertVertexWindingOrder ),
	flipUVs( flipUVs )
{}

BlockMeshFileInfo::~BlockMeshFileInfo()
{}

std::shared_ptr<FileInfo> BlockMeshFileInfo::clone( ) const
{
	return std::make_shared<BlockMeshFileInfo>( *this );
}

void BlockMeshFileInfo::writeBinary( std::vector<char>& data ) const
{
	BlockMeshFileInfoParser::writeBinary( data, *this );
}

void BlockMeshFileInfo::setPath( std::string path ) 
{
	this->path = path;
}

void BlockMeshFileInfo::setFormat( Format format )
{
	this->format = format;
}

void BlockMeshFileInfo::setIndexInFile( int indexInFile )
{
	this->indexInFile = indexInFile;
}

void BlockMeshFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->invertZCoordinate = invertZCoordinate;
}

void BlockMeshFileInfo::setInvertVertexWindingOrder( bool invertVertexWindingOrder )
{
	this->invertVertexWindingOrder = invertVertexWindingOrder;
}

void BlockMeshFileInfo::setFlipUVs( bool flipUVs )
{
	this->flipUVs = flipUVs;
}

Asset::Type BlockMeshFileInfo::getAssetType( ) const
{
	return Asset::Type::BlockMesh;
}

FileInfo::FileType BlockMeshFileInfo::getFileType() const
{
	return FileInfo::FileType::Textual;
}

std::string BlockMeshFileInfo::getPath() const
{
	return path;
}

BlockMeshFileInfo::Format BlockMeshFileInfo::getFormat() const
{
	return format;
}

int BlockMeshFileInfo::getIndexInFile() const
{
	return indexInFile;
}

bool BlockMeshFileInfo::getInvertZCoordinate() const
{
	return invertZCoordinate;
}

bool BlockMeshFileInfo::getInvertVertexWindingOrder() const
{
	return invertVertexWindingOrder;
}

bool BlockMeshFileInfo::getFlipUVs() const
{
	return flipUVs;
}
