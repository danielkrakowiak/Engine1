#include "ReflectionShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ReflectionShadingComputeShader.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

const int ReflectionShadingRenderer::maxRenderTargetCount = 10;

ReflectionShadingRenderer::ReflectionShadingRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
shadingComputeShader( std::make_shared< ReflectionShadingComputeShader >() )
{}

ReflectionShadingRenderer::~ReflectionShadingRenderer()
{}

void ReflectionShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
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

void ReflectionShadingRenderer::performShading( const int level, const Camera& camera,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture )
{
    if ( level < 0 || level >= reflectionTermRenderTargets.size() )
        throw std::exception( "ReflectionShadingRenderer::performShading - level out of bounds or negative." );

    rendererCore.disableRenderingPipeline();

    if ( level == 0 )
        shadingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), level, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, nullptr );
    else
        shadingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), level, positionTexture, normalTexture, albedoTexture, metalnessTexture, roughnessTexture, reflectionTermRenderTargets.at( level - 1 ) );

    rendererCore.enableComputeShader( shadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( reflectionTermRenderTargets.at( level ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    shadingComputeShader->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > ReflectionShadingRenderer::getReflectionTermTarget( int level )
{
    if ( level < 0 || level >= reflectionTermRenderTargets.size() )
        throw std::exception( "ReflectionShadingRenderer::getReflectionTermTarget - level out of bounds or negative." );

    return reflectionTermRenderTargets.at( level );
}

void ReflectionShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    reflectionTermRenderTargets.reserve( maxRenderTargetCount );

    for ( int i = 0; i < maxRenderTargetCount; ++i )
    {
        reflectionTermRenderTargets.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM ) );
    }
    
}

void ReflectionShadingRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    shadingComputeShader->compileFromFile( "../Engine1/Shaders/ReflectionShadingShader/cs.hlsl", device );
}
