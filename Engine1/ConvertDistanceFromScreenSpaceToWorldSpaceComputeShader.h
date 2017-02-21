#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader : public ComputeShader
    {

        public:

        ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader();
        virtual ~ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const float3& cameraPos,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                            const int2 outputTextureSize );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float2 outputTextureSize;
            float2 pad2;
        };

        // Copying is not allowed.
        ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader( const ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader& ) = delete;
        ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader& operator=( const ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader& ) = delete;
    };
}



