#include "Texture2DFileInfoParser.h"

#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfoParser::parseBinary( std::vector<unsigned char>::const_iterator& dataIt )
{
	std::shared_ptr<Texture2DFileInfo> fileInfo = std::make_shared<Texture2DFileInfo>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	fileInfo->setPath(   BinaryFile::readText( dataIt, filePathSize ) );
	fileInfo->setFormat( static_cast<Texture2DFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );

	return fileInfo;
}

void Texture2DFileInfoParser::writeBinary( std::vector<unsigned char>& data, const Texture2DFileInfo& fileInfo )
{
	BinaryFile::writeInt(  data, fileInfo.getPath( ).size( ) );
	BinaryFile::writeText( data, fileInfo.getPath( ) );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
}