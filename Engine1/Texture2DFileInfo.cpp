#include "Texture2DFileInfo.h"

#include "Texture2DFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return Texture2DFileInfoParser::parseBinary( dataIt );
}

Texture2DFileInfo::Texture2DFileInfo( ) :
path( "" ),
format( Format::BMP ),
pixelType( PixelType::UCHAR4 )
{}

Texture2DFileInfo::Texture2DFileInfo( std::string path, Format format, PixelType pixelType ) :
path( path ),
format( format ),
pixelType( pixelType )
{}

Texture2DFileInfo::~Texture2DFileInfo( )
{}

std::shared_ptr<FileInfo> Texture2DFileInfo::clone() const
{
	return std::make_shared<Texture2DFileInfo>( *this );
}

void Texture2DFileInfo::saveToMemory( std::vector<char>& data ) const
{
	Texture2DFileInfoParser::writeBinary( data, *this );
}

void Texture2DFileInfo::setPath( std::string path )
{
	this->path = path;
}

void Texture2DFileInfo::setFormat( Format format )
{
	this->format = format;
}

void Texture2DFileInfo::setPixelType( PixelType pixelType )
{
    this->pixelType = pixelType;
}

std::string Texture2DFileInfo::getPath() const
{
	return path;
}

int Texture2DFileInfo::getIndexInFile() const
{
	return 0;
}

Asset::Type Texture2DFileInfo::getAssetType() const
{
	return Asset::Type::Texture2D;
}

FileInfo::FileType Texture2DFileInfo::getFileType() const
{
	return FileInfo::FileType::Binary;
}

Texture2DFileInfo::Format Texture2DFileInfo::getFormat( ) const
{
	return format;
}

Texture2DFileInfo::PixelType Texture2DFileInfo::getPixelType() const
{
    return pixelType;
}