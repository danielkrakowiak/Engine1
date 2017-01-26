#include "ReplaceValueRenderer.h"

#include "Direct3DRendererCore.h"
#include "ReplaceValueComputeShader.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ReplaceValueRenderer::ReplaceValueRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_replaceValueComputeShader( std::make_shared< ReplaceValueComputeShader >() )
{}

ReplaceValueRenderer::~ReplaceValueRenderer()
{}

void ReplaceValueRenderer::initialize( ComPtr< ID3D11Device > device,
                                       ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device        = device;
    m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void ReplaceValueRenderer::replaceValues( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                                          const float replaceFromValue,
                                          const float replaceToValue )
{
    if ( !m_initialized )
        throw std::exception( "ReplaceValueRenderer::replaceValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( texture );
    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                 unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    m_replaceValueComputeShader->setParameters( *m_deviceContext.Get(), replaceFromValue, replaceToValue );

    m_rendererCore.enableComputeShader( m_replaceValueComputeShader );

    uint3 groupCount( texture->getWidth() / 8, texture->getHeight() / 8, 1 );

    m_rendererCore.compute( groupCount );

    m_replaceValueComputeShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void ReplaceValueRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_replaceValueComputeShader->loadAndInitialize( "Shaders/ReplaceValueShader/ReplaceValue_cs.cso", device );
}
