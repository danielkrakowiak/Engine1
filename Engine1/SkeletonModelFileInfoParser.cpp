#include "SkeletonModelFileInfoParser.h"

#include "BinaryFile.h"
#include "SkeletonModelFileInfo.h"

#include "AssetPathManager.h"
#include "FileUtil.h"

using namespace Engine1;

std::shared_ptr<SkeletonModelFileInfo> SkeletonModelFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
    std::shared_ptr<SkeletonModelFileInfo> fileInfo = std::make_shared<SkeletonModelFileInfo>( );

    const int fileNameSize = BinaryFile::readInt( dataIt );
    const auto fileName = BinaryFile::readText( dataIt, fileNameSize );

    const auto filePath = AssetPathManager::getPathForFileName( fileName );

    fileInfo->setPath( filePath );
    fileInfo->setFormat( static_cast<SkeletonModelFileInfo::Format>(BinaryFile::readInt( dataIt )) );

    return fileInfo;
}

void SkeletonModelFileInfoParser::writeBinary( std::vector<char>& data, const SkeletonModelFileInfo& fileInfo )
{
    const auto fileName = FileUtil::getFileNameFromPath( fileInfo.getPath() );

    BinaryFile::writeInt( data, (int)fileName.size() );
    BinaryFile::writeText( data, fileName );
    BinaryFile::writeInt( data, static_cast<int>(fileInfo.getFormat()) );
}
