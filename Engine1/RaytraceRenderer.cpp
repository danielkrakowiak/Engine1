#include "RaytraceRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
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
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_generateRaysComputeShader( std::make_shared< GenerateRaysComputeShader >() ),
    m_generateFirstReflectedRaysComputeShader( std::make_shared< GenerateFirstReflectedRaysComputeShader >() ),
    m_generateFirstRefractedRaysComputeShader( std::make_shared< GenerateFirstRefractedRaysComputeShader >() ),
    m_generateReflectedRaysComputeShader( std::make_shared< GenerateReflectedRaysComputeShader >() ),
    m_generateRefractedRaysComputeShader( std::make_shared< GenerateRefractedRaysComputeShader >() ),
    m_raytracingPrimaryRaysComputeShader( std::make_shared< RaytracingPrimaryRaysComputeShader >() ),
    m_raytracingSecondaryRaysComputeShader( std::make_shared< RaytracingSecondaryRaysComputeShader >() )
{}


RaytraceRenderer::~RaytraceRenderer()
{}

void RaytraceRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    createComputeTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( *device.Get() );

    createDefaultTextures( *device.Get() );

    m_initialized = true;
}

void RaytraceRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    for ( int i = 0; i < maxRenderTargetCount; ++i )
    {
        m_rayOriginsTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                     ( device, imageWidth, imageHeight, false, true, false,
                                       DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        m_rayDirectionsTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                        ( device, imageWidth, imageHeight, false, true, false,
                                          DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        m_rayHitPositionTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        m_rayHitDistanceTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT ) );

        m_rayHitEmissiveTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >
                                         ( device, imageWidth, imageHeight, false, true, false,
                                           DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM ) );

        m_rayHitAlbedoTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >
                                       ( device, imageWidth, imageHeight, false, true, false,
                                         DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM ) );

        m_rayHitMetalnessTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                          ( device, imageWidth, imageHeight, false, true, false,
                                            DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );

        m_rayHitRoughnessTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                          ( device, imageWidth, imageHeight, false, true, false,
                                            DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );

        m_rayHitRefractiveIndexTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                            ( device, imageWidth, imageHeight, false, true, false,
                                              DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM ) );

        m_rayHitNormalTexture.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
                                       ( device, imageWidth, imageHeight, false, true, false,
                                         DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

        m_currentRefractiveIndexTextures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
                                                  ( device, imageWidth, imageHeight, false, true, false,
                                                    DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM ) );
    }
}

