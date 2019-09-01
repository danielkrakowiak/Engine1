#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2DTypes.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class DX11RendererCore;
    class BlockActor;
    class RasterizingShadowsComputeShader;
    class Light;

    class RasterizeShadowRenderer
    {
        public:

        RasterizeShadowRenderer( DX11RendererCore& rendererCore );
        ~RasterizeShadowRenderer();

        void initialize(
            int imageWidth,
            int imageHeight,
            Microsoft::WRL::ComPtr< ID3D11Device3 > device,
            Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext
            );

        void performShadowMapping(
            const float3& cameraPos,
            const std::shared_ptr< Light > light,
            const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
            const std::shared_ptr< Texture2D< float4 > > surfaceNormalTexture
        );

        std::shared_ptr< RenderTargetTexture2D< unsigned char > > getShadowTexture();
        std::shared_ptr< RenderTargetTexture2D< float > >         getDistanceToOccluder();

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        std::shared_ptr< RenderTargetTexture2D< unsigned char > > m_shadowTexture;
        std::shared_ptr< RenderTargetTexture2D< float > >         m_distanceToOccluderTexture;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device3& device );

        // Shaders.
        std::shared_ptr< RasterizingShadowsComputeShader > m_rasterizeShadowsComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        RasterizeShadowRenderer( const RasterizeShadowRenderer& ) = delete;
        RasterizeShadowRenderer& operator=( const RasterizeShadowRenderer& ) = delete;
    };
}

