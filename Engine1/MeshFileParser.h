#pragma once

#include <vector>
#include <memory>

#include "BlockMeshFileInfo.h"

namespace Engine1
{
    class BlockMesh;
    class SkeletonMesh;

    class MeshFileParser
    {
        public:
        static std::vector< std::shared_ptr<BlockMesh> >    parseBlockMeshFile( BlockMeshFileInfo::Format format, std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
        static std::vector< std::shared_ptr<SkeletonMesh> > parseSkeletonMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );

        static void writeBlockMeshFile( std::vector< char >& data, const BlockMeshFileInfo::Format format, const BlockMesh& mesh );

        private:

        MeshFileParser();
        ~MeshFileParser();

        static std::vector< std::shared_ptr<BlockMesh> > parseBlockMeshFileAssimp( BlockMeshFileInfo::Format format, std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
        static std::shared_ptr<BlockMesh>                parseBlockMeshFileOwnFormat( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt );

        static std::vector< std::shared_ptr<SkeletonMesh> > parseSkeletonMeshFileAssimp( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
        static std::shared_ptr<SkeletonMesh>                parseSkeletonMeshFileOwnFormat( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt );

        static void writeBlockMeshFileAssimp( std::vector< char >& data, const BlockMeshFileInfo::Format format, const BlockMesh& mesh );
        static void writeBlockMeshFileOwnFormat( std::vector< char >& data, const BlockMesh& mesh );
    };
}

