#include "BlockMeshFileInfo.h"

#include "BlockMeshFileInfoParser.h"

#include <memory>
#include <assert.h>

using namespace Engine1;

std::shared_ptr<BlockMeshFileInfo> BlockMeshFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return BlockMeshFileInfoParser::parseBinary( dataIt );
}

BlockMeshFileInfo::FileType BlockMeshFileInfo::getFileTypeFromFormat( const BlockMeshFileInfo::Format format )
{
    if ( format == BlockMeshFileInfo::Format::OBJ || format == BlockMeshFileInfo::Format::DAE )
        return FileInfo::FileType::Textual;
    else
        return FileInfo::FileType::Binary;
}

std::string BlockMeshFileInfo::formatToString( const Format format )
{
    switch ( format )
    {
        case Format::OBJ:
            return "obj";
        case Format::DAE:
            return "dae";
        case Format::FBX:
            return "fbx";
    }

    assert( false );
    return "";
}

BlockMeshFileInfo::BlockMeshFileInfo() : 
	m_path( "" ),
	m_format( Format::OBJ ),
	m_indexInFile( 0 ),
	m_invertZCoordinate( false ),
	m_invertVertexWindingOrder( false ),
	m_flipUVs( false )
{}

BlockMeshFileInfo::BlockMeshFileInfo( std::string path, Format format, int indexInFile, bool invertZCoordinate, bool invertVertexWindingOrder, bool flipUVs ) :
    m_path( path ),
    m_format( format ),
    m_indexInFile( indexInFile ),
    m_invertZCoordinate( invertZCoordinate ),
    m_invertVertexWindingOrder( invertVertexWindingOrder ),
    m_flipUVs( flipUVs )
{}

BlockMeshFileInfo::~BlockMeshFileInfo()
{}

std::shared_ptr<FileInfo> BlockMeshFileInfo::clone( ) const
{
	return std::make_shared<BlockMeshFileInfo>( *this );
}

void BlockMeshFileInfo::saveToMemory( std::vector<char>& data ) const
{
	BlockMeshFileInfoParser::writeBinary( data, *this );
}

void BlockMeshFileInfo::setPath( std::string path ) 
{
	this->m_path = path;
}

void BlockMeshFileInfo::setFormat( Format format )
{
	this->m_format = format;
}

void BlockMeshFileInfo::setIndexInFile( int indexInFile )
{
	this->m_indexInFile = indexInFile;
}

void BlockMeshFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->m_invertZCoordinate = invertZCoordinate;
}

void BlockMeshFileInfo::setInvertVertexWindingOrder( bool invertVertexWindingOrder )
{
	this->m_invertVertexWindingOrder = invertVertexWindingOrder;
}

void BlockMeshFileInfo::setFlipUVs( bool flipUVs )
{
	this->m_flipUVs = flipUVs;
}

Asset::Type BlockMeshFileInfo::getAssetType( ) const
{
	return Asset::Type::BlockMesh;
}

FileInfo::FileType BlockMeshFileInfo::getFileType() const
{
    return getFileTypeFromFormat( m_format );
}

bool BlockMeshFileInfo::canHaveSubAssets() const
{
    return false;
}

std::string BlockMeshFileInfo::getPath() const
{
	return m_path;
}

BlockMeshFileInfo::Format BlockMeshFileInfo::getFormat() const
{
	return m_format;
}

int BlockMeshFileInfo::getIndexInFile() const
{
	return m_indexInFile;
}

bool BlockMeshFileInfo::getInvertZCoordinate() const
{
	return m_invertZCoordinate;
}

bool BlockMeshFileInfo::getInvertVertexWindingOrder() const
{
	return m_invertVertexWindingOrder;
}

bool BlockMeshFileInfo::getFlipUVs() const
{
	return m_flipUVs;
}
