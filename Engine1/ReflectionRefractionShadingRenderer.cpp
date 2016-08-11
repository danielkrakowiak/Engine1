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
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
reflectionShadingComputeShader( std::make_shared< ReflectionShadingComputeShader >() ),
reflectionShadingComputeShader2( std::make_shared< ReflectionShadingComputeShader2 >() ),
refractionShadingComputeShader( std::make_shared< RefractionShadingComputeShader >() ),
refractionShadingComputeShader2( std::make_shared< RefractionShadingComputeShader2 >() )
{}

ReflectionRefractionShadingRenderer::~ReflectionRefractionShadingRenderer()
{}

void ReflectionRefractionShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
                                          ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->device        = device;
	this->deviceContext = deviceContext;

	this->imageWidth  = imageWidth;
	this->imageHeight = imageHeight;

    createRenderTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( *device.Get() );

	initialized = true;
}

void ReflectionRefractionShadingRenderer::performFirstReflectionShading( const Camera& camera,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    rendererCore.disableRenderingPipeline();

    reflectionShadingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), 0, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture );

    rendererCore.enableComputeShader( reflectionShadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( contributionTermRoughnessRenderTargets.at( 0 ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    reflectionShadingComputeShader->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performReflectionShading( const int level,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    rendererCore.disableRenderingPipeline();

    reflectionShadingComputeShader2->setParameters( *deviceContext.Get(), rayOriginTexture, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, getContributionTermRoughnessTarget( level - 1 ) );

    rendererCore.enableComputeShader( reflectionShadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( contributionTermRoughnessRenderTargets.at( level ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    reflectionShadingComputeShader2->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performFirstRefractionShading( const Camera& camera,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    rendererCore.disableRenderingPipeline();

    refractionShadingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), 0, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, nullptr );

    rendererCore.enableComputeShader( refractionShadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( contributionTermRoughnessRenderTargets.at( 0 ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    refractionShadingComputeShader->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performRefractionShading( const int level,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    rendererCore.disableRenderingPipeline();

    refractionShadingComputeShader2->setParameters( *deviceContext.Get(), rayOriginTexture, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, getContributionTermRoughnessTarget( level - 1 ) );

    rendererCore.enableComputeShader( refractionShadingComputeShader2 );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( contributionTermRoughnessRenderTargets.at( level ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    refractionShadingComputeShader2->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > ReflectionRefractionShadingRenderer::getContributionTermRoughnessTarget( int level )
{
    if ( level < 0 || level >= contributionTermRoughnessRenderTargets.size() )
        throw std::exception( "ReflectionRefractionShadingRenderer::getReflectionTermTarget - level out of bounds or negative." );

    return contributionTermRoughnessRenderTargets.at( level );
}

void ReflectionRefractionShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    contributionTermRoughnessRenderTargets.reserve( maxRenderTargetCount );

    for ( int i = 0; i < maxRenderTargetCount; ++i )
    {
        contributionTermRoughnessRenderTargets.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM ) );
    }
    
}

void ReflectionRefractionShadingRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    reflectionShadingComputeShader->compileFromFile( "Shaders/ReflectionShadingShader/cs.hlsl", device );
    reflectionShadingComputeShader2->compileFromFile( "Shaders/ReflectionShadingShader/cs2.hlsl", device );
    refractionShadingComputeShader->compileFromFile( "Shaders/RefractionShadingShader/cs.hlsl", device );
    refractionShadingComputeShader2->compileFromFile( "Shaders/RefractionShadingShader/cs2.hlsl", device );
}
