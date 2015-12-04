#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class SkeletonMeshFileInfo;

    class SkeletonMeshFileInfoParser
    {
        friend class SkeletonMeshFileInfo;

        private:
        static std::shared_ptr<SkeletonMeshFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
        static void                                  writeBinary( std::vector<char>& data, const SkeletonMeshFileInfo& fileInfo );
    };
}

