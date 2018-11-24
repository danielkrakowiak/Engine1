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

    class ShadingComputeShader2 : public ComputeShader
    {

        public:

        ShadingComputeShader2();
        virtual ~ShadingComputeShader2();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, 
                            const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
                            const std::shared_ptr< Texture2D< float4 > > rayHitPositionTexture,
                            const std::shared_ptr< Texture2D< uchar4 > > rayHitAlbedoTexture, 
                            const std::shared_ptr< Texture2D< unsigned char > > rayHitMetalnessTexture, 
                            const std::shared_ptr< Texture2D< unsigned char > > rayHitRoughnessTexture, 
                            const std::shared_ptr< Texture2D< float4 > > rayHitNormalTexture,
						    const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
							const Light& light );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerState;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float4 lightPosition;
            float4 lightColor;
            float  lightLinearAttenuationFactor;
            float3 pad1;
            float  lightQuadraticAttenuationFactor;
            float3 pad2;
            float2 outputTextureSize;
            float2 pad3;
        };

        // Copying is not allowed.
        ShadingComputeShader2( const ShadingComputeShader2& )          = delete;
        ShadingComputeShader2& operator=(const ShadingComputeShader2&) = delete;
    };
}

