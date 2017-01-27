#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class ReplaceValueComputeShader;
    class Direct3DRendererCore;
    class SpreadValueComputeShader;
    class MergeValueComputeShader;

    class UtilityRenderer
    {
        public:

        UtilityRenderer( Direct3DRendererCore& rendererCore );
        ~UtilityRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void replaceValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                            const float replaceFromValue,
                            const float replaceToValue );

        void spreadMaxValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture, 
                              const int repeatCount,
                              const float ignorePixelIfBelowValue, 
                              const int mipmapLevel );

        void spreadMinValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                              const int repeatCount,
                              const float ignorePixelIfBelowValue,
                              const int mipmapLevel );

        void mergeMinValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > texture2 );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< ReplaceValueComputeShader > m_replaceValueComputeShader;
        std::shared_ptr< SpreadValueComputeShader >  m_spreadMaxValueComputeShader;
        std::shared_ptr< SpreadValueComputeShader >  m_spreadMinValueComputeShader;
        std::shared_ptr< MergeValueComputeShader >   m_mergeMinValueComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        UtilityRenderer( const UtilityRenderer& ) = delete;
        UtilityRenderer& operator=( const UtilityRenderer& ) = delete;
    };
}

