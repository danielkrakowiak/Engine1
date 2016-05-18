#include "Texture2DFileInfoParser.h"

#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<Texture2DFileInfo> fileInfo = std::make_shared<Texture2DFileInfo>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	fileInfo->setPath(   BinaryFile::readText( dataIt, filePathSize ) );
	fileInfo->setFormat( static_cast<Texture2DFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
    fileInfo->setPixelType( static_cast<Texture2DFileInfo::PixelType>( BinaryFile::readInt( dataIt ) ) );

	return fileInfo;
}

void Texture2DFileInfoParser::writeBinary( std::vector<char>& data, const Texture2DFileInfo& fileInfo )
{
	BinaryFile::writeInt(  data, (int)fileInfo.getPath( ).size( ) );
	BinaryFile::writeText( data, fileInfo.getPath( ) );
	BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getFormat( ) ) );
    BinaryFile::writeInt(  data, static_cast<int>( fileInfo.getPixelType( ) ) );
}