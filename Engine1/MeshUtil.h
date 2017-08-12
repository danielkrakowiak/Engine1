#pragma once

#include <memory>
#include <vector>

namespace Engine1
{
    class BlockMesh;

    class MeshUtil
    {
        public:

        static std::shared_ptr< BlockMesh > mergeMeshes( const std::vector< std::shared_ptr< BlockMesh > >& meshes );

        static void flipTexcoordsVertically( BlockMesh& mesh );
        static void flipTangents( BlockMesh& mesh );
        static void flipNormals( BlockMesh& mesh );
        static void invertVertexWindingOrder( BlockMesh& mesh );
    };
};

