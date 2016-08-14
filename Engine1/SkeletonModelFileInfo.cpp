#include "SkeletonModelFileInfo.h"

#include "SkeletonModelFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonModelFileInfo> SkeletonModelFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonModelFileInfoParser::parseBinary( dataIt );
}

SkeletonModelFileInfo::SkeletonModelFileInfo( ) :
    m_path( "" ),
    m_format( Format::SKELETONMODEL )
{}

SkeletonModelFileInfo::SkeletonModelFileInfo( std::string path, Format format, int indexInFile ) :
    m_path( path ),
    m_format( format )
{
    indexInFile; // Unused.
}

SkeletonModelFileInfo::SkeletonModelFileInfo( const SkeletonModelFileInfo& obj ) :
    m_path( obj.m_path ),
    m_format( obj.m_format )
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
	this->m_path = path;
}

void SkeletonModelFileInfo::setFormat( Format format )
{
	this->m_format = format;
}

Asset::Type SkeletonModelFileInfo::getAssetType( ) const
{
	return Asset::Type::SkeletonModel;
}

FileInfo::FileType SkeletonModelFileInfo::getFileType( ) const
{
	return FileInfo::FileType::Binary;
}

bool SkeletonModelFileInfo::canHaveSubAssets() const
{
    return true;
}

std::string SkeletonModelFileInfo::getPath( ) const
{
	return m_path;
}

SkeletonModelFileInfo::Format SkeletonModelFileInfo::getFormat( ) const
{
	return m_format;
}

int SkeletonModelFileInfo::getIndexInFile( ) const
{
	return 0;
}