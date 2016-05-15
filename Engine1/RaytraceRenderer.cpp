#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "GenerateRaysComputeShader.h"
#include "GenerateReflectedRefractedRaysComputeShader.h"
#include "RaytracingPrimaryRaysComputeShader.h"
#include "RaytracingSecondaryRaysComputeShader.h"
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
generateRaysComputeShader( std::make_shared< GenerateRaysComputeShader >() ),
generateReflectedRefractedRaysComputeShader( std::make_shared< GenerateReflectedRefractedRaysComputeShader >() ),
raytracingPrimaryRaysComputeShader( std::make_shared< RaytracingPrimaryRaysComputeShader >() ),
raytracingSecondaryRaysComputeShader( std::make_shared< RaytracingSecondaryRaysComputeShader >() )
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
    rayOriginsTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, 
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    rayDirectionsTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, 
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    rayHitPositionTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    rayHitDistanceTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT );

    rayHitAlbedoTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM );

    rayHitNormalTexture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true,
        DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

void RaytraceRenderer::generateAndTracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    generatePrimaryRays( camera );
    tracePrimaryRays( camera, actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceSecondaryRays( const Camera& camera, 
                                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture, 
                                                               const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                               const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateSecondaryRays( camera, positionTexture, normalTexture );
    traceSecondaryRays( actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generatePrimaryRays( const Camera& camera )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    generateRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize );

    rendererCore.enableComputeShader( generateRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayDirectionsTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateSecondaryRays( const Camera& camera, 
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    generateReflectedRefractedRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize, *positionTexture, *normalTexture );

    rendererCore.enableComputeShader( generateReflectedRefractedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayOriginsTexture );
    unorderedAccessTargets.push_back( rayDirectionsTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::tracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.enableComputeShader( raytracingPrimaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    rayHitPositionTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitDistanceTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rayHitAlbedoTexture->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitNormalTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >  unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( rayHitDistanceTexture );
    unorderedAccessTargetsF4.push_back( rayHitPositionTexture );
    unorderedAccessTargetsF4.push_back( rayHitNormalTexture );
    unorderedAccessTargetsU4.push_back( rayHitAlbedoTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU4 );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        if ( actor->getModel()->getAlbedoTextures().empty() )
            continue; // Skip models without textures.

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = actor->getModel()->getMesh()->getBoundingBox();

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture = actor->getModel()->getAlbedoTexture( 0 ).getTexture();

        raytracingPrimaryRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), *rayDirectionsTexture, *actor->getModel()->getMesh(), actor->getPose(), bbMin, bbMax, *albedoTexture );

        rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
    raytracingPrimaryRaysComputeShader->unsetParameters( *deviceContext.Get() );
}

void RaytraceRenderer::traceSecondaryRays( const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.enableComputeShader( raytracingSecondaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    rayHitPositionTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitDistanceTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rayHitAlbedoTexture->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitNormalTexture->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >  unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( rayHitDistanceTexture );
    unorderedAccessTargetsF4.push_back( rayHitPositionTexture );
    unorderedAccessTargetsF4.push_back( rayHitNormalTexture );
    unorderedAccessTargetsU4.push_back( rayHitAlbedoTexture );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU4 );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        if ( actor->getModel()->getAlbedoTextures().empty() )
            continue; // Skip models without textures.

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = actor->getModel()->getMesh()->getBoundingBox();

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture = actor->getModel()->getAlbedoTexture( 0 ).getTexture();

        raytracingSecondaryRaysComputeShader->setParameters( *deviceContext.Get(), *rayOriginsTexture, *rayDirectionsTexture, *actor->getModel()->getMesh(), actor->getPose(), bbMin, bbMax, *albedoTexture );

        rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
    raytracingSecondaryRaysComputeShader->unsetParameters( *deviceContext.Get() );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayOriginsTexture()
{
    return rayOriginsTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayDirectionsTexture()
{
    return rayDirectionsTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitPositionTexture()
{
    return rayHitPositionTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > > 
RaytraceRenderer::getRayHitDistanceTexture()
{
    return rayHitDistanceTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > 
RaytraceRenderer::getRayHitAlbedoTexture()
{
    return rayHitAlbedoTexture;
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitNormalTexture()
{
    return rayHitNormalTexture;
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    generateRaysComputeShader->compileFromFile( "../Engine1/Shaders/GenerateRaysShader/cs.hlsl", device );
    generateReflectedRefractedRaysComputeShader->compileFromFile( "../Engine1/Shaders/GenerateReflectedRefractedRaysShader/cs.hlsl", device );
    raytracingPrimaryRaysComputeShader->compileFromFile( "../Engine1/Shaders/RaytracingPrimaryRaysShader/cs.hlsl", device );
    raytracingSecondaryRaysComputeShader->compileFromFile( "../Engine1/Shaders/RaytracingSecondaryRaysShader/cs.hlsl", device );
}


