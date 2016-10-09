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
    };
};

