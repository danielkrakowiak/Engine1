#include "BlockModelFileInfoParser.h"

#include "BinaryFile.h"
#include "BlockModelFileInfo.h"

#include "AssetPathManager.h"
#include "FileUtil.h"

using namespace Engine1;

std::shared_ptr<BlockModelFileInfo> BlockModelFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
    std::shared_ptr<BlockModelFileInfo> fileInfo = std::make_shared<BlockModelFileInfo>( );

    const int fileNameSize = BinaryFile::readInt( dataIt );
    const auto fileName = BinaryFile::readText( dataIt, fileNameSize );

    const auto filePath = AssetPathManager::getPathForFileName( fileName );

    fileInfo->setPath( filePath );
    fileInfo->setFormat( static_cast<BlockModelFileInfo::Format>(BinaryFile::readInt( dataIt )) );

    return fileInfo;
}

void BlockModelFileInfoParser::writeBinary( std::vector<char>& data, const BlockModelFileInfo& fileInfo )
{
    const auto fileName = FileUtil::getFileNameFromPath( fileInfo.getPath() );

    BinaryFile::writeInt( data, (int)fileName.size() );
    BinaryFile::writeText( data, fileName );
    BinaryFile::writeInt( data, static_cast<int>(fileInfo.getFormat()) );
}
