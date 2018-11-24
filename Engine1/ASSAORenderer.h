#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

#include "ASSAOCoreRenderer.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class ASSAORenderer
    {
        public:

        ASSAORenderer();
        ~ASSAORenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void renderAmbientOcclusion( 
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > destTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< uchar4 > > depthTexture,
            const float44& projectionMatrix,
            const float44& worldToViewspaceMatrix
        );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        std::shared_ptr< ASSAO_Effect > m_effect;

        // Copying is not allowed.
        ASSAORenderer( const ASSAORenderer& )           = delete;
        ASSAORenderer& operator=(const ASSAORenderer& ) = delete;
    };
}


