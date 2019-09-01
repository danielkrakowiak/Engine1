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
    class HitDistanceSearchComputeShader;
    class Camera;

    class HitDistanceSearchRenderer
    {
        public:

        HitDistanceSearchRenderer( DX11RendererCore& rendererCore );
        ~HitDistanceSearchRenderer();

        void initialize( 
            Microsoft::WRL::ComPtr< ID3D11Device3 > device,
            Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext 
        );

        void performHitDistanceSearch( 
            const Camera& camera,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< float > > hitDistance,
            std::shared_ptr< RenderTargetTexture2D< float > > blurredHitDistanceRenderTarget
        );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< HitDistanceSearchComputeShader >  m_hitDistanceSearchComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        HitDistanceSearchRenderer( const HitDistanceSearchRenderer& ) = delete;
        HitDistanceSearchRenderer& operator=( const HitDistanceSearchRenderer& ) = delete;
    };
}