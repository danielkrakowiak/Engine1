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
    class DistanceToOccluderSearchComputeShader;
    class Light;
    class Camera;

    class DistanceToOccluderSearchRenderer
    {
        public:

        DistanceToOccluderSearchRenderer( Direct3DRendererCore& rendererCore );
        ~DistanceToOccluderSearchRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void performDistanceToOccluderSearch( const Camera& camera,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > finalDistanceToOccluderRenderTarget,
                                              const Light& light );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        // Shaders.
        std::shared_ptr< DistanceToOccluderSearchComputeShader >  m_distanceToOccluderSearchComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        DistanceToOccluderSearchRenderer( const DistanceToOccluderSearchRenderer& ) = delete;
        DistanceToOccluderSearchRenderer& operator=( const DistanceToOccluderSearchRenderer& ) = delete;
    };
}





