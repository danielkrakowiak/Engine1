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

        static float s_positionDiffMul;
        static float s_normalDiffMul;
        static float s_positionNormalThreshold;

        CombiningFragmentShader2();
        virtual ~CombiningFragmentShader2();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitNormalTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitPositionTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  previousHitDistanceTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousRayOriginTexture,
                            const float normalThreshold,
                            const float positionThreshold,
                            const int contributionTextureFilledWidth, const int contributionTextureFilledHeight,
                            const int srcTextureFilledWidth, const int srcTextureFilledHeight );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float  normalThreshold;
            float3 pad1;
            float  positionThresholdSquare;
            float3 pad2;
            float2 imageSize;
            float2 pad3;
            float2 contributionTextureFillSize;
            float2 pad4;
            float2 srcTextureFillSize;
            float2 pad5;
            float  positionDiffMul;
            float3 pad6;
            float  normalDiffMul;
            float3 pad7;
            float  positionNormalThreshold;
            float3 pad8;
        };

        int m_resourceCount;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStatePointFilter;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        CombiningFragmentShader2( const CombiningFragmentShader2& ) = delete;
        CombiningFragmentShader2& operator=(const CombiningFragmentShader2&) = delete;
    };
}

