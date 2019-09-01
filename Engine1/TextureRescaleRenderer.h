#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class DX11RendererCore;
    class TextureRescaleComputeShader;

    class TextureRescaleRenderer
    {
        public:

        TextureRescaleRenderer( DX11RendererCore& rendererCore );
        ~TextureRescaleRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void rescaleTexture( const std::shared_ptr< Texture2D< float4 > > srcTexture,
                             const unsigned char srcMipmapLevel,
                             const std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture,
                             const unsigned char destMipmapLevel );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< TextureRescaleComputeShader > m_textureRescaleComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        TextureRescaleRenderer( const TextureRescaleRenderer& )           = delete;
        TextureRescaleRenderer& operator=(const TextureRescaleRenderer& ) = delete;
    };
}

