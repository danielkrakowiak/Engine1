#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class SkeletonModelFragmentShader : public FragmentShader
    {
        public:
        SkeletonModelFragmentShader();
        virtual ~SkeletonModelFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, 
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture,
                            const float4& extraEmissive = float4::ZERO );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float4 extraEmissive;
        };

        // Copying is not allowed.
        SkeletonModelFragmentShader( const SkeletonModelFragmentShader& ) = delete;
        SkeletonModelFragmentShader& operator=(const SkeletonModelFragmentShader&) = delete;
    };
}