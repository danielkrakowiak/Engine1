#include "HitDistanceSearchRenderer.h"

#include "Direct3DRendererCore.h"

#include "HitDistanceSearchComputeShader.h"
#include "Camera.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

HitDistanceSearchRenderer::HitDistanceSearchRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_hitDistanceSearchComputeShader( std::make_shared< HitDistanceSearchComputeShader >() )
{}

HitDistanceSearchRenderer::~HitDistanceSearchRenderer()
{}

void HitDistanceSearchRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device,
                                                   ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    const int imageSizeDivider = 4;

    m_imageWidth  = imageWidth / imageSizeDivider;
    m_imageHeight = imageHeight / imageSizeDivider;

    createRenderTargets( m_imageWidth, m_imageHeight, *device.Get() );

    loadAndCompileShaders( device );

    m_initialized = true;
}

void HitDistanceSearchRenderer::performHitDistanceSearch( const Camera& camera,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > hitDistance )
{
    m_rendererCore.disableRenderingPipeline();

    m_hitDistanceSearchComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), 
                                                     int2(m_imageWidth, m_imageHeight),
                                                     positionTexture, normalTexture, hitDistance );

    m_rendererCore.enableComputeShader( m_hitDistanceSearchComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( m_finalHitDistanceRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( 
        (int)ceil( (float)m_imageWidth / 16.0f ), 
        (int)ceil( (float)m_imageHeight / 16.0f ), 
        1 
    );

    m_rendererCore.compute( groupCount );

    m_hitDistanceSearchComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >
HitDistanceSearchRenderer::getFinalHitDistanceTexture()
{
    return m_finalHitDistanceRenderTarget;
}

void HitDistanceSearchRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_finalHitDistanceRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT );
}

void HitDistanceSearchRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_hitDistanceSearchComputeShader->loadAndInitialize( "Shaders/HitDistanceSearchShader/HitDistanceSearch_cs.cso", device );
}