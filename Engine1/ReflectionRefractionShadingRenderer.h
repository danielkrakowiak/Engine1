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
    class ReflectionShadingComputeShader;
    class ReflectionShadingComputeShader2;
    class RefractionShadingComputeShader;
    class RefractionShadingComputeShader2;
    class Light;
    class Camera;

    class ReflectionRefractionShadingRenderer
    {
        public:

        ReflectionRefractionShadingRenderer( Direct3DRendererCore& rendererCore );
        ~ReflectionRefractionShadingRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void performFirstReflectionShading( 
            const Camera& camera,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > contributionRoughnessRenderTarget
        );

        void performReflectionShading( 
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > prevContributionRoughnessRenderTarget,
            const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > contributionRoughnessRenderTarget
        );

        void performFirstRefractionShading( 
            const Camera& camera,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > contributionRoughnessRenderTarget
        );

        void performRefractionShading( 
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > prevContributionRoughnessRenderTarget,
            const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > contributionRoughnessRenderTarget
        );

        private:

        static const int maxRenderTargetCount;

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;
        
        // Shaders.
        std::shared_ptr< ReflectionShadingComputeShader >   m_reflectionShadingComputeShader;
        std::shared_ptr< ReflectionShadingComputeShader2 >  m_reflectionShadingComputeShader2;
        std::shared_ptr< RefractionShadingComputeShader >   m_refractionShadingComputeShader;
        std::shared_ptr< RefractionShadingComputeShader2 >  m_refractionShadingComputeShader2;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        ReflectionRefractionShadingRenderer( const ReflectionRefractionShadingRenderer& )           = delete;
        ReflectionRefractionShadingRenderer& operator=(const ReflectionRefractionShadingRenderer& ) = delete;
    };
}

