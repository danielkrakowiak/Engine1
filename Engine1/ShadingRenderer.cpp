#include "ShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ShadingComputeShader0.h"
#include "ShadingComputeShader.h"
#include "ShadingComputeShader2.h"
#include "ShadingNoShadowsComputeShader.h"
#include "ShadingNoShadowsComputeShader2.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingRenderer::ShadingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_shadingComputeShader0( std::make_shared< ShadingComputeShader0 >() ),
    m_shadingComputeShader( std::make_shared< ShadingComputeShader >() ),
    m_shadingComputeShader2( std::make_shared< ShadingComputeShader2 >() ),
    m_shadingNoShadowsComputeShader( std::make_shared< ShadingNoShadowsComputeShader >() ),
    m_shadingNoShadowsComputeShader2( std::make_shared< ShadingNoShadowsComputeShader2 >() )
{}

ShadingRenderer::~ShadingRenderer()
{}

void ShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
                                          ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

	this->m_imageWidth  = imageWidth;
	this->m_imageHeight = imageHeight;

    createRenderTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( device );

	m_initialized = true;
}

void ShadingRenderer::performShading( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > emissiveTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingComputeShader0->setParameters( *m_deviceContext.Get(), emissiveTexture );

    m_rendererCore.enableComputeShader( m_shadingComputeShader0 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    const int imageWidth  = emissiveTexture->getWidth();
    const int imageHeight = emissiveTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingComputeShader0->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::performShading( const Camera& camera,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
	                                  const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > illuminationTexture,
	                                  const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, albedoTexture, metalnessTexture, roughnessTexture, normalTexture, illuminationTexture, light );

    m_rendererCore.enableComputeShader( m_shadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    const int imageWidth  = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::performShadingNoShadows( const Camera& camera,
                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                               const std::vector< std::shared_ptr< Light > > lights )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingNoShadowsComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, albedoTexture, metalnessTexture, roughnessTexture, normalTexture, lights );

    m_rendererCore.enableComputeShader( m_shadingNoShadowsComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingNoShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::performShading( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
									  const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > illuminationTexture,
	                                  const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, rayHitPositionTexture, rayHitAlbedoTexture,
                                         rayHitMetalnessTexture, rayHitRoughnessTexture, rayHitNormalTexture, illuminationTexture, light );

    m_rendererCore.enableComputeShader( m_shadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    const int imageWidth  = rayOriginTexture->getWidth();
    const int imageHeight = rayOriginTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::performShadingNoShadows( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                      const std::vector< std::shared_ptr< Light > > lights )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingNoShadowsComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, rayHitPositionTexture, rayHitAlbedoTexture,
                                            rayHitMetalnessTexture, rayHitRoughnessTexture, rayHitNormalTexture, lights );

    m_rendererCore.enableComputeShader( m_shadingNoShadowsComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    const int imageWidth = rayOriginTexture->getWidth();
    const int imageHeight = rayOriginTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingNoShadowsComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::clearColorRenderTarget()
{
	m_colorRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 1.0f ) );
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > ShadingRenderer::getColorRenderTarget()
{
    return m_colorRenderTarget;
}

void ShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_colorRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

void ShadingRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_shadingComputeShader0->loadAndInitialize( "Shaders/ShadingShader/Shading_cs0.cso", device );
    m_shadingComputeShader->loadAndInitialize( "Shaders/ShadingShader/Shading_cs.cso", device );
    m_shadingComputeShader2->loadAndInitialize( "Shaders/ShadingShader/Shading_cs2.cso", device );
    m_shadingNoShadowsComputeShader->loadAndInitialize( "Shaders/ShadingNoShadowsShader/ShadingNoShadows_cs.cso", device );
    m_shadingNoShadowsComputeShader2->loadAndInitialize( "Shaders/ShadingNoShadowsShader/ShadingNoShadows_cs2.cso", device );
}
