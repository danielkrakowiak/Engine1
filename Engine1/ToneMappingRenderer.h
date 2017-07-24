#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2D.h"

#include "float4.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class ToneMappingComputeShader;

    class ToneMappingRenderer
    {
        public:

        ToneMappingRenderer( Direct3DRendererCore& rendererCore );
        ~ToneMappingRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void performToneMapping( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                                 std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture,
                                 const float exposure );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        // Shaders.
        std::shared_ptr< ToneMappingComputeShader >  m_toneMappingComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        ToneMappingRenderer( const ToneMappingRenderer& ) = delete;
        ToneMappingRenderer& operator=( const ToneMappingRenderer& ) = delete;
    };
}







