#include "BlurShadowsRenderer.h"

#include "Direct3DRendererCore.h"

#include "BlurShadowsComputeShader.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

BlurShadowsRenderer::BlurShadowsRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_blurShadowsComputeShader( std::make_shared< BlurShadowsComputeShader >() )
{}

BlurShadowsRenderer::~BlurShadowsRenderer()
{}

void BlurShadowsRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device,
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

void BlurShadowsRenderer::blurShadows( const Camera& camera,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > illuminationTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluderTexture,
                                      const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

    m_blurShadowsComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, normalTexture, illuminationTexture, distanceToOccluderTexture, light );

    m_rendererCore.enableComputeShader( m_blurShadowsComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;
    
    unorderedAccessTargetsU1.push_back( m_illuminationRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_blurShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

//void BlurShadowsRenderer::performShading( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
//                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > illuminationTexture,
//                                      const Light& light )
//{
//    m_rendererCore.disableRenderingPipeline();
//
//    m_shadingComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, rayHitPositionTexture, rayHitAlbedoTexture,
//                                            rayHitMetalnessTexture, rayHitRoughnessTexture, rayHitNormalTexture, illuminationTexture, light );
//
//    m_rendererCore.enableComputeShader( m_shadingComputeShader2 );
//
//    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
//    unorderedAccessTargets.push_back( m_colorRenderTarget );
//
//    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );
//
//    const int imageWidth = rayOriginTexture->getWidth();
//    const int imageHeight = rayOriginTexture->getHeight();
//
//    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );
//
//    m_rendererCore.compute( groupCount );
//
//    m_shadingComputeShader2->unsetParameters( *m_deviceContext.Get() );
//
//    m_rendererCore.disableComputePipeline();
//}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > BlurShadowsRenderer::getIlluminationTexture()
{
    return m_illuminationRenderTarget;
}

void BlurShadowsRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_illuminationRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
}

void BlurShadowsRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_blurShadowsComputeShader->loadAndInitialize( "Shaders/BlurShadowsShader/BlurShadows_cs.cso", device );
}

