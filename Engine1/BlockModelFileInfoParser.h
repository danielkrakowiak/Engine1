#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class BlockModelFileInfo;

    class BlockModelFileInfoParser
    {
        friend class BlockModelFileInfo;

        private:
        static std::shared_ptr<BlockModelFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
        static void                                writeBinary( std::vector<char>& data, const BlockModelFileInfo& fileInfo );
    };
}

