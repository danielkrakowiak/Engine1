#include "BlockMeshFileInfoParser.h"

#include "BinaryFile.h"
#include "BlockMeshFileInfo.h"

std::shared_ptr<BlockMeshFileInfo> BlockMeshFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<BlockMeshFileInfo> fileInfo = std::make_shared<BlockMeshFileInfo>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	fileInfo->setPath(                     BinaryFile::readText( dataIt, filePathSize ) );
	fileInfo->setIndexInFile(              BinaryFile::readInt( dataIt ) );
	fileInfo->setFormat(                   static_cast<BlockMeshFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
	fileInfo->setInvertZCoordinate(        BinaryFile::readBool( dataIt ) );
	fileInfo->setInvertVertexWindingOrder( BinaryFile::readBool( dataIt ) );
	fileInfo->setFlipUVs(                  BinaryFile::readBool( dataIt ) );

	return fileInfo;
}

void BlockMeshFileInfoParser::writeBinary( std::vector<char>& data, const BlockMeshFileInfo& fileInfo )
{
	BinaryFile::writeInt(  data, fileInfo.getPath( ).size( ) );
	BinaryFile::writeText( data, fileInfo.getPath( ) );
	BinaryFile::writeInt(  data, fileInfo.getIndexInFile( ) );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
	BinaryFile::writeBool( data, fileInfo.getInvertZCoordinate( ) );
	BinaryFile::writeBool( data, fileInfo.getInvertVertexWindingOrder( ) );
	BinaryFile::writeBool( data, fileInfo.getFlipUVs( ) );
}
