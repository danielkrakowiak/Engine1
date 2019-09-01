#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"
#include "RectangleMesh.h"

#include "float.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class DX11RendererCore;
    class GenerateMipmapMinValueComputeShader;
    class GenerateMipmapVertexShader;
	class ResampleTextureFragmentShader;
    class GenerateMipmapMinValueFragmentShader;
    class GenerateMipmapWithSampleRejectionFragmentShader;

    class MipmapRenderer
    {
        public:

        MipmapRenderer( DX11RendererCore& rendererCore );
        ~MipmapRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

		void resampleTexture( std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture, int destMipmapLevel,
			std::shared_ptr< Texture2D< float4 > > srcTexture, int srcMipmapLevel );

		void generateMipmaps( std::shared_ptr< RenderTargetTexture2D< float4 > > texture, int startSrcMipmapLevel = 0, int generateMipmapCount = -1 );

        void generateMipmapsMinValue( std::shared_ptr< RenderTargetTexture2D< float > >& texture );

        // @param generateMipmapCount - if 0 passed, generate all mips down to 1x1.
        void generateMipmapsWithSampleRejection( const std::shared_ptr< RenderTargetTexture2D< float > >& texture, 
                                                 const float maxAcceptableValue, const int initialSrcMipmapLevel, int generateMipmapCount = 0 );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > createRasterizerState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr< ID3D11BlendState >      createBlendState( ID3D11Device3& device );

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

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        MipmapRenderer( const MipmapRenderer& ) = delete;
        MipmapRenderer& operator=( const MipmapRenderer& ) = delete;
    };
}



