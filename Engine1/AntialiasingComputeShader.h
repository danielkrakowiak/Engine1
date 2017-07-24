#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class AntialiasingComputeShader : public ComputeShader
    {

        public:

        AntialiasingComputeShader();
        virtual ~AntialiasingComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( 
            ID3D11DeviceContext3& deviceContext,
            Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& srcTexture,
            const float fxaaQualitySubpix,
            const float fxaaQualityEdgeThreshold,
            const float fxaaQualityEdgeThresholdMin 
        );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float2 pixelSizeInTexcoords;
            float2 pad1;
            float  fxaaQualitySubpix;
            float3 pad2;
            float  fxaaQualityEdgeThreshold;
            float3 pad3;
            float  fxaaQualityEdgeThresholdMin;
            float3 pad4;
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        AntialiasingComputeShader( const AntialiasingComputeShader& ) = delete;
        AntialiasingComputeShader& operator=( const AntialiasingComputeShader& ) = delete;
    };
}







