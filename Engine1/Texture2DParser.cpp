#include "Texture2DParser.h"

#include "Texture2D.h"
#include "BinaryFile.h"

std::shared_ptr<Texture2D> Texture2DParser::parseFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt )
{
	std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	texture->filePath      = BinaryFile::readText( dataIt, filePathSize );
	texture->fileFormat    = static_cast<Texture2D::FileFormat>( BinaryFile::readInt( dataIt ) );

	return texture;
}

void Texture2DParser::writeFileInfoBinary( std::vector<unsigned char>& data, const Texture2D& texture )
{
	BinaryFile::writeInt( data, texture.getFilePath( ).size( ) );
	BinaryFile::writeText( data, texture.getFilePath( ) );
	BinaryFile::writeInt( data, static_cast<int>( texture.getFileFormat( ) ) );
}