#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "uint3.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RaytraceRenderer::RaytraceRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
generateRaysComputeShader( std::make_shared<GenerateRaysComputeShader>() )
{}


RaytraceRenderer::~RaytraceRenderer()
{}

void RaytraceRenderer::initialize( int imageWidth, int imageHeight, ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
    this->device = &device;
    this->deviceContext = &deviceContext;

    this->imageWidth = imageWidth;
    this->imageHeight = imageHeight;

    createComputeTargets( imageWidth, imageHeight, device );

    loadAndCompileShaders( device );

    initialized = true;
}

void RaytraceRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    // Create render target.
    computeTarget = std::make_shared<ComputeTargetTexture2D>( imageWidth, imageHeight, device );
}

void RaytraceRenderer::clearComputeTargets( float4 color, float depth )
{
    computeTarget->clearOnGpu( color, *deviceContext.Get() );
}

void RaytraceRenderer::generateRays()
{
    // Disable rendering pipeline.
    rendererCore.disableRenderingShaders();
    rendererCore.disableRenderTargets();
    rendererCore.enableDefaultBlendState();
    rendererCore.enableDefaultRasterizerState();
    rendererCore.enableDefaultDepthStencilState();
    rendererCore.disableShaderInputs();

    rendererCore.enableComputeShader( generateRaysComputeShader );
    rendererCore.enableComputeTarget( computeTarget );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    rendererCore.disableComputeShaders();
    rendererCore.disableComputeTargets();
}

std::shared_ptr<ComputeTargetTexture2D> RaytraceRenderer::getComputeTarget()
{
    return computeTarget;
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    generateRaysComputeShader->compileFromFile( "../Engine1/Shaders/GenerateRaysShader/cs.hlsl", device );
}


