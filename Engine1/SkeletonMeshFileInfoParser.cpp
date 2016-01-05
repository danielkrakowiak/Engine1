#include "SkeletonMeshFileInfoParser.h"

#include "BinaryFile.h"
#include "SkeletonMeshFileInfo.h"

using namespace Engine1;

std::shared_ptr<SkeletonMeshFileInfo> SkeletonMeshFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<SkeletonMeshFileInfo> fileInfo = std::make_shared<SkeletonMeshFileInfo>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	fileInfo->setPath(                     BinaryFile::readText( dataIt, filePathSize ) );
	fileInfo->setIndexInFile(              BinaryFile::readInt( dataIt ) );
	fileInfo->setFormat(                   static_cast<SkeletonMeshFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
	fileInfo->setInvertZCoordinate(        BinaryFile::readBool( dataIt ) );
	fileInfo->setInvertVertexWindingOrder( BinaryFile::readBool( dataIt ) );
	fileInfo->setFlipUVs(                  BinaryFile::readBool( dataIt ) );

	return fileInfo;
}

void SkeletonMeshFileInfoParser::writeBinary( std::vector<char>& data, const SkeletonMeshFileInfo& fileInfo )
{
	BinaryFile::writeInt(  data, (int)fileInfo.getPath( ).size( ) );
	BinaryFile::writeText( data, fileInfo.getPath( ) );
	BinaryFile::writeInt(  data, fileInfo.getIndexInFile( ) );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
	BinaryFile::writeBool( data, fileInfo.getInvertZCoordinate( ) );
	BinaryFile::writeBool( data, fileInfo.getInvertVertexWindingOrder( ) );
	BinaryFile::writeBool( data, fileInfo.getFlipUVs( ) );
}
