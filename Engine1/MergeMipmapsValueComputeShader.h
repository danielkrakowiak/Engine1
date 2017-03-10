#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class MergeMipmapsValueComputeShader : public ComputeShader
    {

        public:

        MergeMipmapsValueComputeShader();
        virtual ~MergeMipmapsValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const int2 outputTextureSize,
                            Texture2DSpecBind< TexBind::ShaderResource, float4 >& inputTexture,
                            int firstMipmapLevel,
                            int lastMipmapLevel );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float2 outputTextureSize;
            float2 pad1;
            float  firstMipmapLevel;
            float3 pad2;
            float  lastMipmapLevel;
            float3 pad3;
        };

        // Copying is not allowed.
        MergeMipmapsValueComputeShader( const MergeMipmapsValueComputeShader& ) = delete;
        MergeMipmapsValueComputeShader& operator=( const MergeMipmapsValueComputeShader& ) = delete;
    };
}





