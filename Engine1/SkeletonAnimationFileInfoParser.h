#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class SkeletonAnimationFileInfo;

    class SkeletonAnimationFileInfoParser
    {
        friend class SkeletonAnimationFileInfo;

        private:
        static std::shared_ptr<SkeletonAnimationFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
        static void                                       writeBinary( std::vector<char>& data, const SkeletonAnimationFileInfo& fileInfo );
    };
}

