#include "Texture2DFileInfo.h"

#include "Texture2DFileInfoParser.h"

#include <memory>

std::shared_ptr<Texture2DFileInfo> Texture2DFileInfo::parseBinary( std::vector<unsigned char>::const_iterator& dataIt )
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

void Texture2DFileInfo::writeBinary( std::vector<unsigned char>& data ) const
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

std::string Texture2DFileInfo::getPath( ) const
{
	return path;
}

Texture2DFileInfo::Format Texture2DFileInfo::getFormat( ) const
{
	return format;
}