#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class Direct3DRendererCore;
    class TextureRescaleComputeShader;

    class TextureRescaleRenderer
    {
        public:

        TextureRescaleRenderer( Direct3DRendererCore& rendererCore );
        ~TextureRescaleRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void rescaleTexture( const std::shared_ptr< Texture2D< float4 > > srcTexture,
                             const unsigned char srcMipmapLevel,
                             const std::shared_ptr< Texture2D< float4 > > destTexture,
                             const unsigned char destMipmapLevel );

        private:

        Direct3DRendererCore& m_rendererCore;

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

