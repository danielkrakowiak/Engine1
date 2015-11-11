#include "Texture2DFileInfo.h"

#include "Texture2DFileInfoParser.h"

#include <memory>

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfo::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	return Texture2DFileInfoParser::parseBinary( dataIt );
}

Texture2DFileInfo::Texture2DFileInfo( ) :
path( "" ),
format( Format::BMP )
{}

Texture2DFileInfo::Texture2DFileInfo( std::string path, Format format ) :
path( path ),
format( format )
{}

Texture2DFileInfo::~Texture2DFileInfo( )
{}

std::shared_ptr<FileInfo> Texture2DFileInfo::clone() const
{
	return std::make_shared<Texture2DFileInfo>( *this );
}

void Texture2DFileInfo::writeBinary( std::vector<char>& data ) const
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

Asset::Type Texture2DFileInfo::getAssetType() const
{
	return Asset::Type::Texture2D;
}

FileInfo::FileType Texture2DFileInfo::getFileType() const
{
	return FileInfo::FileType::Binary;
}

std::string Texture2DFileInfo::getPath( ) const
{
	return path;
}

Texture2DFileInfo::Format Texture2DFileInfo::getFormat( ) const
{
	return format;
}