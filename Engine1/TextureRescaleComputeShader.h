#pragma once

#include "ComputeShader.h"

#include <string>

#include "uint2.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class TextureRescaleComputeShader : public ComputeShader
    {

        public:

        TextureRescaleComputeShader();
        virtual ~TextureRescaleComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& srcTexture,
                            const int destTextureWidth, const int destTextureHeight,
                            const unsigned char srcMipmapLevel );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr< ID3D11SamplerState > m_samplerState;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float2 destTexturePixelSize;
            float2 pad1;
            unsigned int srcMipmapLevel;
            float3 pad2;
        };

        // Copying is not allowed.
        TextureRescaleComputeShader( const TextureRescaleComputeShader& )          = delete;
        TextureRescaleComputeShader& operator=(const TextureRescaleComputeShader&) = delete;
    };
}

