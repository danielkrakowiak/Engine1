#include "SkeletonMeshFileInfo.h"

#include "SkeletonMeshFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonMeshFileInfo> SkeletonMeshFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonMeshFileInfoParser::parseBinary( dataIt );
}

SkeletonMeshFileInfo::SkeletonMeshFileInfo( ) :
m_path( "" ),
m_format( Format::DAE ),
m_indexInFile( 0 ),
m_invertZCoordinate( false ),
m_invertVertexWindingOrder( false ),
m_flipUVs( false )
{}

SkeletonMeshFileInfo::SkeletonMeshFileInfo( std::string path, Format format, int indexInFile, bool invertZCoordinate, bool invertVertexWindingOrder, bool flipUVs ) :
    m_path( path ),
    m_format( format ),
    m_indexInFile( indexInFile ),
    m_invertZCoordinate( invertZCoordinate ),
    m_invertVertexWindingOrder( invertVertexWindingOrder ),
    m_flipUVs( flipUVs )
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
	this->m_path = path;
}

void SkeletonMeshFileInfo::setFormat( Format format )
{
	this->m_format = format;
}

void SkeletonMeshFileInfo::setIndexInFile( int indexInFile )
{
	this->m_indexInFile = indexInFile;
}

void SkeletonMeshFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->m_invertZCoordinate = invertZCoordinate;
}

void SkeletonMeshFileInfo::setInvertVertexWindingOrder( bool invertVertexWindingOrder )
{
	this->m_invertVertexWindingOrder = invertVertexWindingOrder;
}

void SkeletonMeshFileInfo::setFlipUVs( bool flipUVs )
{
	this->m_flipUVs = flipUVs;
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
	return m_path;
}

SkeletonMeshFileInfo::Format SkeletonMeshFileInfo::getFormat( ) const
{
	return m_format;
}

int SkeletonMeshFileInfo::getIndexInFile( ) const
{
	return m_indexInFile;
}

bool SkeletonMeshFileInfo::getInvertZCoordinate( ) const
{
	return m_invertZCoordinate;
}

bool SkeletonMeshFileInfo::getInvertVertexWindingOrder( ) const
{
	return m_invertVertexWindingOrder;
}

bool SkeletonMeshFileInfo::getFlipUVs( ) const
{
	return m_flipUVs;
}
