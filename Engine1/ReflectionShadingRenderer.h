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
    class ReflectionShadingComputeShader;
    class Light;
    class Camera;

    class ReflectionShadingRenderer
    {
        public:

        ReflectionShadingRenderer( Direct3DRendererCore& rendererCore );
        ~ReflectionShadingRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void performShading( const int level, const Camera& camera,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > getReflectionTermTarget( int level );

        private:

        static const int maxRenderTargetCount;

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;
        
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > > reflectionTermRenderTargets;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< ReflectionShadingComputeShader >  shadingComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        ReflectionShadingRenderer( const ReflectionShadingRenderer& )           = delete;
        ReflectionShadingRenderer& operator=(const ReflectionShadingRenderer& ) = delete;
    };
}

