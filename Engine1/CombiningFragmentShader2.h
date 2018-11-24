#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class CombiningFragmentShader2 : public FragmentShader
    {
        public:

        CombiningFragmentShader2();
        virtual ~CombiningFragmentShader2();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const std::shared_ptr< Texture2D< float4 > > srcTexture,
                            const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture, 
                            const std::shared_ptr< Texture2D< float4 > > previousHitNormalTexture,
                            const std::shared_ptr< Texture2D< float4 > > previousHitPositionTexture,
                            const std::shared_ptr< Texture2D< float > >  previousHitDistanceTexture,
                            const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,
                            const std::shared_ptr< Texture2D< float4 > > previousRayOriginTexture,
                            const float normalThreshold,
                            const float positionThreshold,
                            const int contributionTextureFilledWidth, const int contributionTextureFilledHeight,
                            const int srcTextureFilledWidth, const int srcTextureFilledHeight );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float  normalThreshold;
            float3 pad1;
            float2 imageSize;
            float2 pad2;
            float2 contributionTextureFillSize;
            float2 pad3;
            float2 srcTextureFillSize;
            float2 pad4;
            float  positionDiffMul;
            float3 pad5;
            float  normalDiffMul;
            float3 pad6;
            float  positionNormalThreshold;
            float3 pad7;
            float  roughnessMul;
            float3 pad8;
            float  elongationMul;
            float3 pad10;
            float  radialBlurEnabled; // 1.0 - enabled, 0.0 - disabled.
            float3 pad11;
            float  reflectionSamplingQualityInv; // 0 - highest quality, 1 - lowest quality.
            float3 pad12;
            float  debugHitDistPower;
            float3 pad13;
        };

        int m_resourceCount;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStatePointFilter;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        CombiningFragmentShader2( const CombiningFragmentShader2& ) = delete;
        CombiningFragmentShader2& operator=(const CombiningFragmentShader2&) = delete;
    };
}

