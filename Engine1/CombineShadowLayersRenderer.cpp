#include "CombineShadowLayersRenderer.h"

#include "Direct3DRendererCore.h"

#include "CombineShadowLayersComputeShader.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombineShadowLayersRenderer::CombineShadowLayersRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_combineShadowLayersComputeShader( std::make_shared< CombineShadowLayersComputeShader >() )
{}

CombineShadowLayersRenderer::~CombineShadowLayersRenderer()
{}

void CombineShadowLayersRenderer::initialize( ComPtr< ID3D11Device3 > device,
                                       ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}


void CombineShadowLayersRenderer::combineShadowLayers( 
    std::shared_ptr< RenderTargetTexture2D< unsigned char > > finalShadowTexture,
    Texture2D< unsigned char >& hardShadowTexture,
    Texture2D< unsigned char >& mediumShadowTexture,
    Texture2D< unsigned char >& softShadowTexture )
{
    if ( !m_initialized )
        throw std::exception( "CombineShadowLayersRenderer::sumValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

	RenderTargets unorderedAccessTargets;

	unorderedAccessTargets.typeUchar.push_back( finalShadowTexture );
    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    m_combineShadowLayersComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        hardShadowTexture, 
        mediumShadowTexture,
        softShadowTexture
    );

    m_rendererCore.enableComputeShader( m_combineShadowLayersComputeShader );

    uint3 groupCount( 
        finalShadowTexture->getWidth() / 16, 
        finalShadowTexture->getHeight() / 16, 
        1 
    );

    m_rendererCore.compute( groupCount );

    m_combineShadowLayersComputeShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();

    m_rendererCore.disableComputePipeline();
}

void CombineShadowLayersRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_combineShadowLayersComputeShader->loadAndInitialize( "Engine1/Shaders/CombineShadowLayersShader/CombineShadowLayers_cs.cso", device );
}

