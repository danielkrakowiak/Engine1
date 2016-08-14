#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "ComputeTargetTexture2D.h"
#include "GenerateRaysComputeShader.h"
#include "GenerateFirstReflectedRaysComputeShader.h"
#include "GenerateFirstRefractedRaysComputeShader.h"
#include "GenerateReflectedRaysComputeShader.h"
#include "GenerateRefractedRaysComputeShader.h"
#include "RaytracingPrimaryRaysComputeShader.h"
#include "RaytracingSecondaryRaysComputeShader.h"
#include "uint3.h"
#include "Camera.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

const int RaytraceRenderer::maxRenderTargetCount = 10;

RaytraceRenderer::RaytraceRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
generateRaysComputeShader( std::make_shared< GenerateRaysComputeShader >() ),
generateFirstReflectedRaysComputeShader( std::make_shared< GenerateFirstReflectedRaysComputeShader >() ),
generateFirstRefractedRaysComputeShader( std::make_shared< GenerateFirstRefractedRaysComputeShader >() ),
generateReflectedRaysComputeShader( std::make_shared< GenerateReflectedRaysComputeShader >() ),
generateRefractedRaysComputeShader( std::make_shared< GenerateRefractedRaysComputeShader >() ),
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

    createDefaultTextures( *device.Get() );

    initialized = true;
}

void RaytraceRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    for ( int i = 0; i < maxRenderTargetCount; ++i )
    {
        rayOriginsTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                     ( device, imageWidth, imageHeight, false, true, false,
                                       DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        rayDirectionsTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                        ( device, imageWidth, imageHeight, false, true, false,
                                          DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        rayHitPositionTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        rayHitDistanceTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT ) );

        rayHitEmissiveTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM ) );

        rayHitAlbedoTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >
                                       ( device, imageWidth, imageHeight, false, true, false,
                                         DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM ) );

        rayHitMetalnessTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                          ( device, imageWidth, imageHeight, false, true, false,
                                            DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );

        rayHitRoughnessTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                          ( device, imageWidth, imageHeight, false, true, false,
                                            DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );

        rayHitNormalTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                       ( device, imageWidth, imageHeight, false, true, false,
                                         DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        rayHitIndexOfRefractionTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                                  ( device, imageWidth, imageHeight, false, true, false,
                                                    DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );
    }
}

