#pragma once

#include "ComputeShader.h"

#include <string>

#include "float2.h"
#include "float3.h"
#include "float4.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class GenerateRaysComputeShader : public ComputeShader
    {

        public:

        GenerateRaysComputeShader();
        virtual ~GenerateRaysComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float3 cameraPos, const float3 viewportCenter, const float3 viewportUp, const float3 viewportRight, const float2 viewportSize );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float3 viewportCenter;
            float  pad2;
            float3 viewportUp;
            float  pad3;
            float3 viewportRight;
            float  pad4;
            float2 viewportSizeHalf;
            float2 pad5;
        };

        // Copying is not allowed.
        GenerateRaysComputeShader( const GenerateRaysComputeShader& ) = delete;
        GenerateRaysComputeShader& operator=(const GenerateRaysComputeShader&) = delete;
    };
}

