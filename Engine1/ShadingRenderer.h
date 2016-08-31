#pragma once

#include <wrl.h>
#include <memory>

#include "TTexture2D.h"

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
    class ShadingComputeShader;
    class ShadingComputeShader2;
    class Light;
    class Camera;

    class ShadingRenderer
    {
        public:

        ShadingRenderer( Direct3DRendererCore& rendererCore );
        ~ShadingRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void performShading( const Camera& camera,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > emissiveTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                             const std::vector< std::shared_ptr< Light > >& lights );

        void performShading( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitEmissiveTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                             const std::vector< std::shared_ptr< Light > >& lights );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > getColorRenderTarget();

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_colorRenderTarget;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< ShadingComputeShader >  m_shadingComputeShader;
        std::shared_ptr< ShadingComputeShader2 > m_shadingComputeShader2;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        ShadingRenderer( const ShadingRenderer& )           = delete;
        ShadingRenderer& operator=(const ShadingRenderer& ) = delete;
    };
}

