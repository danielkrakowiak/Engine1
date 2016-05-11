#include "ShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ShadingComputeShader.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingRenderer::ShadingRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
shadingComputeShader( std::make_shared< ShadingComputeShader >() )
{}

ShadingRenderer::~ShadingRenderer()
{}

void ShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
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

void ShadingRenderer::performShading( const Camera& camera,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > > normalTexture,
                                      const std::vector< std::shared_ptr< Light > >& lights )
{
    rendererCore.disableRenderingPipeline();

    shadingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), positionTexture, albedoTexture, normalTexture, lights );

    rendererCore.enableComputeShader( shadingComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( colorRenderTarget );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    rendererCore.compute( groupCount );

    shadingComputeShader->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 > > ShadingRenderer::getColorRenderTarget()
{
    return colorRenderTarget;
}

void ShadingRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    colorRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

void ShadingRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    shadingComputeShader->compileFromFile( "../Engine1/Shaders/ShadingShader/cs.hlsl", device );
}
