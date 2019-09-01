#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

#include "float4.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class DX11RendererCore;
    class ExtractBrightPixelsComputeShader;

    class ExtractBrightPixelsRenderer
    {
        public:

        ExtractBrightPixelsRenderer( DX11RendererCore& rendererCore );
        ~ExtractBrightPixelsRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void extractBrightPixels( std::shared_ptr< Texture2D< float4 > > colorTexture,
                                  std::shared_ptr< RenderTargetTexture2D< float4 > > destinationTexture,
                                  const float minBrightness );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< ExtractBrightPixelsComputeShader >  m_extractBrightPixelsComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        ExtractBrightPixelsRenderer( const ExtractBrightPixelsRenderer& ) = delete;
        ExtractBrightPixelsRenderer& operator=( const ExtractBrightPixelsRenderer& ) = delete;
    };
}





