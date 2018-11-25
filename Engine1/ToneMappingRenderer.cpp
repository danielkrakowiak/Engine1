#include "ToneMappingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ToneMappingComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ToneMappingRenderer::ToneMappingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_toneMappingComputeShader( std::make_shared< ToneMappingComputeShader >() )
{}

ToneMappingRenderer::~ToneMappingRenderer()
{}

void ToneMappingRenderer::initialize( ComPtr< ID3D11Device3 > device,
                                              ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void ToneMappingRenderer::performToneMapping( std::shared_ptr< Texture2D< float4 > > srcTexture,
                                              std::shared_ptr< RenderTargetTexture2D< uchar4 > > dstTexture,
                                              const float exposure )
{
    m_rendererCore.disableRenderingPipeline();

    m_toneMappingComputeShader->setParameters( *m_deviceContext.Get(), *srcTexture, exposure );

    m_rendererCore.enableComputeShader( m_toneMappingComputeShader );

	RenderTargets unorderedAccessTargets;
	unorderedAccessTargets.typeUchar4.push_back( dstTexture );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = dstTexture->getWidth();
    const int imageHeight = dstTexture->getHeight();

    uint3 groupCount(
        imageWidth / 16,
        imageHeight / 16,
        1
    );

    m_rendererCore.compute( groupCount );

    m_toneMappingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ToneMappingRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_toneMappingComputeShader->loadAndInitialize( "Engine1/Shaders/ToneMappingShader/ToneMapping_cs.cso", device );
}



