#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "GenerateRaysComputeShader.h"
#include "RaytracingComputeShader.h"
#include "uint3.h"
#include "Camera.h"
#include "MathUtil.h"
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

void RaytraceRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->device = device;
    this->deviceContext = deviceContext;

    this->imageWidth = imageWidth;
    this->imageHeight = imageHeight;

    createComputeTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( *device.Get() );

    initialized = true;
}

void RaytraceRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    rayDirectionsTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, 
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    rayHitDistanceTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT );

    rayHitBarycentricCoordsTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT );

    rayHitNormalTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT );
}

void RaytraceRenderer::generateAndTraceRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    disableRenderingPipeline();

    generateRays( camera );
    traceRays( camera, actors );

    disableComputePipeline();
}

void RaytraceRenderer::disableRenderingPipeline()
{
    rendererCore.disableRenderingShaders();
    rendererCore.disableRenderTargetViews();
    rendererCore.enableDefaultBlendState();
    rendererCore.enableDefaultRasterizerState();
    rendererCore.enableDefaultDepthStencilState();
    rendererCore.disableShaderInputs();
}

void RaytraceRenderer::disableComputePipeline()
{
    rendererCore.disableComputeShaders();
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateRays( const Camera& camera )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * fieldOfView;
    const float3 viewportRight   = camera.getRight() * screenAspect * fieldOfView;
    const float3 viewportTopLeft = camera.getPosition() + camera.getDirection() - 0.5f * viewportRight + 0.5f * viewportUp;

    generateRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportTopLeft, viewportUp, viewportRight, viewportSize );

    rendererCore.enableComputeShader( generateRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayDirectionsTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::traceRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.enableComputeShader( raytracingComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    rayHitDistanceTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rayHitBarycentricCoordsTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitNormalTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >  unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4;

    unorderedAccessTargetsF1.push_back( rayHitDistanceTexture );
    unorderedAccessTargetsF2.push_back( rayHitBarycentricCoordsTexture );
    unorderedAccessTargetsF2.push_back( rayHitNormalTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4 );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = actor->getModel()->getMesh()->getBoundingBox();
        raytracingComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), *rayDirectionsTexture, *actor->getModel()->getMesh(), actor->getPose(), bbMin, bbMax );

        rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
    raytracingComputeShader->unsetParameters( *deviceContext.Get() );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayDirectionsTexture()
{
    return rayDirectionsTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > > 
RaytraceRenderer::getRayHitDistanceTexture()
{
    return rayHitDistanceTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > > 
RaytraceRenderer::getRayHitBarycentricTexture()
{
    return rayHitBarycentricCoordsTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > > 
RaytraceRenderer::getRayHitNormalTexture()
{
    return rayHitNormalTexture;
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    generateRaysComputeShader->compileFromFile( "../Engine1/Shaders/GenerateRaysShader/cs.hlsl", device );
    raytracingComputeShader->compileFromFile( "../Engine1/Shaders/RaytracingShader/cs.hlsl", device );
}


