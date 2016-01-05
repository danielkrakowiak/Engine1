#include "BlockModelFileInfoParser.h"

#include "BinaryFile.h"
#include "BlockModelFileInfo.h"

using namespace Engine1;

std::shared_ptr<BlockModelFileInfo> BlockModelFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
    std::shared_ptr<BlockModelFileInfo> fileInfo = std::make_shared<BlockModelFileInfo>( );

    const int filePathSize = BinaryFile::readInt( dataIt );
    fileInfo->setPath( BinaryFile::readText( dataIt, filePathSize ) );
    fileInfo->setFormat( static_cast<BlockModelFileInfo::Format>(BinaryFile::readInt( dataIt )) );

    return fileInfo;
}

void BlockModelFileInfoParser::writeBinary( std::vector<char>& data, const BlockModelFileInfo& fileInfo )
{
    BinaryFile::writeInt( data, (int)fileInfo.getPath().size() );
    BinaryFile::writeText( data, fileInfo.getPath() );
    BinaryFile::writeInt( data, static_cast<int>(fileInfo.getFormat()) );
}