void RaytraceRenderer::generateAndTracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    generatePrimaryRays( camera );
    tracePrimaryRays( camera, actors );

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
    unorderedAccessTargets.push_back( rayDirectionsTexture.at( 0 ) ); // #TODO: Whatch out for texture at zero index. Depending on setup could also be used as first reflection/refraction rays direction!

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateAndTraceFirstReflectedRays( const Camera& camera, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                                           const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstReflectedRays( camera, positionTexture, normalTexture, roughnessTexture, reflectionTermTexture );
    traceSecondaryRays( 0, actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceFirstRefractedRays( const Camera& camera, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture,
                                                           const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstRefractedRays( camera, positionTexture, normalTexture, roughnessTexture, refractionTermTexture );
    traceSecondaryRays( 0, actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceReflectedRays( const int level,
                                                      /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,*/
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                                      const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    generateReflectedRays( level, 
                           getRayDirectionsTexture( level - 1 ), 
                           getRayHitPositionTexture( level - 1 ), 
                           getRayHitNormalTexture( level - 1 ), 
                           getRayHitRoughnessTexture( level - 1 ), 
                           reflectionTermTexture );

    traceSecondaryRays( level, actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceRefractedRays( const int level,
                                                      /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,*/
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture,
                                                      const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.disableRenderingPipeline();

    generateRefractedRays( level, 
                           getRayDirectionsTexture( level - 1 ), 
                           getRayHitPositionTexture( level - 1 ), 
                           getRayHitNormalTexture( level - 1 ), 
                           getRayHitRoughnessTexture( level - 1 ), 
                           refractionTermTexture );

    traceSecondaryRays( level, actors );

    rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateFirstReflectedRays( const Camera& camera, 
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    generateFirstReflectedRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize, *positionTexture, *normalTexture, *roughnessTexture, *reflectionTermTexture );

    rendererCore.enableComputeShader( generateFirstReflectedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayOriginsTexture.at( 0 ) );
    unorderedAccessTargets.push_back( rayDirectionsTexture.at( 0 ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateFirstRefractedRays( const Camera& camera, 
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)imageWidth / (float)imageHeight;
    const float2 viewportSize    = float2( 1024.0f, 768.0f );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    generateFirstRefractedRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize, *positionTexture, *normalTexture, *roughnessTexture, *refractionTermTexture );

    rendererCore.enableComputeShader( generateFirstRefractedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayOriginsTexture.at( 0 ) );
    unorderedAccessTargets.push_back( rayDirectionsTexture.at( 0 ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateReflectedRays( int level,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture )
{
    generateReflectedRaysComputeShader->setParameters( *deviceContext.Get(), *rayDirectionTexture, *rayHitPositionTexture, *rayHitNormalTexture, *rayHitRoughnessTexture, *reflectionTermTexture );

    rendererCore.enableComputeShader( generateReflectedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayOriginsTexture.at( level ) );
    unorderedAccessTargets.push_back( rayDirectionsTexture.at( level ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

    rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateRefractedRays( int level,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture )
{
    generateRefractedRaysComputeShader->setParameters( *deviceContext.Get(), *rayDirectionTexture, *rayHitPositionTexture, *rayHitNormalTexture, *rayHitRoughnessTexture, *refractionTermTexture );

    rendererCore.enableComputeShader( generateRefractedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( rayOriginsTexture.at( level ) );
    unorderedAccessTargets.push_back( rayDirectionsTexture.at( level ) );

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
    rayHitPositionTexture.at( 0 )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitDistanceTexture.at( 0 )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rayHitEmissiveTexture.at( 0 )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitAlbedoTexture.at( 0 )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitMetalnessTexture.at( 0 )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitRoughnessTexture.at( 0 )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitNormalTexture.at( 0 )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitIndexOfRefractionTexture.at( 0 )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( rayHitDistanceTexture.at( 0 ) );
    unorderedAccessTargetsF4.push_back( rayHitPositionTexture.at( 0 ) );
    unorderedAccessTargetsF4.push_back( rayHitNormalTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( rayHitMetalnessTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( rayHitRoughnessTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( rayHitIndexOfRefractionTexture.at( 0 ) );
    unorderedAccessTargetsU4.push_back( rayHitEmissiveTexture.at( 0 ) );
    unorderedAccessTargetsU4.push_back( rayHitAlbedoTexture.at( 0 ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        const BlockModel& model = *actor->getModel();

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = model.getMesh()->getBoundingBox();

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *defaultIndexOfRefractionTexture;

        raytracingPrimaryRaysComputeShader->setParameters( *deviceContext.Get(), camera.getPosition(), *getRayDirectionsTexture( 0 ), *actor->getModel()->getMesh(), actor->getPose(),
                                                           bbMin, bbMax, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

        rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
    raytracingPrimaryRaysComputeShader->unsetParameters( *deviceContext.Get() );
}

void RaytraceRenderer::traceSecondaryRays( int level, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    rendererCore.enableComputeShader( raytracingSecondaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    rayHitPositionTexture.at( level )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitDistanceTexture.at( level )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rayHitEmissiveTexture.at( level )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitAlbedoTexture.at( level )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitMetalnessTexture.at( level )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitRoughnessTexture.at( level )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rayHitNormalTexture.at( level )->clearUnorderedAccessViewFloat( *deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rayHitIndexOfRefractionTexture.at( level )->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( rayHitDistanceTexture.at( level ) );
    unorderedAccessTargetsF4.push_back( rayHitPositionTexture.at( level ) );
    unorderedAccessTargetsF4.push_back( rayHitNormalTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( rayHitMetalnessTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( rayHitRoughnessTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( rayHitIndexOfRefractionTexture.at( level ) );
    unorderedAccessTargetsU4.push_back( rayHitEmissiveTexture.at( level ) );
    unorderedAccessTargetsU4.push_back( rayHitAlbedoTexture.at( level ) );

    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        const BlockModel& model = *actor->getModel();

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = model.getMesh()->getBoundingBox();

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *defaultIndexOfRefractionTexture;

        raytracingSecondaryRaysComputeShader->setParameters( *deviceContext.Get(), *rayOriginsTexture.at( level ), *rayDirectionsTexture.at( level ), *actor->getModel()->getMesh(), actor->getPose(), 
                                                             bbMin, bbMax, alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

        rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    rendererCore.disableUnorderedAccessViews();
    raytracingSecondaryRaysComputeShader->unsetParameters( *deviceContext.Get() );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayOriginsTexture( int level )
{
    return rayOriginsTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayDirectionsTexture( int level )
{
    return rayDirectionsTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitPositionTexture( int level )
{
    return rayHitPositionTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > > 
RaytraceRenderer::getRayHitDistanceTexture( int level )
{
    return rayHitDistanceTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > 
RaytraceRenderer::getRayHitEmissiveTexture( int level )
{
    return rayHitEmissiveTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > 
RaytraceRenderer::getRayHitAlbedoTexture( int level )
{
    return rayHitAlbedoTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > 
RaytraceRenderer::getRayHitMetalnessTexture( int level )
{
    return rayHitMetalnessTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > 
RaytraceRenderer::getRayHitRoughnessTexture( int level )
{
    return rayHitRoughnessTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitNormalTexture( int level )
{
    return rayHitNormalTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > 
RaytraceRenderer::getRayHitIndexOfRefractionTexture( int level )
{
    return rayHitIndexOfRefractionTexture.at( level );
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    generateRaysComputeShader->compileFromFile( "Shaders/GenerateRaysShader/cs.hlsl", device );
    generateFirstReflectedRaysComputeShader->compileFromFile( "Shaders/GenerateFirstReflectedRaysShader/cs.hlsl", device );
    generateReflectedRaysComputeShader->compileFromFile( "Shaders/GenerateReflectedRaysShader/cs.hlsl", device );
    generateFirstRefractedRaysComputeShader->compileFromFile( "Shaders/GenerateFirstRefractedRaysShader/cs.hlsl", device );
    generateRefractedRaysComputeShader->compileFromFile( "Shaders/GenerateRefractedRaysShader/cs.hlsl", device );
    raytracingPrimaryRaysComputeShader->compileFromFile( "Shaders/RaytracingPrimaryRaysShader/cs.hlsl", device );
    raytracingSecondaryRaysComputeShader->compileFromFile( "Shaders/RaytracingSecondaryRaysShader/cs.hlsl", device );
}

void RaytraceRenderer::createDefaultTextures( ID3D11Device& device )
{
    std::vector< unsigned char > dataAlpha             = { 255 };
    std::vector< unsigned char > dataMetalness         = { 0 };
    std::vector< unsigned char > dataRoughness         = { 0 };
    std::vector< unsigned char > dataIndexOfRefraction = { 0 };
    std::vector< uchar4 >        dataEmissive          = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataAlbedo            = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataNormal            = { uchar4( 0, 0, 255, 0 ) };

    defaultAlphaTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultMetalnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultRoughnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultIndexOfRefractionTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultEmissiveTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    defaultAlbedoTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    defaultNormalTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
}


