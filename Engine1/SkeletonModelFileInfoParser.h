#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class SkeletonModelFileInfo;

    class SkeletonModelFileInfoParser
    {
        friend class SkeletonModelFileInfo;

        private:
        static std::shared_ptr<SkeletonModelFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
        static void                                   writeBinary( std::vector<char>& data, const SkeletonModelFileInfo& fileInfo );
    };
}

