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

    class HitDistanceSearchComputeShader : public ComputeShader
    {
        public:

        static float s_positionDiffMul;
        static float s_normalDiffMul;
        static float s_positionNormalThreshold;
        static float s_minSampleWeightBasedOnDistance;

        HitDistanceSearchComputeShader();
        virtual ~HitDistanceSearchComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float3& cameraPos, const int2 outputTextureDimensions,
                            const std::shared_ptr< Texture2D< float4 > > positionTexture,
                            const std::shared_ptr< Texture2D< float4 > > normalTexture,
                            const std::shared_ptr< Texture2D< float > > distanceToOccluder );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerState;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float2 outputTextureSize;
            float2 pad2;
            float2 inputTextureSize;
            float2 pad3;
            float  positionThreshold;
            float3 pad4;
            float  positionDiffMul;
            float3 pad5;
            float  normalDiffMul;
            float3 pad6;
            float  positionNormalThreshold;
            float3 pad7;
            float  minSampleWeightBasedOnDistance;
            float3 pad8;
            float  decreaseBlurForSmallValues; // 0 - no, 1 - yes.
            float3 pad9;
            float  maxHitDistForDecreasedBlur;
            float3 pad10;
        };

        // Copying is not allowed.
        HitDistanceSearchComputeShader( const HitDistanceSearchComputeShader& ) = delete;
        HitDistanceSearchComputeShader& operator=( const HitDistanceSearchComputeShader& ) = delete;
    };
}







