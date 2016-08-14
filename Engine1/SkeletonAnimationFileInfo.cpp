#include "SkeletonAnimationFileInfo.h"

#include "SkeletonAnimationFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonAnimationFileInfo> SkeletonAnimationFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonAnimationFileInfoParser::parseBinary( dataIt );
}

SkeletonAnimationFileInfo::SkeletonAnimationFileInfo() :
    m_path( "" ),
    m_format( Format::XAF ),
    m_invertZCoordinate( false )
{}

SkeletonAnimationFileInfo::SkeletonAnimationFileInfo( std::string path, Format format, const SkeletonMeshFileInfo meshFileInfo, bool invertZCoordinate ) :
    m_path( path ),
    m_format( format ),
    m_meshFileInfo( meshFileInfo ),
    m_invertZCoordinate( invertZCoordinate )
{}

SkeletonAnimationFileInfo::~SkeletonAnimationFileInfo()
{}

std::shared_ptr<FileInfo> SkeletonAnimationFileInfo::clone() const
{
	return std::make_shared<SkeletonAnimationFileInfo>( *this );
}

void SkeletonAnimationFileInfo::saveToMemory( std::vector<char>& data ) const
{
	SkeletonAnimationFileInfoParser::writeBinary( data, *this );
}

void SkeletonAnimationFileInfo::setPath( std::string path )
{
	this->m_path = path;
}

void SkeletonAnimationFileInfo::setFormat( Format format )
{
	this->m_format = format;
}

void SkeletonAnimationFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->m_invertZCoordinate = invertZCoordinate;
}

void SkeletonAnimationFileInfo::setMeshFileInfo( const SkeletonMeshFileInfo meshFileInfo )
{
	this->m_meshFileInfo = meshFileInfo;
}

Asset::Type SkeletonAnimationFileInfo::getAssetType() const
{
	return Asset::Type::SkeletonAnimation;
}

FileInfo::FileType SkeletonAnimationFileInfo::getFileType() const
{
	return FileInfo::FileType::Textual;
}

bool SkeletonAnimationFileInfo::canHaveSubAssets() const
{
    return true; // Note: Animation may require a reference mesh to be loaded.
}

std::string SkeletonAnimationFileInfo::getPath() const
{
	return m_path;
}

SkeletonAnimationFileInfo::Format SkeletonAnimationFileInfo::getFormat() const
{
	return m_format;
}

int SkeletonAnimationFileInfo::getIndexInFile() const
{
	return 0;
}

bool SkeletonAnimationFileInfo::getInvertZCoordinate() const
{
	return m_invertZCoordinate;
}

SkeletonMeshFileInfo SkeletonAnimationFileInfo::getMeshFileInfo( ) const
{
	return m_meshFileInfo;
}