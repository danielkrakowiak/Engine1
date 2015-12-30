#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "uint3.h"
#include "Camera.h"
#include "MathUtil.h"

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

void RaytraceRenderer::clearComputeTargets( float4 value )
{
    computeTarget->clearOnGpu( value, *deviceContext.Get() );
}

void RaytraceRenderer::generateRays( const Camera& camera )
{
    // Disable rendering pipeline.
    rendererCore.disableRenderingShaders();
    rendererCore.disableRenderTargets();
    rendererCore.enableDefaultBlendState();
    rendererCore.enableDefaultRasterizerState();
    rendererCore.enableDefaultDepthStencilState();
    rendererCore.disableShaderInputs();

    const float fieldOfView         = (float)MathUtil::pi / 4.0f;
    const float screenAspect        = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize       = float2( 1024.0f, 768.0f );
    const float3 viewportUp         = camera.getUp() * fieldOfView;
    const float3 viewportRight      = camera.getRight() * screenAspect;
    const float3 viewportBottomLeft = camera.getPosition() + camera.getDirection() - 0.5f * viewportRight - 0.5f * viewportUp;

    generateRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportBottomLeft, viewportUp, viewportRight, viewportSize );

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


