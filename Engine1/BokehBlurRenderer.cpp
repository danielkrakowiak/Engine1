#include "BokehBlurRenderer.h"

#include "Direct3DRendererCore.h"

using namespace Engine1;
using namespace Microsoft::WRL;

BokehBlurRenderer::BokehBlurRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_shader( std::make_shared< BokehBlurComputeShader >() )
{
}

void BokehBlurRenderer::initialize( 
    ComPtr< ID3D11Device3 > device, 
    ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void BokehBlurRenderer::bokehBlur( 
    std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture,
    const Texture2D< float4 >& srcTexture,
    const Texture2D< uchar4 >& depthTexture )
{
    if ( !m_initialized )
        throw std::exception( "BokehBlurRenderer::bokehBlur - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF4.push_back( destTexture );
    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF3,
        unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    m_shader->setParameters( *m_deviceContext.Get(), srcTexture, depthTexture );

    m_rendererCore.enableComputeShader( m_shader );

    uint3 groupCount( 
        destTexture->getWidth() / 8, 
        destTexture->getHeight() / 8, 
        1 
    );

    m_rendererCore.compute( groupCount );

    m_shader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void BokehBlurRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_shader->loadAndInitialize( "Engine1/Shaders/BokehBlur/BokehBlur_cs.cso", device );
}
