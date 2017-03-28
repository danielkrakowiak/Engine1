#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
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

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void blurShadows( const Camera& camera,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > hardShadowTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > softShadowTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > finalDistanceToOccluder,
                             const Light& light );

        void blurShadowsHorzVert( const Camera& camera,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > hardShadowTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > softShadowTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > finalDistanceToOccluder,
                          const Light& light );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > getShadowTexture();

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > m_shadowRenderTarget;

        // Extra render target used when 2-pass separable blur is run.
        // It stores horizontally blurred illumination before the vertical pass.
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > m_shadowTemporaryRenderTarget;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsComputeShader;
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsHorizontalComputeShader;
        std::shared_ptr< BlurShadowsComputeShader >  m_blurShadowsVerticalComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        BlurShadowsRenderer( const BlurShadowsRenderer& ) = delete;
        BlurShadowsRenderer& operator=( const BlurShadowsRenderer& ) = delete;
    };
}



