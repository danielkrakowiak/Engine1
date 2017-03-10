#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class BlurValueComputeShader : public ComputeShader
    {
        public:

        BlurValueComputeShader();
        virtual ~BlurValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::ShaderResource, float4 > texture,
                            const int textureMipmapLevel,
                            const int2 outputTextureSize );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float  blurRadius;
            float3 pad1;
            float2 outputTextureSize;
            float2 pad2;
            float2 inputTextureSize;
            float2 pad3;
        };

        // Copying is not allowed.
        BlurValueComputeShader( const BlurValueComputeShader& ) = delete;
        BlurValueComputeShader& operator=( const BlurValueComputeShader& ) = delete;
    };
}





