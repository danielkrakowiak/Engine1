#pragma once

#include <wrl.h>
#include <memory>

#include "TTexture2D.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class TextureRescaleComputeShader;

    class TextureRescaleRenderer
    {
        public:

        TextureRescaleRenderer( Direct3DRendererCore& rendererCore );
        ~TextureRescaleRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void rescaleTexture( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                             const unsigned char srcMipmapLevel,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 > > destTexture );

        private:

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        bool initialized;

        // Shaders.
        std::shared_ptr< TextureRescaleComputeShader > textureRescaleComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        TextureRescaleRenderer( const TextureRescaleRenderer& )           = delete;
        TextureRescaleRenderer& operator=(const TextureRescaleRenderer& ) = delete;
    };
}

