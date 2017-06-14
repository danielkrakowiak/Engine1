#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class ToneMappingComputeShader : public ComputeShader
    {

        public:

        ToneMappingComputeShader();
        virtual ~ToneMappingComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::ShaderResource, float4 >& srcTexture,
                            const float exposure );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float  exposure;
            float3 pad1;
        };

        // Copying is not allowed.
        ToneMappingComputeShader( const ToneMappingComputeShader& ) = delete;
        ToneMappingComputeShader& operator=( const ToneMappingComputeShader& ) = delete;
    };
}





