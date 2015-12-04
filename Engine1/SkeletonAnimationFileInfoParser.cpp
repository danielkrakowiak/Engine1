#include "SkeletonAnimationFileInfoParser.h"

#include "BinaryFile.h"
#include "SkeletonAnimationFileInfo.h"

using namespace Engine1;

std::shared_ptr<SkeletonAnimationFileInfo> SkeletonAnimationFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<SkeletonAnimationFileInfo> fileInfo = std::make_shared<SkeletonAnimationFileInfo>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	fileInfo->setPath( BinaryFile::readText( dataIt, filePathSize ) );
	fileInfo->setFormat( static_cast<SkeletonAnimationFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
	fileInfo->setInvertZCoordinate( BinaryFile::readBool( dataIt ) );

	return fileInfo;
}

void SkeletonAnimationFileInfoParser::writeBinary( std::vector<char>& data, const SkeletonAnimationFileInfo& fileInfo )
{
	BinaryFile::writeInt( data, fileInfo.getPath().size() );
	BinaryFile::writeText( data, fileInfo.getPath() );
	BinaryFile::writeInt( data, static_cast<int>( fileInfo.getFormat() ) );
	BinaryFile::writeBool( data, fileInfo.getInvertZCoordinate() );
}
