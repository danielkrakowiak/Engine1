#include "SkeletonMeshFileInfo.h"

#include "SkeletonMeshFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonMeshFileInfo> SkeletonMeshFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonMeshFileInfoParser::parseBinary( dataIt );
}

SkeletonMeshFileInfo::SkeletonMeshFileInfo( ) :
path( "" ),
format( Format::DAE ),
indexInFile( 0 ),
invertZCoordinate( false ),
invertVertexWindingOrder( false ),
flipUVs( false )
{}

SkeletonMeshFileInfo::SkeletonMeshFileInfo( std::string path, Format format, int indexInFile, bool invertZCoordinate, bool invertVertexWindingOrder, bool flipUVs ) :
path( path ),
format( format ),
indexInFile( indexInFile ),
invertZCoordinate( invertZCoordinate ),
invertVertexWindingOrder( invertVertexWindingOrder ),
flipUVs( flipUVs )
{}

SkeletonMeshFileInfo::~SkeletonMeshFileInfo( )
{}

std::shared_ptr<FileInfo> SkeletonMeshFileInfo::clone( ) const
{
	return std::make_shared<SkeletonMeshFileInfo>( *this );
}

void SkeletonMeshFileInfo::saveToMemory( std::vector<char>& data ) const
{
	SkeletonMeshFileInfoParser::writeBinary( data, *this );
}

void SkeletonMeshFileInfo::setPath( std::string path )
{
	this->path = path;
}

void SkeletonMeshFileInfo::setFormat( Format format )
{
	this->format = format;
}

void SkeletonMeshFileInfo::setIndexInFile( int indexInFile )
{
	this->indexInFile = indexInFile;
}

void SkeletonMeshFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->invertZCoordinate = invertZCoordinate;
}

void SkeletonMeshFileInfo::setInvertVertexWindingOrder( bool invertVertexWindingOrder )
{
	this->invertVertexWindingOrder = invertVertexWindingOrder;
}

void SkeletonMeshFileInfo::setFlipUVs( bool flipUVs )
{
	this->flipUVs = flipUVs;
}

Asset::Type SkeletonMeshFileInfo::getAssetType( ) const
{
	return Asset::Type::SkeletonMesh;
}

FileInfo::FileType SkeletonMeshFileInfo::getFileType( ) const
{
	return FileInfo::FileType::Textual;
}

bool SkeletonMeshFileInfo::canHaveSubAssets() const
{
    return false;
}

std::string SkeletonMeshFileInfo::getPath( ) const
{
	return path;
}

SkeletonMeshFileInfo::Format SkeletonMeshFileInfo::getFormat( ) const
{
	return format;
}

int SkeletonMeshFileInfo::getIndexInFile( ) const
{
	return indexInFile;
}

bool SkeletonMeshFileInfo::getInvertZCoordinate( ) const
{
	return invertZCoordinate;
}

bool SkeletonMeshFileInfo::getInvertVertexWindingOrder( ) const
{
	return invertVertexWindingOrder;
}

bool SkeletonMeshFileInfo::getFlipUVs( ) const
{
	return flipUVs;
}
