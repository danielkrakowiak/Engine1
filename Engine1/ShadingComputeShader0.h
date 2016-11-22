#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Light;

    class ShadingComputeShader0 : public ComputeShader
    {

        public:

        ShadingComputeShader0();
        virtual ~ShadingComputeShader0();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > emissiveTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerState;

        // Copying is not allowed.
        ShadingComputeShader0( const ShadingComputeShader0& )          = delete;
        ShadingComputeShader0& operator=(const ShadingComputeShader0&) = delete;
    };
}

