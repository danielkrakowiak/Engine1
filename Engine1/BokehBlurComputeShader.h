#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class BokehBlurComputeShader : public ComputeShader
    {
        public:

        BokehBlurComputeShader() = default;
        virtual ~BokehBlurComputeShader() = default;

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& depthTexture );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr< ID3D11SamplerState > m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float2 textureSize;
            float2 pad1;
            float  apertureDiameter;
            float3 pad2;
            float  cameraFocusDist;
            float3 pad3;
            float  focalLength;
            float3 pad4;
            float  coCMul;
            float3 pad5;
            float  maxCoC;
            float3 pad6;
            float  relativeDepthThreshold;
            float3 pad7;
        };

        // Copying is not allowed.
        BokehBlurComputeShader( const BokehBlurComputeShader& ) = delete;
        BokehBlurComputeShader& operator=( const BokehBlurComputeShader& ) = delete;
    };
}


