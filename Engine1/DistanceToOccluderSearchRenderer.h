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
    class DX11RendererCore;
    class DistanceToOccluderSearchComputeShader;
    class Light;
    class Camera;

    class DistanceToOccluderSearchRenderer
    {
        public:

        DistanceToOccluderSearchRenderer( DX11RendererCore& rendererCore );
        ~DistanceToOccluderSearchRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void performDistanceToOccluderSearch( 
            const Camera& camera,
            const float positionThreshold,
            const float normalThreshold,
            const float searchRadiusInShadow,
            const float searchStepInShadow,
            const float searchRadiusInLight,
            const float searchStepInLight,
            const int searchMipmapLevel,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< float > > distanceToOccluder,
            const std::shared_ptr< RenderTargetTexture2D< float > > finalDistanceToOccluderRenderTarget,
            const Light& light  
        );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        // Shaders.
        std::shared_ptr< DistanceToOccluderSearchComputeShader >  m_distanceToOccluderSearchComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        DistanceToOccluderSearchRenderer( const DistanceToOccluderSearchRenderer& ) = delete;
        DistanceToOccluderSearchRenderer& operator=( const DistanceToOccluderSearchRenderer& ) = delete;
    };
}





