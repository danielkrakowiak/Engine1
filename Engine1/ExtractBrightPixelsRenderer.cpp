#include "ExtractBrightPixelsRenderer.h"

#include "Direct3DRendererCore.h"

#include "ExtractBrightPixelsComputeShader.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ExtractBrightPixelsRenderer::ExtractBrightPixelsRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_extractBrightPixelsComputeShader( std::make_shared< ExtractBrightPixelsComputeShader >() )
{}

ExtractBrightPixelsRenderer::~ExtractBrightPixelsRenderer()
{}

void ExtractBrightPixelsRenderer::initialize( ComPtr< ID3D11Device > device,
                                              ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void ExtractBrightPixelsRenderer::extractBrightPixels( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture,
                                                       std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > destinationTexture,
                                                       const float minBrightness )
{
    m_rendererCore.disableRenderingPipeline();

    m_extractBrightPixelsComputeShader->setParameters( *m_deviceContext.Get(), *colorTexture, minBrightness );

    m_rendererCore.enableComputeShader( m_extractBrightPixelsComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF4.push_back( destinationTexture );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

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

void ExtractBrightPixelsRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_extractBrightPixelsComputeShader->loadAndInitialize( "Engine1/Shaders/ExtractBrightPixelsShader/ExtractBrightPixels_cs.cso", device );
}


