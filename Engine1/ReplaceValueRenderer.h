#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class ReplaceValueComputeShader;

    class ReplaceValueRenderer
    {
        public:

        ReplaceValueRenderer( Direct3DRendererCore& rendererCore );
        ~ReplaceValueRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void replaceValues( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                            const float replaceFromValue,
                            const float replaceToValue );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< ReplaceValueComputeShader > m_replaceValueComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        ReplaceValueRenderer( const ReplaceValueRenderer& ) = delete;
        ReplaceValueRenderer& operator=( const ReplaceValueRenderer& ) = delete;
    };
}

