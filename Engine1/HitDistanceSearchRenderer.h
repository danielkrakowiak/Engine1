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
    class HitDistanceSearchComputeShader;
    class Camera;

    class HitDistanceSearchRenderer
    {
        public:

        HitDistanceSearchRenderer( Direct3DRendererCore& rendererCore );
        ~HitDistanceSearchRenderer();

        void initialize( 
            Microsoft::WRL::ComPtr< ID3D11Device > device,
            Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext 
        );

        void performHitDistanceSearch( 
            const Camera& camera,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > hitDistance,
            std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > blurredHitDistanceRenderTarget
        );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< HitDistanceSearchComputeShader >  m_hitDistanceSearchComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        HitDistanceSearchRenderer( const HitDistanceSearchRenderer& ) = delete;
        HitDistanceSearchRenderer& operator=( const HitDistanceSearchRenderer& ) = delete;
    };
}