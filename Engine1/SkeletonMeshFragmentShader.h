#pragma once
#include "FragmentShader.h"

namespace Engine1
{
    class SkeletonMeshFragmentShader : public FragmentShader
    {

        public:

        SkeletonMeshFragmentShader();
        virtual ~SkeletonMeshFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        private:

        // Copying is not allowed.
        SkeletonMeshFragmentShader( const SkeletonMeshFragmentShader& ) = delete;
        SkeletonMeshFragmentShader& operator=(const SkeletonMeshFragmentShader&) = delete;
    };
}

