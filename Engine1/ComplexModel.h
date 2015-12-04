#pragma once

#include <vector>

#include "BlockModelLOD.h"
#include "BlockMeshLOD.h"

//#TODO: add copy ctor

namespace Engine1
{
    class ComplexModel
    {

        public:
        ComplexModel();
        ~ComplexModel();

        std::shared_ptr<BlockModel>& getModelLOD( float distance );
        std::shared_ptr<BlockMesh>& getShadowMeshLOD( float distance );

        private:

        std::vector<BlockModelLOD> modelLODs;
        std::vector<BlockMeshLOD> shadowMeshLODs;

        // Copying is not allowed.
        ComplexModel& operator=(const ComplexModel&) = delete;
    };
}

