#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class BlockModelFragmentShader : public FragmentShader
    {
        public:
        BlockModelFragmentShader();
        virtual ~BlockModelFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        void setParameters( ID3D11DeviceContext& deviceContext, 
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture, const float alphaMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture, const float3& emissiveMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture, const float3& albedoMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture, const float3& normalMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture, const float metalnessMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture, const float roughnessMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture, const float indexOfRefractionMul,
                            const float4& extraEmissive = float4::ZERO );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float  alphaMul;
            float3 pad1;
            float3 emissiveMul;
            float  pad2;
            float3 albedoMul;
            float  pad3;
            float3 normalMul;
            float  pad4;
            float  metalnessMul;
            float3 pad5;
            float  roughnessMul;
            float3 pad6;
            float  indexOfRefractionMul;
            float3 pad7;
            float4 extraEmissive;
        };

        // Copying is not allowed.
        BlockModelFragmentShader( const BlockModelFragmentShader& ) = delete;
        BlockModelFragmentShader& operator=(const BlockModelFragmentShader&) = delete;
    };
}