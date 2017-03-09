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
    class ExtractBrightPixelsComputeShader : public ComputeShader
    {

        public:

        ExtractBrightPixelsComputeShader();
        virtual ~ExtractBrightPixelsComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& colorTexture,
                            const float minBrightness );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float  minBrightness;
            float3 pad1;
        };

        // Copying is not allowed.
        ExtractBrightPixelsComputeShader( const ExtractBrightPixelsComputeShader& )          = delete;
        ExtractBrightPixelsComputeShader& operator=(const ExtractBrightPixelsComputeShader&) = delete;
    };
}



