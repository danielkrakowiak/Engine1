#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"
#include "RectangleMesh.h"

#include "float.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class GenerateMipmapMinValueComputeShader;
    class GenerateMipmapMinValueVertexShader;
    class GenerateMipmapMinValueFragmentShader;

    class MipmapMinValueRenderer
    {
        public:

        MipmapMinValueRenderer( Direct3DRendererCore& rendererCore );
        ~MipmapMinValueRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void generateMipmapsMinValue( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >& texture );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > createRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr< ID3D11BlendState >      createBlendState( ID3D11Device& device );

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > m_rasterizerState;
        Microsoft::WRL::ComPtr< ID3D11BlendState >      m_blendState;

        // Default mesh.
        RectangleMesh m_rectangleMesh;

        // Shaders.
        std::shared_ptr< GenerateMipmapMinValueComputeShader >  m_generateMipmapMinValueComputeShader;
        std::shared_ptr< GenerateMipmapMinValueVertexShader >   m_generateMipmapMinValueVertexShader;
        std::shared_ptr< GenerateMipmapMinValueFragmentShader > m_generateMipmapMinValueFragmentShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        MipmapMinValueRenderer( const MipmapMinValueRenderer& ) = delete;
        MipmapMinValueRenderer& operator=( const MipmapMinValueRenderer& ) = delete;
    };
}



