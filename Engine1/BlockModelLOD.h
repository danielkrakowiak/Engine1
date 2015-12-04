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
            return distance;
        }

        std::shared_ptr<BlockModel>& getModel()
        {
            return model;
        }

        private:
        float distance;
        std::shared_ptr<BlockModel> model;

        // Copying is not allowed.
        BlockModelLOD( const BlockModelLOD& ) = delete;
        BlockModelLOD& operator=(const BlockModelLOD&) = delete;
    };
}

