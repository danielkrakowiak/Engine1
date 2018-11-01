#include "SkeletonAnimationFileInfoParser.h"

#include "BinaryFile.h"
#include "SkeletonAnimationFileInfo.h"

#include "AssetPathManager.h"
#include "FileUtil.h"

using namespace Engine1;

std::shared_ptr<SkeletonAnimationFileInfo> SkeletonAnimationFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
	std::shared_ptr<SkeletonAnimationFileInfo> fileInfo = std::make_shared<SkeletonAnimationFileInfo>( );

    const int fileNameSize = BinaryFile::readInt( dataIt );
    const auto fileName = BinaryFile::readText( dataIt, fileNameSize );

    const auto filePath = AssetPathManager::get().getPathForFileName( fileName );

	fileInfo->setPath( filePath );
	fileInfo->setFormat( static_cast<SkeletonAnimationFileInfo::Format>( BinaryFile::readInt( dataIt ) ) );
	fileInfo->setInvertZCoordinate( BinaryFile::readBool( dataIt ) );

	return fileInfo;
}

void SkeletonAnimationFileInfoParser::writeBinary( std::vector<char>& data, const SkeletonAnimationFileInfo& fileInfo )
{
    const auto fileName = FileUtil::getFileNameFromPath( fileInfo.getPath() );

	BinaryFile::writeInt( data, (int)fileName.size() );
	BinaryFile::writeText( data, fileName );
	BinaryFile::writeInt( data, static_cast<int>( fileInfo.getFormat() ) );
	BinaryFile::writeBool( data, fileInfo.getInvertZCoordinate() );
}
