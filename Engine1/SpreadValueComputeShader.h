#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class SpreadValueComputeShader : public ComputeShader
    {

        public:

        SpreadValueComputeShader();
        virtual ~SpreadValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const float skipPixelIfBelowValue,
                            const float minAcceptableValue,
                            const int  totalSpread,
                            const int spreadDistance,
                            const int offset,
                            const int2 textureSize,
                            const float3 cameraPos,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float3 > > positionTexture );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float  skipPixelIfBelowValue;
            float3 pad1;
            float  minAcceptableValue;
            float3 pad2;
            int    spreadDistance;
            float3 pad3;
            int    offset;
            float3 pad4;
            float2 textureSize;
            float2 pad5;
            float3 cameraPos;
            float  pad6;
            float  totalSpread;
            float3 pad7;
        };

        // Copying is not allowed.
        SpreadValueComputeShader( const SpreadValueComputeShader& ) = delete;
        SpreadValueComputeShader& operator=( const SpreadValueComputeShader& ) = delete;
    };
}



