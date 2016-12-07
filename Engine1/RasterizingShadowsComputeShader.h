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
    class BlockMesh;
    class Light;

    class RasterizingShadowsComputeShader : public ComputeShader
    {

        public:

        RasterizingShadowsComputeShader();
        virtual ~RasterizingShadowsComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        void setParameters(
            ID3D11DeviceContext& deviceContext,
            const float3& cameraPos,
            const Light& light,
            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& surfacePositionTexture,
            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& surfaceNormalTexture,
            const int outputTextureWidth, const int outputTextureHeight );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float2  outputTextureSize;
            float2  pad1;
            float3  lightPosition;
            float   pad2;
            float   lightConeMinDot;
            float3  pad3;
            float3  lightDirection;
            float   pad4;
            float   lightEmitterRadius;
            float3  pad5;
            float44 shadowMapViewMatrix;
            float44 shadowMapProjectionMatrix;
            float3  cameraPosition;
            float   pad6;
        };

        Microsoft::WRL::ComPtr< ID3D11SamplerState > m_pointSamplerState;
        Microsoft::WRL::ComPtr< ID3D11SamplerState > m_linearSamplerState;

        // Copying is not allowed.
        RasterizingShadowsComputeShader( const RasterizingShadowsComputeShader& ) = delete;
        RasterizingShadowsComputeShader& operator=( const RasterizingShadowsComputeShader& ) = delete;
    };
}

