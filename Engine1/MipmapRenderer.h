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
    class GenerateMipmapVertexShader;
	class ResampleTextureFragmentShader;
    class GenerateMipmapMinValueFragmentShader;
    class GenerateMipmapWithSampleRejectionFragmentShader;

    class MipmapRenderer
    {
        public:

        MipmapRenderer( Direct3DRendererCore& rendererCore );
        ~MipmapRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

		void resampleTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture, int destMipmapLevel,
			std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture, int srcMipmapLevel );

		void generateMipmaps( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > texture, int startSrcMipmapLevel = 0, int generateMipmapCount = -1 );

        void generateMipmapsMinValue( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >& texture );

        // @param generateMipmapCount - if 0 passed, generate all mips down to 1x1.
        void generateMipmapsWithSampleRejection( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >& texture, 
                                                 const float maxAcceptableValue, const int initialSrcMipmapLevel, int generateMipmapCount = 0 );

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
        std::shared_ptr< GenerateMipmapMinValueComputeShader >             m_generateMipmapMinValueComputeShader;
        std::shared_ptr< GenerateMipmapVertexShader >                      m_generateMipmapVertexShader;
		std::shared_ptr< ResampleTextureFragmentShader >                    m_resampleTextureFragmentShader;
        std::shared_ptr< GenerateMipmapMinValueFragmentShader >            m_generateMipmapMinValueFragmentShader;
        std::shared_ptr< GenerateMipmapWithSampleRejectionFragmentShader > m_generateMipmapWithSampleRejectionFragmentShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Copying is not allowed.
        MipmapRenderer( const MipmapRenderer& ) = delete;
        MipmapRenderer& operator=( const MipmapRenderer& ) = delete;
    };
}



