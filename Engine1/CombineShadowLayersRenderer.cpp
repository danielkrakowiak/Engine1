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
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > finalShadowTexture,
    Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& hardShadowTexture,
    Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& mediumShadowTexture,
    Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& softShadowTexture )
{
    if ( !m_initialized )
        throw std::exception( "CombineShadowLayersRenderer::sumValues - renderer has not been initialized." );

    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsU1.push_back( finalShadowTexture );
    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF3, 
        unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 
    );

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
    m_rendererCore.disableUnorderedAccessViews();

    m_rendererCore.disableComputePipeline();
}

void CombineShadowLayersRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_combineShadowLayersComputeShader->loadAndInitialize( "Engine1/Shaders/CombineShadowLayersShader/CombineShadowLayers_cs.cso", device );
}

