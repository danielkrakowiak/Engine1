#pragma once
#include "FragmentShader.h"

namespace Engine1
{
    class SkeletonMeshFragmentShader : public FragmentShader
    {

        public:

        SkeletonMeshFragmentShader();
        virtual ~SkeletonMeshFragmentShader();

        void compileFromFile( std::string path, ID3D11Device& device );

        private:

        // Copying is not allowed.
        SkeletonMeshFragmentShader( const SkeletonMeshFragmentShader& ) = delete;
        SkeletonMeshFragmentShader& operator=(const SkeletonMeshFragmentShader&) = delete;
    };
}

