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
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    createRenderTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( device );

    m_initialized = true;
}

void HitDistanceSearchRenderer::performHitDistanceSearch( const Camera& camera,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > hitDistance )
{
    m_rendererCore.disableRenderingPipeline();

    m_hitDistanceSearchComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture,
                                                            normalTexture, hitDistance );

    m_rendererCore.enableComputeShader( m_hitDistanceSearchComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( m_finalHitDistanceRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

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