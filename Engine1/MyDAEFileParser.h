#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class BlockMesh;
    class SkeletonMesh;

    class MyDAEFileParser
    {
        private:
        MyDAEFileParser();
        ~MyDAEFileParser();

        public:
        static std::vector< std::shared_ptr<BlockMesh> >    parseBlockMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
        static std::vector< std::shared_ptr<SkeletonMesh> > parseSkeletonMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
    };
}

