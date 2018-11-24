#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class BlurShadowPatternComputeShader;
    class Light;
    class Camera;

    class BlurShadowPatternRenderer
    {
        public:

        BlurShadowPatternRenderer( Direct3DRendererCore& rendererCore );
        ~BlurShadowPatternRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void blurShadowPattern( 
            const Camera& camera,
            const float positionThreshold,
            const float normalThreshold,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
            const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowRenderTarget,
            const Light& light 
        );

        // shadowTemporaryRenderTarget is an extra render target used when 2-pass separable blur is run.
        // It stores horizontally blurred image before the vertical pass.
        void blurShadowPatternHorzVert( 
            const Camera& camera,
            const float positionThreshold,
            const float normalThreshold,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
            const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowRenderTarget,
            const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowTemporaryRenderTarget,
            const Light& light 
        );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        // Shaders.
        std::shared_ptr< BlurShadowPatternComputeShader >  m_blurShadowPatternComputeShader;
        std::shared_ptr< BlurShadowPatternComputeShader >  m_blurShadowPatternHorizontalComputeShader;
        std::shared_ptr< BlurShadowPatternComputeShader >  m_blurShadowPatternVerticalComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        BlurShadowPatternRenderer( const BlurShadowPatternRenderer& ) = delete;
        BlurShadowPatternRenderer& operator=( const BlurShadowPatternRenderer& ) = delete;
    };
}





