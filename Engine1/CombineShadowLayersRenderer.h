#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class Direct3DRendererCore;
    class CombineShadowLayersComputeShader;

    class CombineShadowLayersRenderer
    {
        public:

        CombineShadowLayersRenderer( Direct3DRendererCore& rendererCore );
        ~CombineShadowLayersRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void combineShadowLayers( 
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > finalShadowTexture,
            Texture2D< unsigned char >& hardShadowTexture,
            Texture2D< unsigned char >& mediumShadowTexture,
            Texture2D< unsigned char >& softShadowTexture
        );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< CombineShadowLayersComputeShader > m_combineShadowLayersComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        CombineShadowLayersRenderer( const CombineShadowLayersRenderer& ) = delete;
        CombineShadowLayersRenderer& operator=( const CombineShadowLayersRenderer& ) = delete;
    };
}

