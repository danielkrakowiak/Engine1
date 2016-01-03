#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "GenerateRaysComputeShader.h"
#include "RaytracingComputeShader.h"
#include "uint3.h"
#include "Camera.h"
#include "MathUtil.h"
#include "Texture2D.h"
#include "BlockModel.h"
#include "BlockActor.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RaytraceRenderer::RaytraceRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
generateRaysComputeShader( std::make_shared<GenerateRaysComputeShader>() ),
raytracingComputeShader( std::make_shared<RaytracingComputeShader>() )
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
    rayDirectionsTexture = std::make_shared<ComputeTargetTexture2D>( imageWidth, imageHeight, device );
    rayHitsAlbedoTexture = std::make_shared<ComputeTargetTexture2D>( imageWidth, imageHeight, device );
}

void RaytraceRenderer::clearComputeTargets( float4 value )
{
    rayDirectionsTexture->clearOnGpu( value, *deviceContext.Get() );
    rayHitsAlbedoTexture->clearOnGpu( value, *deviceContext.Get() );
}

void RaytraceRenderer::generateAndTraceRays( const Camera& camera, const BlockActor& actor )
{
    disableRenderingPipeline();

    generateRays( camera );
    traceRays( camera, actor );

    disableComputePipeline();
}

void RaytraceRenderer::disableRenderingPipeline()
{
    rendererCore.disableRenderingShaders();
    rendererCore.disableRenderTargets();
    rendererCore.enableDefaultBlendState();
    rendererCore.enableDefaultRasterizerState();
    rendererCore.enableDefaultDepthStencilState();
    rendererCore.disableShaderInputs();
}

void RaytraceRenderer::disableComputePipeline()
{
    rendererCore.disableComputeShaders();
    rendererCore.disableComputeTargets();
}

void RaytraceRenderer::generateRays( const Camera& camera )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * fieldOfView;
    const float3 viewportRight   = camera.getRight() * screenAspect;
    const float3 viewportTopLeft = camera.getPosition() + camera.getDirection() - 0.5f * viewportRight + 0.5f * viewportUp;

    generateRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportTopLeft, viewportUp, viewportRight, viewportSize );

    rendererCore.enableComputeShader( generateRaysComputeShader );
    rendererCore.enableComputeTarget( rayDirectionsTexture );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableComputeTargets();
}

void RaytraceRenderer::traceRays( const Camera& camera, const BlockActor& actor )
{
    float3 bbMin, bbMax;
    std::tie( bbMin, bbMax ) = actor.getModel()->getMesh()->getBoundingBox();
    raytracingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), *rayDirectionsTexture, *actor.getModel()->getMesh(), actor.getPose(), bbMin, bbMax );

    rendererCore.enableComputeShader( raytracingComputeShader );
    rendererCore.enableComputeTarget( rayHitsAlbedoTexture );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableComputeTargets();
    raytracingComputeShader->unsetParameters( *deviceContext.Get() );
}

std::shared_ptr<ComputeTargetTexture2D> RaytraceRenderer::getRayDirectionsTexture()
{
    return rayDirectionsTexture;
}

std::shared_ptr<ComputeTargetTexture2D> RaytraceRenderer::getRayHitsAlbedoTexture()
{
    return rayHitsAlbedoTexture;
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    generateRaysComputeShader->compileFromFile( "../Engine1/Shaders/GenerateRaysShader/cs.hlsl", device );
    raytracingComputeShader->compileFromFile( "../Engine1/Shaders/RaytracingShader/cs.hlsl", device );
}


