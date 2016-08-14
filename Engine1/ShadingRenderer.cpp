#include "ShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ShadingComputeShader.h"
#include "ShadingComputeShader2.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingRenderer::ShadingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_shadingComputeShader( std::make_shared< ShadingComputeShader >() ),
    m_shadingComputeShader2( std::make_shared< ShadingComputeShader2 >() )
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

    loadAndCompileShaders( *device.Get() );

	m_initialized = true;
}

void ShadingRenderer::performShading( const Camera& camera,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > emissiveTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > indexOfRefractionTexture,
                                      const std::vector< std::shared_ptr< Light > >& lights )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, emissiveTexture, albedoTexture, metalnessTexture, roughnessTexture, normalTexture, indexOfRefractionTexture, lights );

    m_rendererCore.enableComputeShader( m_shadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ShadingRenderer::performShading( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitEmissiveTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitIndexOfRefractionTexture,
                                      const std::vector< std::shared_ptr< Light > >& lights )
{
    m_rendererCore.disableRenderingPipeline();

    m_shadingComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, rayHitPositionTexture, rayHitEmissiveTexture, rayHitAlbedoTexture,
                                         rayHitMetalnessTexture, rayHitRoughnessTexture, rayHitNormalTexture, rayHitIndexOfRefractionTexture, lights );

    m_rendererCore.enableComputeShader( m_shadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_colorRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_shadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > ShadingRenderer::getColorRenderTarget()
{
    return m_colorRenderTarget;
}

void ShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_colorRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

void ShadingRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    m_shadingComputeShader->compileFromFile( "Shaders/ShadingShader/cs.hlsl", device );
    m_shadingComputeShader2->compileFromFile( "Shaders/ShadingShader/cs2.hlsl", device );
}
