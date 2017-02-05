#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class BlockActor;
    class RasterizingShadowsComputeShader;
    class Light;

    class RasterizeShadowRenderer
    {
        public:

        RasterizeShadowRenderer( Direct3DRendererCore& rendererCore );
        ~RasterizeShadowRenderer();

        void initialize(
            int imageWidth,
            int imageHeight,
            Microsoft::WRL::ComPtr< ID3D11Device > device,
            Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext
            );

        void performShadowMapping(
            const float3& cameraPos,
            const std::shared_ptr< Light > light,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > surfaceNormalTexture
        );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > getIlluminationTexture();
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         getMinIlluminationBlurRadiusTexture();
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         getMaxIlluminationBlurRadiusTexture();

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > m_illuminationTexture;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         m_minIlluminationBlurRadiusTexture;

        // Note: This texture is not a good fit here. It's not used by this shader. But I placed it here, because it's sibling of m_minIlluminationBlurRadiusTexture.
        // #TODO: What to do with both of these textures? Store them somewhere else? In "big" Renderer?
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         m_maxIlluminationBlurRadiusTexture;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< RasterizingShadowsComputeShader > m_rasterizeShadowsComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        RasterizeShadowRenderer( const RasterizeShadowRenderer& ) = delete;
        RasterizeShadowRenderer& operator=( const RasterizeShadowRenderer& ) = delete;
    };
}

