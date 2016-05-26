#pragma once

#include <wrl.h>
#include <memory>

#include "TTexture2D.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class EdgeDetectionComputeShader;
    class EdgeDistanceComputeShader;
    class Light;
    class Camera;

    class EdgeDetectionRenderer
    {
        public:

        EdgeDetectionRenderer( Direct3DRendererCore& rendererCore );
        ~EdgeDetectionRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void performEdgeDetection( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getValueRenderTarget();

        private:

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > valueRenderTarget0;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > valueRenderTarget1;

        void swapSrcDestRenderTargets();

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > valueRenderTargetSrc;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > valueRenderTargetDest;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< EdgeDetectionComputeShader > edgeDetectionComputeShader;
        std::shared_ptr< EdgeDistanceComputeShader >  edgeDistanceComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        EdgeDetectionRenderer( const EdgeDetectionRenderer& )           = delete;
        EdgeDetectionRenderer& operator=(const EdgeDetectionRenderer& ) = delete;
    };
}

