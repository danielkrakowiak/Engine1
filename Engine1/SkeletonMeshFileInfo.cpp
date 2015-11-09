#include "SkeletonMeshFileInfo.h"

#include "SkeletonMeshFileInfoParser.h"

#include <memory>

std::shared_ptr<SkeletonMeshFileInfo> SkeletonMeshFileInfo::parseBinary( std::vector<unsigned char>::const_iterator& dataIt )
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

void SkeletonMeshFileInfo::writeBinary( std::vector<unsigned char>& data ) const
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
