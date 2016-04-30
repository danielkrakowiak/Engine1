#pragma once

#include "FragmentShader.h"

#include "TTexture2D.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class SkeletonModelFragmentShader : public FragmentShader
    {
        public:
        SkeletonModelFragmentShader();
        virtual ~SkeletonModelFragmentShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

        // Copying is not allowed.
        SkeletonModelFragmentShader( const SkeletonModelFragmentShader& ) = delete;
        SkeletonModelFragmentShader& operator=(const SkeletonModelFragmentShader&) = delete;
    };
}