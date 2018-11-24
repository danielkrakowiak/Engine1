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
    class BlurShadowsComputeShader;
    class Light;
    class Camera;

    class BlurShadowsRenderer
    {
        public:

        BlurShadowsRenderer( Direct3DRendererCore& rendererCore );
        ~BlurShadowsRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void blurShadows( 
            const Camera& camera,
            const float positionThreshold,
            const float normalThreshold,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
            const std::shared_ptr< Texture2D< unsigned char > > shadowRenderTarget,
            const Light& light 
        );

        // shadowTemporaryRenderTarget is an extra render target used when 2-pass separable blur is run.
        // It stores horizontally blurred illumination before the vertical pass.
        void blurShadowsHorzVert( 
            const Camera& camera,
            const float positionThreshold,
            const float normalThreshold,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
            const std::shared_ptr< Texture2D< unsigned char > > shadowRenderTarget,
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
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsComputeShader;
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsHorizontalComputeShader;
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsVerticalComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        BlurShadowsRenderer( const BlurShadowsRenderer& ) = delete;
        BlurShadowsRenderer& operator=( const BlurShadowsRenderer& ) = delete;
    };
}



