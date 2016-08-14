#include "ReflectionRefractionShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ReflectionShadingComputeShader.h"
#include "ReflectionShadingComputeShader2.h"
#include "RefractionShadingComputeShader.h"
#include "RefractionShadingComputeShader2.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

const int ReflectionRefractionShadingRenderer::maxRenderTargetCount = 10;

ReflectionRefractionShadingRenderer::ReflectionRefractionShadingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_reflectionShadingComputeShader( std::make_shared< ReflectionShadingComputeShader >() ),
    m_reflectionShadingComputeShader2( std::make_shared< ReflectionShadingComputeShader2 >() ),
    m_refractionShadingComputeShader( std::make_shared< RefractionShadingComputeShader >() ),
    m_refractionShadingComputeShader2( std::make_shared< RefractionShadingComputeShader2 >() )
{}

ReflectionRefractionShadingRenderer::~ReflectionRefractionShadingRenderer()
{}

void ReflectionRefractionShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
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

void ReflectionRefractionShadingRenderer::performFirstReflectionShading( const Camera& camera,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_reflectionShadingComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture );

    m_rendererCore.enableComputeShader( m_reflectionShadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_contributionTermRoughnessRenderTargets.at( 0 ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_reflectionShadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performReflectionShading( const int level,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_reflectionShadingComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, getContributionTermRoughnessTarget( level - 1 ) );

    m_rendererCore.enableComputeShader( m_reflectionShadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_contributionTermRoughnessRenderTargets.at( level ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_reflectionShadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performFirstRefractionShading( const Camera& camera,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_refractionShadingComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, nullptr );

    m_rendererCore.enableComputeShader( m_refractionShadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_contributionTermRoughnessRenderTargets.at( 0 ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_refractionShadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performRefractionShading( const int level,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    m_rendererCore.disableRenderingPipeline();

    m_refractionShadingComputeShader2->setParameters( *m_deviceContext.Get(), rayOriginTexture, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, getContributionTermRoughnessTarget( level - 1 ) );

    m_rendererCore.enableComputeShader( m_refractionShadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_contributionTermRoughnessRenderTargets.at( level ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_refractionShadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > ReflectionRefractionShadingRenderer::getContributionTermRoughnessTarget( int level )
{
    if ( level < 0 || level >= m_contributionTermRoughnessRenderTargets.size() )
        throw std::exception( "ReflectionRefractionShadingRenderer::getReflectionTermTarget - level out of bounds or negative." );

    return m_contributionTermRoughnessRenderTargets.at( level );
}

void ReflectionRefractionShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_contributionTermRoughnessRenderTargets.reserve( maxRenderTargetCount );

    for ( int i = 0; i < maxRenderTargetCount; ++i )
    {
        m_contributionTermRoughnessRenderTargets.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM ) );
    }
    
}

void ReflectionRefractionShadingRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    m_reflectionShadingComputeShader->compileFromFile( "Shaders/ReflectionShadingShader/cs.hlsl", device );
    m_reflectionShadingComputeShader2->compileFromFile( "Shaders/ReflectionShadingShader/cs2.hlsl", device );
    m_refractionShadingComputeShader->compileFromFile( "Shaders/RefractionShadingShader/cs.hlsl", device );
    m_refractionShadingComputeShader2->compileFromFile( "Shaders/RefractionShadingShader/cs2.hlsl", device );
}
