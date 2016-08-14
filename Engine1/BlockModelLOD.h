#pragma once

#include <memory>

#include "BlockModel.h"

namespace Engine1
{
    class BlockModelLOD
    {
        public:
        BlockModelLOD( float distance, std::shared_ptr<BlockModel>& model );
        ~BlockModelLOD();

        float getDistance() const
        {
            return m_distance;
        }

        std::shared_ptr<BlockModel>& getModel()
        {
            return m_model;
        }

        private:
        float m_distance;
        std::shared_ptr<BlockModel> m_model;

        // Copying is not allowed.
        BlockModelLOD( const BlockModelLOD& ) = delete;
        BlockModelLOD& operator=(const BlockModelLOD&) = delete;
    };
}

