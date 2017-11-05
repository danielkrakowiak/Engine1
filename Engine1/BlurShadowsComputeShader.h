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

    class BlurShadowsComputeShader : public ComputeShader
    {

        public:

        BlurShadowsComputeShader();
        virtual ~BlurShadowsComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( 
            ID3D11DeviceContext3& deviceContext, const float3& cameraPos,
            const float positionThreshold,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > shadowTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > finalDistanceToOccluder,
            const Light& light 
        );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float3 lightPosition;
            float  pad2;
            float  lightConeMinDot;
            float3 pad3;
            float3 lightDirection;
            float  pad4;
            float  lightEmitterRadius;
            float3 pad5;
            float2 outputTextureSize;
            float2 pad6;
            float  positionThreshold;
            float3 pad7;
        };

        // Copying is not allowed.
        BlurShadowsComputeShader( const BlurShadowsComputeShader& ) = delete;
        BlurShadowsComputeShader& operator=( const BlurShadowsComputeShader& ) = delete;
    };
}



