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

    class GenerateMipmapMinValueFragmentShader : public FragmentShader
    {
        public:

        GenerateMipmapMinValueFragmentShader();
        virtual ~GenerateMipmapMinValueFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            RenderTargetTexture2D< float >& texture,
                            const int srcMipLevel );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float2 srcPixelSizeInTexcoords;
            float2 pad1;
            float  srcMipmapLevel;
            float3 pad2;
        };

        int m_resourceCount;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        GenerateMipmapMinValueFragmentShader( const GenerateMipmapMinValueFragmentShader& ) = delete;
        GenerateMipmapMinValueFragmentShader& operator=( const GenerateMipmapMinValueFragmentShader& ) = delete;
    };
}



