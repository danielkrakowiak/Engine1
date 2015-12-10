#include "SkeletonAnimationFileInfo.h"

#include "SkeletonAnimationFileInfoParser.h"

#include <memory>

using namespace Engine1;

std::shared_ptr<SkeletonAnimationFileInfo> SkeletonAnimationFileInfo::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
	return SkeletonAnimationFileInfoParser::parseBinary( dataIt );
}

SkeletonAnimationFileInfo::SkeletonAnimationFileInfo() :
path( "" ),
format( Format::XAF ),
invertZCoordinate( false )
{}

SkeletonAnimationFileInfo::SkeletonAnimationFileInfo( std::string path, Format format, const SkeletonMeshFileInfo meshFileInfo, bool invertZCoordinate ) :
path( path ),
format( format ),
meshFileInfo( meshFileInfo ),
invertZCoordinate( invertZCoordinate )
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
	this->path = path;
}

void SkeletonAnimationFileInfo::setFormat( Format format )
{
	this->format = format;
}

void SkeletonAnimationFileInfo::setInvertZCoordinate( bool invertZCoordinate )
{
	this->invertZCoordinate = invertZCoordinate;
}

void SkeletonAnimationFileInfo::setMeshFileInfo( const SkeletonMeshFileInfo meshFileInfo )
{
	this->meshFileInfo = meshFileInfo;
}

Asset::Type SkeletonAnimationFileInfo::getAssetType() const
{
	return Asset::Type::SkeletonAnimation;
}

FileInfo::FileType SkeletonAnimationFileInfo::getFileType() const
{
	return FileInfo::FileType::Textual;
}

std::string SkeletonAnimationFileInfo::getPath() const
{
	return path;
}

SkeletonAnimationFileInfo::Format SkeletonAnimationFileInfo::getFormat() const
{
	return format;
}

int SkeletonAnimationFileInfo::getIndexInFile() const
{
	return 0;
}

bool SkeletonAnimationFileInfo::getInvertZCoordinate() const
{
	return invertZCoordinate;
}

SkeletonMeshFileInfo SkeletonAnimationFileInfo::getMeshFileInfo( ) const
{
	return meshFileInfo;
}