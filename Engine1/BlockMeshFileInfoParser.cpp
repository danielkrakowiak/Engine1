#include "BlockMeshFileInfoParser.h"

#include "BinaryFile.h"
#include "BlockMeshFileInfo.h"

#include "FileUtil.h"
#include "AssetPathManager.h"

using namespace Engine1;

std::shared_ptr<BlockMeshFileInfo> BlockMeshFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<BlockMeshFileInfo> fileInfo = std::make_shared<BlockMeshFileInfo>( );

	const int fileNameSize = BinaryFile::readInt( dataIt );
    const auto fileName = BinaryFile::readText( dataIt, fileNameSize );

    const auto filePath = AssetPathManager::getPathForFileName( fileName );

	fileInfo->setPath(                     filePath );
	fileInfo->setIndexInFile(              BinaryFile::readInt( dataIt ) );
	fileInfo->setFormat(                   static_cast<BlockMeshFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
	fileInfo->setInvertZCoordinate(        BinaryFile::readBool( dataIt ) );
	fileInfo->setInvertVertexWindingOrder( BinaryFile::readBool( dataIt ) );
	fileInfo->setFlipUVs(                  BinaryFile::readBool( dataIt ) );

	return fileInfo;
}

void BlockMeshFileInfoParser::writeBinary( std::vector<char>& data, const BlockMeshFileInfo& fileInfo )
{
    const auto fileName = FileUtil::getFileNameFromPath( fileInfo.getPath() );

	BinaryFile::writeInt(  data, (int)fileName.size( ) );
	BinaryFile::writeText( data, fileName );
	BinaryFile::writeInt(  data, fileInfo.getIndexInFile( ) );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
	BinaryFile::writeBool( data, fileInfo.getInvertZCoordinate( ) );
	BinaryFile::writeBool( data, fileInfo.getInvertVertexWindingOrder( ) );
	BinaryFile::writeBool( data, fileInfo.getFlipUVs( ) );
}
