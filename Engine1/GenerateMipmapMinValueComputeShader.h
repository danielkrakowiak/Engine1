#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class GenerateMipmapMinValueComputeShader : public ComputeShader
    {

        public:

        GenerateMipmapMinValueComputeShader();
        virtual ~GenerateMipmapMinValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            RenderTargetTexture2D< float >& texture,
                            const int srcMipLevel );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        GenerateMipmapMinValueComputeShader( const GenerateMipmapMinValueComputeShader& ) = delete;
        GenerateMipmapMinValueComputeShader& operator=( const GenerateMipmapMinValueComputeShader& ) = delete;
    };
}



