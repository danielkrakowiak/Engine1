#pragma once

#include <memory>

#include "BlockMesh.h"

namespace Engine1
{
    class BlockMeshLOD
    {
        public:
        BlockMeshLOD( float distance, std::shared_ptr<BlockMesh>& mesh );
        ~BlockMeshLOD();

        float getDistance() const
        {
            return distance;
        }

        std::shared_ptr<BlockMesh>& getMesh()
        {
            return mesh;
        }

        private:
        float distance;
        std::shared_ptr<BlockMesh> mesh;

        // Copying is not allowed.
        BlockMeshLOD( const BlockMeshLOD& ) = delete;
        BlockMeshLOD& operator=(const BlockMeshLOD&) = delete;
    };
}

