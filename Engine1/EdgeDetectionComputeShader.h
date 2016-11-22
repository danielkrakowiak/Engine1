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
    class EdgeDetectionComputeShader : public ComputeShader
    {

        public:

        EdgeDetectionComputeShader();
        virtual ~EdgeDetectionComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& positionTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& normalTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        /*__declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
        };*/

        // Copying is not allowed.
        EdgeDetectionComputeShader( const EdgeDetectionComputeShader& )          = delete;
        EdgeDetectionComputeShader& operator=(const EdgeDetectionComputeShader&) = delete;
    };
}

