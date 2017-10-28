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

        // Threshold used to decide whether a sample can be used in search operation.
        // Samples which are too "different" from the search center pixel are rejected,
        // because they are parts of completely disconnected objects 
        // (example: objects near and far from the camera).
        static float s_positionThreshold;

        DistanceToOccluderSearchComputeShader();
        virtual ~DistanceToOccluderSearchComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( 
            ID3D11DeviceContext3& deviceContext, 
            const float3& cameraPos,
            const float searchRadius,
            const float searchStep,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
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
            float  searchRadius;
            float3 pad8;
            float  searchStep; // In pixels - distance between neighbor samples.
            float3 pad9;
        };

        // Copying is not allowed.
        DistanceToOccluderSearchComputeShader( const DistanceToOccluderSearchComputeShader& ) = delete;
        DistanceToOccluderSearchComputeShader& operator=( const DistanceToOccluderSearchComputeShader& ) = delete;
    };
}





