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
    class Light;

    class ShadingComputeShader : public ComputeShader
    {

        public:

        ShadingComputeShader();
        virtual ~ShadingComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float3& cameraPos,
                            const std::shared_ptr< Texture2D< float4 > > positionTexture,
                            const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
                            const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture, 
                            const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture, 
                            const std::shared_ptr< Texture2D< float4 > > normalTexture,
							const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
							const Light& light );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerState;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float4 lightPosition;
            float4 lightColor;
            float  lightLinearAttenuationFactor;
            float3 pad2;
            float  lightQuadraticAttenuationFactor;
            float3 pad3;
            float2 outputTextureSize;
            float2 pad4;
        };

        // Copying is not allowed.
        ShadingComputeShader( const ShadingComputeShader& )          = delete;
        ShadingComputeShader& operator=(const ShadingComputeShader&) = delete;
    };
}

