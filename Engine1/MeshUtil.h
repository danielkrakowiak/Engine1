#pragma once

#include <memory>
#include <vector>

namespace Engine1
{
    class BlockMesh;
    class float43;

    class MeshUtil
    {
        public:

        static std::shared_ptr< BlockMesh > mergeMeshes( const std::vector< std::shared_ptr< BlockMesh > >& meshes, const std::vector< float43 >& transforms );

        static void flipTexcoordsVertically( BlockMesh& mesh );
        static void flipTangents( BlockMesh& mesh );
        static void flipNormals( BlockMesh& mesh );
        static void invertVertexWindingOrder( BlockMesh& mesh );

        static void transformVertices( BlockMesh& mesh, const float43& transform, const int startVertexIdx, const int endVertexIndex );
    };
};

