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

    class CombiningFragmentShader : public FragmentShader
    {
        public:

        CombiningFragmentShader();
        virtual ~CombiningFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
                            const float normalThreshold,
                            const float positionThreshold,
                            const float3 cameraPosition,
                            const int contributionTextureFilledWidth, const int contributionTextureFilledHeight,
                            const int srcTextureFilledWidth, const int srcTextureFilledHeight );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float  normalThreshold;
            float3 pad1;
            float3 cameraPosition;
            float  pad2;
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
            float  roughnessMul;
            float3 pad9;
        };

        int m_resourceCount;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStatePointFilter;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        CombiningFragmentShader( const CombiningFragmentShader& ) = delete;
        CombiningFragmentShader& operator=(const CombiningFragmentShader&) = delete;
    };
}

