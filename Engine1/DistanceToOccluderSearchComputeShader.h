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

    class DistanceToOccluderSearchComputeShader : public ComputeShader
    {
        public:

        DistanceToOccluderSearchComputeShader();
        virtual ~DistanceToOccluderSearchComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( 
            ID3D11DeviceContext3& deviceContext, 
            const float3& cameraPos,
            const float positionThreshold,
            const float normalThreshold,
            const float searchRadiusInShadow,
            const float searchStepInShadow,
            const float searchRadiusInLight,
            const float searchStepInLight,
            const int searchMipmapLevel,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const int2 outputImageDimensions,
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
            float2 inputTextureSize;
            float2 pad6;
            float2 outputTextureSize;
            float2 pad7;
            float  positionThreshold;
            float3 pad8;
            float  normalThreshold;
            float3 pad9;
            float  searchRadiusInShadow;
            float3 pad10;
            float  searchStepInShadow; // In pixels - distance between neighbor samples.
            float3 pad11;
            float  searchRadiusInLight;
            float3 pad12;
            float  searchStepInLight; // In pixels - distance between neighbor samples.
            float3 pad13;
            float  positionSampleMipmapLevel; // Can be used to improve performance by sampling higher level mipmaps.
            float  pad14;
            float  normalSampleMipmapLevel;
            float  pad15;
        };

        // Copying is not allowed.
        DistanceToOccluderSearchComputeShader( const DistanceToOccluderSearchComputeShader& ) = delete;
        DistanceToOccluderSearchComputeShader& operator=( const DistanceToOccluderSearchComputeShader& ) = delete;
    };
}





