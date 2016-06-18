#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class BlockMesh;
    class SkeletonMesh;

    class MyOBJFileParser
    {
        private:
        MyOBJFileParser();
        ~MyOBJFileParser();

        public:
        static std::vector< std::shared_ptr<BlockMesh> >    parseBlockMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
    };
}

