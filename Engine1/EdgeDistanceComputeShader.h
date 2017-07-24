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
    class EdgeDistanceComputeShader : public ComputeShader
    {

        public:

        EdgeDistanceComputeShader();
        virtual ~EdgeDistanceComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& distToEdgeTexture,
                            const unsigned char passIndex );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            unsigned int passIndex;
            float3       pad1;
        };

        // Copying is not allowed.
        EdgeDistanceComputeShader( const EdgeDistanceComputeShader& )          = delete;
        EdgeDistanceComputeShader& operator=(const EdgeDistanceComputeShader&) = delete;
    };
}

