#include "UtilityRenderer.h"

#include "Direct3DRendererCore.h"

#include "ReplaceValueComputeShader.h"
#include "SpreadValueComputeShader.h"
#include "MergeValueComputeShader.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

UtilityRenderer::UtilityRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_replaceValueComputeShader( std::make_shared< ReplaceValueComputeShader >() ),
    m_spreadMaxValueComputeShader( std::make_shared< SpreadValueComputeShader >() ),
    m_spreadMinValueComputeShader( std::make_shared< SpreadValueComputeShader >() ),
    m_spreadSparseMinValueComputeShader( std::make_shared< SpreadValueComputeShader >() ),
    m_mergeMinValueComputeShader( std::make_shared< MergeValueComputeShader >() )
{}

UtilityRenderer::~UtilityRenderer()
{}

void UtilityRenderer::initialize( ComPtr< ID3D11Device > device,
                                       ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void UtilityRenderer::replaceValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                                     const int mipmapLevel,
                                     const float replaceFromValue,
                                     const float replaceToValue )
{
    if ( !m_initialized )
        throw std::exception( "UtilityRenderer::replaceValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( texture );
    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                 unorderedAccessTargetsU1, unorderedAccessTargetsU4, mipmapLevel );

    m_replaceValueComputeShader->setParameters( *m_deviceContext.Get(), replaceFromValue, replaceToValue );

    m_rendererCore.enableComputeShader( m_replaceValueComputeShader );

    uint3 groupCount( texture->getWidth() / 8, texture->getHeight() / 8, 1 );

    m_rendererCore.compute( groupCount );

    m_replaceValueComputeShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void UtilityRenderer::spreadMaxValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture, 
                                       const int mipmapLevel,
                                       const int repeatCount,
                                       const float ignorePixelIfBelowValue )
{
    if ( !m_initialized )
        throw std::exception( "SpreadValueRenderer::spreadMaxValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( texture );
    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                 unorderedAccessTargetsU1, unorderedAccessTargetsU4, mipmapLevel );

    uint3 groupCount( 
        texture->getWidth( mipmapLevel ) / 8, 
        texture->getHeight( mipmapLevel ) / 8, 
        1 
    );

    m_rendererCore.enableComputeShader( m_spreadMaxValueComputeShader );

    for( int i = 0; i < repeatCount; ++i )
    {
        m_spreadMaxValueComputeShader->setParameters( *m_deviceContext.Get(), ignorePixelIfBelowValue, 666.0f, 1, 0 ); //#TODO: min spread value to be removed.

        m_rendererCore.compute( groupCount );
    }

    m_spreadMaxValueComputeShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void UtilityRenderer::spreadMinValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                                       const int mipmapLevel,
                                       const int repeatCount,
                                       const float ignorePixelIfBelowValue,
                                       const int spreadDistance,
                                       const int offset )
{
    if ( !m_initialized )
        throw std::exception( "SpreadValueRenderer::spreadMinValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( texture );
    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                 unorderedAccessTargetsU1, unorderedAccessTargetsU4, mipmapLevel );

    const bool useSparseSpread = (spreadDistance > 0);

    uint3 groupCount(
        std::ceil( (float)texture->getWidth( mipmapLevel ) / (float)std::max( 1, spreadDistance ) / 8.0f ),
        std::ceil( (float)texture->getHeight( mipmapLevel ) / (float)std::max( 1, spreadDistance ) / 8.0f ),
        1
    );

    // Decide which shader to use depending on spread distance.
    auto& spreadValueShader = useSparseSpread 
        ? m_spreadSparseMinValueComputeShader 
        : m_spreadMinValueComputeShader;

    m_rendererCore.enableComputeShader( spreadValueShader );

    for ( int i = 0; i < repeatCount; ++i ) 
    {
        const float minAcceptableValue = logf( (float)i + 1.0f ) * 0.01f;

        spreadValueShader->setParameters( *m_deviceContext.Get(), ignorePixelIfBelowValue, minAcceptableValue, spreadDistance, offset );

        m_rendererCore.compute( groupCount );
    }

    spreadValueShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void UtilityRenderer::mergeMinValues( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > texture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > texture2,
                                      const int mipmapLevel )
{
    if ( !m_initialized )
        throw std::exception( "UtilityRenderer::replaceValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( texture );
    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                 unorderedAccessTargetsU1, unorderedAccessTargetsU4, mipmapLevel );

    m_mergeMinValueComputeShader->setParameters( *m_deviceContext.Get(), *texture2 );

    m_rendererCore.enableComputeShader( m_mergeMinValueComputeShader );

    uint3 groupCount( texture->getWidth() / 8, texture->getHeight() / 8, 1 );

    m_rendererCore.compute( groupCount );

    m_mergeMinValueComputeShader->unsetParameters( *m_deviceContext.Get() );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void UtilityRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_replaceValueComputeShader->loadAndInitialize( "Shaders/ReplaceValueShader/ReplaceValue_cs.cso", device );
    m_spreadMaxValueComputeShader->loadAndInitialize( "Shaders/SpreadValueShader/SpreadMaxValue_cs.cso", device );
    m_spreadMinValueComputeShader->loadAndInitialize( "Shaders/SpreadValueShader/SpreadMinValue_cs.cso", device );
    m_spreadSparseMinValueComputeShader->loadAndInitialize( "Shaders/SpreadValueShader/SpreadSparseMinValue_cs.cso", device );
    m_mergeMinValueComputeShader->loadAndInitialize( "Shaders/MergeValueShader/MergeMinValue_cs.cso", device );
}
