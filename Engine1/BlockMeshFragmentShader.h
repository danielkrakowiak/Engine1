#pragma once

#include "FragmentShader.h"

namespace Engine1
{
    class BlockMeshFragmentShader : public FragmentShader
    {
        public:

        BlockMeshFragmentShader();
        virtual ~BlockMeshFragmentShader();

        private:

        // Copying is not allowed.
        BlockMeshFragmentShader( const BlockMeshFragmentShader& ) = delete;
        BlockMeshFragmentShader& operator=(const BlockMeshFragmentShader&) = delete;
    };
}