void RaytraceRenderer::generateAndTracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generatePrimaryRays( camera );
    tracePrimaryRays( camera, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generatePrimaryRays( const Camera& camera )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)m_imageWidth / (float)m_imageHeight;
    const float2 viewportSize    = float2( m_imageWidth, m_imageHeight );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    m_generateRaysComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize );

    m_rendererCore.enableComputeShader( m_generateRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_rayDirectionsTexture.at( 0 ) ); // #TODO: Whatch out for texture at zero index. Depending on setup could also be used as first reflection/refraction rays direction!

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateAndTraceFirstReflectedRays( const Camera& camera, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                                           const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstReflectedRays( camera, positionTexture, normalTexture, roughnessTexture, contributionTermTexture );
    traceSecondaryRays( 0, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceFirstRefractedRays( const Camera& camera, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture, 
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > refractiveIndexTexture,
                                                           const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                                           const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstRefractedRays( camera, positionTexture, normalTexture, roughnessTexture, refractiveIndexTexture, contributionTermTexture );
    traceSecondaryRays( 0, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceReflectedRays( const int level,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                                      const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generateReflectedRays( level, 
                           getRayDirectionsTexture( level - 1 ), 
                           getRayHitPositionTexture( level - 1 ), 
                           getRayHitNormalTexture( level - 1 ), 
                           getRayHitRoughnessTexture( level - 1 ), 
                           contributionTermTexture );

    traceSecondaryRays( level, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceRefractedRays( const int level, const int refractionLevel,
                                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                                      const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generateRefractedRays( level, refractionLevel,
                           getRayDirectionsTexture( level - 1 ), 
                           getRayHitPositionTexture( level - 1 ), 
                           getRayHitNormalTexture( level - 1 ), 
                           getRayHitRoughnessTexture( level - 1 ), 
                           getRayHitRefractiveIndexTexture( level - 1 ),
                           contributionTermTexture );

    traceSecondaryRays( level, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateFirstReflectedRays( const Camera& camera, 
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)m_imageWidth / (float)m_imageHeight;
    const float2 viewportSize    = float2( m_imageWidth, m_imageHeight );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    m_generateFirstReflectedRaysComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize, *positionTexture, *normalTexture, *roughnessTexture, *contributionTermTexture );

    m_rendererCore.enableComputeShader( m_generateFirstReflectedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_rayOriginsTexture.at( 0 ) );
    unorderedAccessTargets.push_back( m_rayDirectionsTexture.at( 0 ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateFirstRefractedRays( const Camera& camera, 
                                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > refractiveIndexTexture,
                                                   const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture )
{
    const float fieldOfView      = (float)MathUtil::pi / 4.0f;
    const float screenAspect     = (float)m_imageWidth / (float)m_imageHeight;
    const float2 viewportSize    = float2( m_imageWidth, m_imageHeight );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    m_generateFirstRefractedRaysComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize, 
                                                              *positionTexture, *normalTexture, *roughnessTexture, *refractiveIndexTexture, *contributionTermTexture/*, getCurrentRefractiveIndexTextures()*/ );

    m_rendererCore.enableComputeShader( m_generateFirstRefractedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;
    unorderedAccessTargetsF4.push_back( m_rayOriginsTexture.at( 0 ) );
    unorderedAccessTargetsF4.push_back( m_rayDirectionsTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( m_currentRefractiveIndexTextures.at( 0 ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateReflectedRays( int level,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture )
{
    m_generateReflectedRaysComputeShader->setParameters( *m_deviceContext.Get(), *rayDirectionTexture, *rayHitPositionTexture, *rayHitNormalTexture, *rayHitRoughnessTexture, *contributionTermTexture );

    m_rendererCore.enableComputeShader( m_generateReflectedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargets;
    unorderedAccessTargets.push_back( m_rayOriginsTexture.at( level ) );
    unorderedAccessTargets.push_back( m_rayDirectionsTexture.at( level ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::generateRefractedRays( int level, const int refractionLevel,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRefractiveIndexTexture,
                                              const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture )
{
    // Optional clearing - for better debug. Clearing probably not needed, because it gets overwritten in the shader anyways.
    #if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
    m_rayHitRefractiveIndexTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_currentRefractiveIndexTextures.at( refractionLevel )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    #endif

    m_generateRefractedRaysComputeShader->setParameters( *m_deviceContext.Get(), 
                                                         refractionLevel, 
                                                         *rayDirectionTexture, 
                                                         *rayHitPositionTexture, 
                                                         *rayHitNormalTexture, 
                                                         *rayHitRoughnessTexture,
                                                         *rayHitRefractiveIndexTexture, 
                                                         *contributionTermTexture,
                                                         refractionLevel >= 2 ? m_currentRefractiveIndexTextures.at( refractionLevel - 2 ) : nullptr,
                                                         refractionLevel >= 1 ? m_currentRefractiveIndexTextures.at( refractionLevel - 1 ) : nullptr
                                                         );

    assert( refractionLevel < m_currentRefractiveIndexTextures.size() );

    m_rendererCore.enableComputeShader( m_generateRefractedRaysComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;
    unorderedAccessTargetsF4.push_back( m_rayOriginsTexture.at( level ) );
    unorderedAccessTargetsF4.push_back( m_rayDirectionsTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( m_currentRefractiveIndexTextures.at( refractionLevel ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
}

void RaytraceRenderer::tracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.enableComputeShader( m_raytracingPrimaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    m_rayHitPositionTexture.at( 0 )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    m_rayHitDistanceTexture.at( 0 )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    m_rayHitEmissiveTexture.at( 0 )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitAlbedoTexture.at( 0 )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitMetalnessTexture.at( 0 )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitRoughnessTexture.at( 0 )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitNormalTexture.at( 0 )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    m_rayHitRefractiveIndexTexture.at( 0 )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( m_rayHitDistanceTexture.at( 0 ) );
    unorderedAccessTargetsF4.push_back( m_rayHitPositionTexture.at( 0 ) );
    unorderedAccessTargetsF4.push_back( m_rayHitNormalTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( m_rayHitMetalnessTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( m_rayHitRoughnessTexture.at( 0 ) );
    unorderedAccessTargetsU1.push_back( m_rayHitRefractiveIndexTexture.at( 0 ) );
    unorderedAccessTargetsU4.push_back( m_rayHitEmissiveTexture.at( 0 ) );
    unorderedAccessTargetsU4.push_back( m_rayHitAlbedoTexture.at( 0 ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        const BlockModel& model = *actor->getModel();

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = model.getMesh()->getBoundingBox();

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *m_defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *m_defaultIndexOfRefractionTexture;

        m_raytracingPrimaryRaysComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), *getRayDirectionsTexture( 0 ), *actor->getModel()->getMesh(), actor->getPose(),
                                                           bbMin, bbMax, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

        m_rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
    m_raytracingPrimaryRaysComputeShader->unsetParameters( *m_deviceContext.Get() );
}

void RaytraceRenderer::traceSecondaryRays( int level, const std::vector< std::shared_ptr< const BlockActor > >& actors )
{
    m_rendererCore.enableComputeShader( m_raytracingSecondaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    m_rayHitPositionTexture.at( level )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    m_rayHitDistanceTexture.at( level )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    m_rayHitEmissiveTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitAlbedoTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitMetalnessTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitRoughnessTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    m_rayHitNormalTexture.at( level )->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    m_rayHitRefractiveIndexTexture.at( level )->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( m_rayHitDistanceTexture.at( level ) );
    unorderedAccessTargetsF4.push_back( m_rayHitPositionTexture.at( level ) );
    unorderedAccessTargetsF4.push_back( m_rayHitNormalTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( m_rayHitMetalnessTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( m_rayHitRoughnessTexture.at( level ) );
    unorderedAccessTargetsU1.push_back( m_rayHitRefractiveIndexTexture.at( level ) );
    unorderedAccessTargetsU4.push_back( m_rayHitEmissiveTexture.at( level ) );
    unorderedAccessTargetsU4.push_back( m_rayHitAlbedoTexture.at( level ) );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    uint3 groupCount( m_imageWidth / 16, m_imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        const BlockModel& model = *actor->getModel();

        float3 bbMin, bbMax;
        std::tie( bbMin, bbMax ) = model.getMesh()->getBoundingBox();

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *m_defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *m_defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *m_defaultIndexOfRefractionTexture;

        m_raytracingSecondaryRaysComputeShader->setParameters( *m_deviceContext.Get(), *m_rayOriginsTexture.at( level ), *m_rayDirectionsTexture.at( level ), *actor->getModel()->getMesh(), actor->getPose(), 
                                                             bbMin, bbMax, alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

        m_rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();
    m_raytracingSecondaryRaysComputeShader->unsetParameters( *m_deviceContext.Get() );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayOriginsTexture( int level )
{
    return m_rayOriginsTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayDirectionsTexture( int level )
{
    return m_rayDirectionsTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitPositionTexture( int level )
{
    return m_rayHitPositionTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > > 
RaytraceRenderer::getRayHitDistanceTexture( int level )
{
    return m_rayHitDistanceTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > 
RaytraceRenderer::getRayHitEmissiveTexture( int level )
{
    return m_rayHitEmissiveTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > 
RaytraceRenderer::getRayHitAlbedoTexture( int level )
{
    return m_rayHitAlbedoTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > 
RaytraceRenderer::getRayHitMetalnessTexture( int level )
{
    return m_rayHitMetalnessTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > 
RaytraceRenderer::getRayHitRoughnessTexture( int level )
{
    return m_rayHitRoughnessTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
RaytraceRenderer::getRayHitRefractiveIndexTexture( int level )
{
    return m_rayHitRefractiveIndexTexture.at( level );
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > 
RaytraceRenderer::getRayHitNormalTexture( int level )
{
    return m_rayHitNormalTexture.at( level );
}

const std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& 
RaytraceRenderer::getCurrentRefractiveIndexTextures()
{
    return m_currentRefractiveIndexTextures;
}

void RaytraceRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    m_generateRaysComputeShader->compileFromFile( "Shaders/GenerateRaysShader/cs.hlsl", device );
    m_generateFirstReflectedRaysComputeShader->compileFromFile( "Shaders/GenerateFirstReflectedRaysShader/cs.hlsl", device );
    m_generateReflectedRaysComputeShader->compileFromFile( "Shaders/GenerateReflectedRaysShader/cs.hlsl", device );
    m_generateFirstRefractedRaysComputeShader->compileFromFile( "Shaders/GenerateFirstRefractedRaysShader/cs.hlsl", device );
    m_generateRefractedRaysComputeShader->compileFromFile( "Shaders/GenerateRefractedRaysShader/cs.hlsl", device );
    m_raytracingPrimaryRaysComputeShader->compileFromFile( "Shaders/RaytracingPrimaryRaysShader/cs.hlsl", device );
    m_raytracingSecondaryRaysComputeShader->compileFromFile( "Shaders/RaytracingSecondaryRaysShader/cs.hlsl", device );
}

void RaytraceRenderer::createDefaultTextures( ID3D11Device& device )
{
    std::vector< unsigned char > dataAlpha             = { 255 };
    std::vector< unsigned char > dataMetalness         = { 0 };
    std::vector< unsigned char > dataRoughness         = { 0 };
    std::vector< unsigned char > dataIndexOfRefraction = { 0 };
    std::vector< uchar4 >        dataEmissive          = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataAlbedo            = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataNormal            = { uchar4( 128, 128, 255, 255 ) };

    m_defaultAlphaTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultMetalnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultRoughnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultIndexOfRefractionTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultEmissiveTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_defaultAlbedoTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_defaultNormalTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
}


