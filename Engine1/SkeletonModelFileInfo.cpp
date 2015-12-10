#include "SkeletonModelFileInfo.h"

#include "SkeletonModelFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonModelFileInfo> SkeletonModelFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonModelFileInfoParser::parseBinary( dataIt );
}

SkeletonModelFileInfo::SkeletonModelFileInfo( ) :
path( "" ),
format( Format::SKELETONMODEL )
{}

SkeletonModelFileInfo::SkeletonModelFileInfo( std::string path, Format format, int indexInFile ) :
path( path ),
format( format )
{}

SkeletonModelFileInfo::SkeletonModelFileInfo( const SkeletonModelFileInfo& obj ) :
path( obj.path ),
format( obj.format )
{}

SkeletonModelFileInfo::~SkeletonModelFileInfo( )
{}

std::shared_ptr<FileInfo> SkeletonModelFileInfo::clone( ) const
{
	return std::make_shared<SkeletonModelFileInfo>( *this );
}

void SkeletonModelFileInfo::saveToMemory( std::vector<char>& data ) const
{
	SkeletonModelFileInfoParser::writeBinary( data, *this );
}

void SkeletonModelFileInfo::setPath( std::string path )
{
	this->path = path;
}

void SkeletonModelFileInfo::setFormat( Format format )
{
	this->format = format;
}

Asset::Type SkeletonModelFileInfo::getAssetType( ) const
{
	return Asset::Type::SkeletonModel;
}

FileInfo::FileType SkeletonModelFileInfo::getFileType( ) const
{
	return FileInfo::FileType::Binary;
}

std::string SkeletonModelFileInfo::getPath( ) const
{
	return path;
}

SkeletonModelFileInfo::Format SkeletonModelFileInfo::getFormat( ) const
{
	return format;
}

int SkeletonModelFileInfo::getIndexInFile( ) const
{
	return 0;
}