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
    class EdgeDetectionComputeShader;
    class EdgeDistanceComputeShader;
    class Light;
    class Camera;

    class EdgeDetectionRenderer
    {
        public:

        EdgeDetectionRenderer( DX11RendererCore& rendererCore );
        ~EdgeDetectionRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void performEdgeDetection( const std::shared_ptr< Texture2D< float4 > > positionTexture,
                                   const std::shared_ptr< Texture2D< float4 > > normalTexture );

        std::shared_ptr< RenderTargetTexture2D< unsigned char > > getValueRenderTarget();

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        std::shared_ptr< RenderTargetTexture2D< unsigned char > > m_valueRenderTarget0;
        std::shared_ptr< RenderTargetTexture2D< unsigned char > > m_valueRenderTarget1;

        void swapSrcDestRenderTargets();

        std::shared_ptr< RenderTargetTexture2D< unsigned char > > m_valueRenderTargetSrc;
        std::shared_ptr< RenderTargetTexture2D< unsigned char > > m_valueRenderTargetDest;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device3& device );

        // Shaders.
        std::shared_ptr< EdgeDetectionComputeShader > m_edgeDetectionComputeShader;
        std::shared_ptr< EdgeDistanceComputeShader >  m_edgeDistanceComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        EdgeDetectionRenderer( const EdgeDetectionRenderer& )           = delete;
        EdgeDetectionRenderer& operator=(const EdgeDetectionRenderer& ) = delete;
    };
}

