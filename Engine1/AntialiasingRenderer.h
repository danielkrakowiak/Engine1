#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

#include "uchar4.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class ComputeShader;
    class AntialiasingComputeShader;

    class AntialiasingRenderer
    {
        public:

        AntialiasingRenderer( Direct3DRendererCore& rendererCore );
        ~AntialiasingRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        // Calculates luminance from RGB channels and saves it into Alpha channel.
        void calculateLuminance( std::shared_ptr< RenderTargetTexture2D< uchar4 > > texture );

        void performAntialiasing( std::shared_ptr< Texture2D< uchar4 > > srcTexture,
                                  std::shared_ptr< RenderTargetTexture2D< uchar4 > > dstTexture );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< ComputeShader >              m_luminanceComputeShader;
        std::shared_ptr< AntialiasingComputeShader >  m_antialiasingComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        AntialiasingRenderer( const AntialiasingRenderer& ) = delete;
        AntialiasingRenderer& operator=( const AntialiasingRenderer& ) = delete;
    };
}







