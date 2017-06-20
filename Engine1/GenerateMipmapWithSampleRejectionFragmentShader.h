#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class GenerateMipmapWithSampleRejectionFragmentShader : public FragmentShader
    {
        public:

        GenerateMipmapWithSampleRejectionFragmentShader();
        virtual ~GenerateMipmapWithSampleRejectionFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float >& texture,
                            const int srcMipLevel, const float maxAcceptableValue );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float2 srcPixelSizeInTexcoords;
            float2 pad1;
            float  srcMipmapLevel;
            float3 pad2;
            float  maxAcceptableValue;
            float3 pad3;
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        GenerateMipmapWithSampleRejectionFragmentShader( const GenerateMipmapWithSampleRejectionFragmentShader& ) = delete;
        GenerateMipmapWithSampleRejectionFragmentShader& operator=( const GenerateMipmapWithSampleRejectionFragmentShader& ) = delete;
    };
}



