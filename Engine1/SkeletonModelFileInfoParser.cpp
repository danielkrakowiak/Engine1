#include "SkeletonModelFileInfoParser.h"

#include "BinaryFile.h"
#include "SkeletonModelFileInfo.h"

using namespace Engine1;

std::shared_ptr<SkeletonModelFileInfo> SkeletonModelFileInfoParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
    std::shared_ptr<SkeletonModelFileInfo> fileInfo = std::make_shared<SkeletonModelFileInfo>( );

    const int filePathSize = BinaryFile::readInt( dataIt );
    fileInfo->setPath( BinaryFile::readText( dataIt, filePathSize ) );
    fileInfo->setFormat( static_cast<SkeletonModelFileInfo::Format>(BinaryFile::readInt( dataIt )) );

    return fileInfo;
}

void SkeletonModelFileInfoParser::writeBinary( std::vector<char>& data, const SkeletonModelFileInfo& fileInfo )
{
    BinaryFile::writeInt( data, fileInfo.getPath().size() );
    BinaryFile::writeText( data, fileInfo.getPath() );
    BinaryFile::writeInt( data, static_cast<int>(fileInfo.getFormat()) );
}
