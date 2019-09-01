#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"
#include "BokehBlurComputeShader.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class DX11RendererCore;

    class BokehBlurRenderer
    {
        public:

        BokehBlurRenderer( DX11RendererCore& rendererCore );
        ~BokehBlurRenderer() = default;

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void bokehBlur( 
            std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture,
            const Texture2D< float4 >& srcTexture,
            const Texture2D< uchar4 >& depthTexture
        );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        std::shared_ptr< BokehBlurComputeShader > m_shader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        BokehBlurRenderer( const BokehBlurRenderer& )           = delete;
        BokehBlurRenderer& operator=(const BokehBlurRenderer& ) = delete;
    };
}


