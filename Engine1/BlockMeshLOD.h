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
            return m_distance;
        }

        std::shared_ptr<BlockMesh>& getMesh()
        {
            return m_mesh;
        }

        private:
        float m_distance;
        std::shared_ptr<BlockMesh> m_mesh;

        // Copying is not allowed.
        BlockMeshLOD( const BlockMeshLOD& ) = delete;
        BlockMeshLOD& operator=(const BlockMeshLOD&) = delete;
    };
}

