#include "ExtractBrightPixelsRenderer.h"

#include "DX11RendererCore.h"

#include "ExtractBrightPixelsComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ExtractBrightPixelsRenderer::ExtractBrightPixelsRenderer( DX11RendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_extractBrightPixelsComputeShader( std::make_shared< ExtractBrightPixelsComputeShader >() )
{}

ExtractBrightPixelsRenderer::~ExtractBrightPixelsRenderer()
{}

void ExtractBrightPixelsRenderer::initialize( ComPtr< ID3D11Device3 > device,
                                              ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void ExtractBrightPixelsRenderer::extractBrightPixels( std::shared_ptr< Texture2D< float4 > > colorTexture,
                                                       std::shared_ptr< RenderTargetTexture2D< float4 > > destinationTexture,
                                                       const float minBrightness )
{
    m_rendererCore.disableRenderingPipeline();

    m_extractBrightPixelsComputeShader->setParameters( *m_deviceContext.Get(), *colorTexture, minBrightness );

    m_rendererCore.enableComputeShader( m_extractBrightPixelsComputeShader );

	RenderTargets unorderedAccessTargets;

	unorderedAccessTargets.typeFloat4.push_back( destinationTexture );

	m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = colorTexture->getWidth();
    const int imageHeight = colorTexture->getHeight();

    uint3 groupCount( 
        imageWidth / 16, 
        imageHeight / 16, 
        1 
    );

    m_rendererCore.compute( groupCount );

    m_extractBrightPixelsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ExtractBrightPixelsRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_extractBrightPixelsComputeShader->loadAndInitialize( "Engine1/Shaders/ExtractBrightPixelsShader/ExtractBrightPixels_cs.cso", device );
}


