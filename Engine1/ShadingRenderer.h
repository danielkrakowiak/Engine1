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
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > > normalTexture,
                             const std::vector< std::shared_ptr< Light > >& lights );

        std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 > > getColorRenderTarget();

        private:

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > colorRenderTarget;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< ShadingComputeShader > shadingComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        ShadingRenderer( const ShadingRenderer& )           = delete;
        ShadingRenderer& operator=(const ShadingRenderer& ) = delete;
    };
}

