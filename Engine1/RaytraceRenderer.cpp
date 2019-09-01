#include "RaytraceRenderer.h"

#include <d3d11_3.h>

#include "DX11RendererCore.h"
#include "GenerateRaysComputeShader.h"
#include "GenerateFirstReflectedRaysComputeShader.h"
#include "GenerateFirstRefractedRaysComputeShader.h"
#include "GenerateReflectedRaysComputeShader.h"
#include "GenerateRefractedRaysComputeShader.h"
#include "RaytracingPrimaryRaysComputeShader.h"
#include "RaytracingSecondaryRaysComputeShader.h"
#include "SumValuesComputeShader.h"
#include "uint3.h"
#include "Camera.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"
#include "Texture2Dtypes.h"

#include "Settings.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RaytraceRenderer::RaytraceRenderer( DX11RendererCore& rendererCore ) :
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
    m_raytracingSecondaryRaysComputeShader( std::make_shared< RaytracingSecondaryRaysComputeShader >() ),
    m_sumValueComputeShader( std::make_shared< SumValuesComputeShader< float > >() )
{}


RaytraceRenderer::~RaytraceRenderer()
{}

void RaytraceRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device, ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void RaytraceRenderer::generateAndTracePrimaryRays( 
    const Camera& camera, 
    RaytraceRenderTargets& renderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generatePrimaryRays( camera, renderTargets );
    tracePrimaryRays( camera, renderTargets, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generatePrimaryRays( 
    const Camera& camera,
    RaytraceRenderTargets& rtRenderTargets )
{
    const float fieldOfView      = camera.getFieldOfView();
    const float screenAspect     = (float)m_imageWidth / (float)m_imageHeight;
    const float2 viewportSize    = float2( (float)m_imageWidth, (float)m_imageHeight );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    m_generateRaysComputeShader->setParameters( *m_deviceContext.Get(), camera.getPosition(), viewportCenter, viewportUp, viewportRight, viewportSize );

    m_rendererCore.enableComputeShader( m_generateRaysComputeShader );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayDirection );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( m_imageWidth / 32, m_imageHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
}

void RaytraceRenderer::generateAndTraceFirstReflectedRays( 
    const Camera& camera, 
    InputTextures1& inputTextures,
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstReflectedRays( camera, inputTextures, rtRenderTargets );
    traceSecondaryRays( rtRenderTargets, actors );

    // #TODO: Account for depth and calculate hit-dist-to-camera.
    //calculateHitDistanceToCamera( renderTargets );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceFirstRefractedRays( 
    const Camera& camera, 
    InputTextures1& inputTextures,
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    //generatePrimaryRays( camera );
    generateFirstRefractedRays( camera, inputTextures, rtRenderTargets );
    traceSecondaryRays( rtRenderTargets, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceReflectedRays( 
    InputTextures2& inputTextures,
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generateReflectedRays( inputTextures, rtRenderTargets );

    traceSecondaryRays( rtRenderTargets, actors );

    calculateHitDistanceToCamera( inputTextures, rtRenderTargets );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateAndTraceRefractedRays( 
    const int refractionLevel,
    InputTextures2& inputTextures,
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.disableRenderingPipeline();

    generateRefractedRays( refractionLevel, inputTextures, rtRenderTargets );

    traceSecondaryRays( rtRenderTargets, actors );

    m_rendererCore.disableComputePipeline();
}

void RaytraceRenderer::generateFirstReflectedRays( 
    const Camera& camera, 
    InputTextures1& inputTextures,
    RaytraceRenderTargets& rtRenderTargets )
{
    const int outputTextureWidth  = rtRenderTargets.rayOrigin->getWidth();
    const int outputTextureHeight = rtRenderTargets.rayOrigin->getHeight();

    const float fieldOfView      = camera.getFieldOfView();
    const float screenAspect     = (float)outputTextureWidth / (float)outputTextureHeight;
    const float2 viewportSize    = float2( (float)outputTextureWidth, (float)outputTextureHeight );
    const float3 viewportUp      = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight   = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter  = camera.getPosition() + camera.getDirection();

    m_generateFirstReflectedRaysComputeShader->setParameters( 
        *m_deviceContext.Get(), camera.getPosition(), 
        viewportCenter, viewportUp, viewportRight, viewportSize,
        *inputTextures.prevHitPosition, 
        *inputTextures.prevHitNormal, 
        *inputTextures.prevHitRoughness, 
        *inputTextures.contribution,
        outputTextureWidth, outputTextureHeight 
    );

    m_rendererCore.enableComputeShader( m_generateFirstReflectedRaysComputeShader );

	RenderTargets unorderedAccessTargets;
	unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayOrigin );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayDirection );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( outputTextureWidth / 32, outputTextureHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
}

void RaytraceRenderer::generateFirstRefractedRays( 
    const Camera& camera, 
    InputTextures1& inputTextures,
    RaytraceRenderTargets& rtRenderTargets )
{
    const int outputTextureWidth  = rtRenderTargets.rayOrigin->getWidth();
    const int outputTextureHeight = rtRenderTargets.rayOrigin->getHeight();

    const float fieldOfView     = camera.getFieldOfView();
    const float screenAspect    = (float)outputTextureWidth / (float)outputTextureHeight;
    const float2 viewportSize   = float2( (float)outputTextureWidth, (float)outputTextureHeight );
    const float3 viewportUp     = camera.getUp() * tan( fieldOfView * 0.5f );
    const float3 viewportRight  = camera.getRight() * screenAspect * viewportUp.length();
    const float3 viewportCenter = camera.getPosition() + camera.getDirection();

    m_generateFirstRefractedRaysComputeShader->setParameters( 
        *m_deviceContext.Get(), camera.getPosition(), 
        viewportCenter, viewportUp, viewportRight, viewportSize,
        *inputTextures.prevHitPosition, 
        *inputTextures.prevHitNormal, 
        *inputTextures.prevHitRoughness, 
        *inputTextures.prevHitRefractiveIndex, 
        *inputTextures.contribution,
        outputTextureWidth, outputTextureHeight 
    );

    m_rendererCore.enableComputeShader( m_generateFirstRefractedRaysComputeShader );

    RenderTargets unorderedAccessTargets;

    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayOrigin );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayDirection );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.currentRefractiveIndex );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( outputTextureWidth / 32, outputTextureHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
}

void RaytraceRenderer::generateReflectedRays( 
    InputTextures2& inputTextures,
    RaytraceRenderTargets& renderTargets )
{
    const int outputTextureWidth  = renderTargets.rayOrigin->getWidth();
    const int outputTextureHeight = renderTargets.rayOrigin->getHeight();

    m_generateReflectedRaysComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        *inputTextures.prevRayDirection, 
        *inputTextures.prevHitPosition,
        *inputTextures.prevHitNormal, 
        *inputTextures.prevHitRoughness, 
        *inputTextures.contribution,
        outputTextureWidth, outputTextureHeight 
    );

    m_rendererCore.enableComputeShader( m_generateReflectedRaysComputeShader );

    RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeFloat4.push_back( renderTargets.rayOrigin );
    unorderedAccessTargets.typeFloat4.push_back( renderTargets.rayDirection );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( outputTextureWidth / 32, outputTextureHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
}

void RaytraceRenderer::generateRefractedRays( 
    const int refractionLevel,
    InputTextures2& inputTextures,
    RaytraceRenderTargets& rtRenderTargets )
{
    // Optional clearing - for better debug. Clearing probably not needed, because it gets overwritten in the shader anyways.
    #if defined(_DEBUG)
    rtRenderTargets.hitRefractiveIndex->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rtRenderTargets.currentRefractiveIndex->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    #endif

    const int outputTextureWidth  = rtRenderTargets.rayOrigin->getWidth();
    const int outputTextureHeight = rtRenderTargets.rayOrigin->getHeight();

    m_generateRefractedRaysComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        refractionLevel,
        *inputTextures.prevRayDirection,
        *inputTextures.prevHitPosition,
        *inputTextures.prevHitNormal,
        *inputTextures.prevHitRoughness,
        *inputTextures.prevHitRefractiveIndex,
        *inputTextures.contribution,
        /*refractionLevel >= 2 ? m_currentRefractiveIndexTextures.at( refractionLevel - 2 ) : nullptr, // #TODO: IMPORTANT logic to be moved to calling classes.
        refractionLevel >= 1 ? m_currentRefractiveIndexTextures.at( refractionLevel - 1 ) : nullptr,*/
        inputTextures.prevPrevCurrentRefractiveIndex,
        inputTextures.prevCurrentRefractiveIndex,
        outputTextureWidth, outputTextureHeight
    );

    //assert( refractionLevel < m_currentRefractiveIndexTextures.size() );

    m_rendererCore.enableComputeShader( m_generateRefractedRaysComputeShader );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayOrigin );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.rayDirection );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.currentRefractiveIndex );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( outputTextureWidth / 32, outputTextureHeight / 32, 1 );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
}

void RaytraceRenderer::tracePrimaryRays( 
    const Camera& camera, 
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    m_rendererCore.enableComputeShader( m_raytracingPrimaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
    rtRenderTargets.hitPosition->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rtRenderTargets.hitDistance->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
    rtRenderTargets.hitEmissive->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rtRenderTargets.hitAlbedo->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rtRenderTargets.hitMetalness->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rtRenderTargets.hitRoughness->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
    rtRenderTargets.hitNormal->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
    rtRenderTargets.hitRefractiveIndex->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

	RenderTargets unorderedAccessTargets;

    unorderedAccessTargets.typeFloat.push_back( rtRenderTargets.hitDistance );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.hitPosition );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.hitNormal );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitMetalness );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitRoughness );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitRefractiveIndex );
	unorderedAccessTargets.typeUchar4.push_back( rtRenderTargets.hitEmissive );
	unorderedAccessTargets.typeUchar4.push_back( rtRenderTargets.hitAlbedo );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = rtRenderTargets.hitPosition->getWidth();
    const int imageHeight = rtRenderTargets.hitPosition->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        const BlockModel& model = *actor->getModel();

        const BoundingBox bbBox = model.getMesh()->getBoundingBox();

        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ZERO;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 0.0f;

        const Texture2D< uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() && model.getEmissiveTextures()[ 0 ].getTexture()
            ? *model.getEmissiveTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.emissive;

        const Texture2D< uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() && model.getAlbedoTextures()[ 0 ].getTexture()
            ? *model.getAlbedoTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.albedo;

        const Texture2D< uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() && model.getNormalTextures()[ 0 ].getTexture()
            ? *model.getNormalTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.normal;

        const Texture2D< unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() && model.getMetalnessTextures()[ 0 ].getTexture()
            ? *model.getMetalnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.metalness;

        const Texture2D< unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() && model.getRoughnessTextures()[ 0 ].getTexture()
            ? *model.getRoughnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.roughness;

        const Texture2D< unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() && model.getRefractiveIndexTextures()[ 0 ].getTexture()
            ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.refractiveIndex;

        m_raytracingPrimaryRaysComputeShader->setParameters( 
            *m_deviceContext.Get(), 
            camera.getPosition(), 
            *rtRenderTargets.rayDirection, 
            *actor->getModel()->getMesh(), 
            actor->getPose(),
            bbBox.getMin(), 
            bbBox.getMax(), 
            emissiveTexture, emissiveMul,
            albedoTexture, albedoMul,
            normalTexture, normalMul,
            metalnessTexture, metalnessMul,
            roughnessTexture, roughnessMul,
            indexOfRefractionTexture, indexOfRefractionMul );

        m_rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
    m_raytracingPrimaryRaysComputeShader->unsetParameters( *m_deviceContext.Get() );
}

void RaytraceRenderer::traceSecondaryRays( 
    RaytraceRenderTargets& rtRenderTargets,
    const std::vector< std::shared_ptr< BlockActor > >& actors )
{
	m_rendererCore.enableComputeShader( m_raytracingSecondaryRaysComputeShader );

    // Clear unordered access targets.
    const float maxDist = 15000.0f; // Note: Should be less than max dist in the raytracing shader!
	rtRenderTargets.hitPosition->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	rtRenderTargets.hitDistance->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( maxDist, 0.0f, 0.0f, 0.0f ) );
	rtRenderTargets.hitEmissive->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( (uint3)(settings().rendering.skyColor * 255.0f), 0 ) );
	rtRenderTargets.hitAlbedo->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
	rtRenderTargets.hitMetalness->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
	rtRenderTargets.hitRoughness->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );
	rtRenderTargets.hitNormal->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	rtRenderTargets.hitRefractiveIndex->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 0 ) );

	RenderTargets unorderedAccessTargets;

    unorderedAccessTargets.typeFloat.push_back( rtRenderTargets.hitDistance );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.hitPosition );
    unorderedAccessTargets.typeFloat4.push_back( rtRenderTargets.hitNormal );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitMetalness );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitRoughness );
	unorderedAccessTargets.typeUchar.push_back( rtRenderTargets.hitRefractiveIndex );
	unorderedAccessTargets.typeUchar4.push_back( rtRenderTargets.hitEmissive );
	unorderedAccessTargets.typeUchar4.push_back( rtRenderTargets.hitAlbedo );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  =rtRenderTargets.hitPosition->getWidth();
    const int imageHeight =rtRenderTargets.hitPosition->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    for ( const std::shared_ptr< const BlockActor >& actor : actors )
    {
        if ( !actor->getModel() || !actor->getModel()->getMesh() )
            continue;

        const BlockModel& model = *actor->getModel();

        const BoundingBox bbBox = model.getMesh()->getBoundingBox();

        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ZERO;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 0.0f;

        const Texture2D< unsigned char >& alphaTexture 
            = !model.getAlphaTextures().empty() && model.getAlphaTextures()[ 0 ].getTexture()
            ? *model.getAlphaTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.alpha;

        const Texture2D< uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() && model.getEmissiveTextures()[ 0 ].getTexture() 
            ? *model.getEmissiveTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.emissive;

        const Texture2D< uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() && model.getAlbedoTextures()[ 0 ].getTexture()
            ? *model.getAlbedoTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.albedo;

        const Texture2D< uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() && model.getNormalTextures()[ 0 ].getTexture()
            ? *model.getNormalTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.normal;

        const Texture2D< unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() && model.getMetalnessTextures()[ 0 ].getTexture()
            ? *model.getMetalnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.metalness;

        const Texture2D< unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() && model.getRoughnessTextures()[ 0 ].getTexture()
            ? *model.getRoughnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.roughness;

        const Texture2D< unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() && model.getRefractiveIndexTextures()[ 0 ].getTexture()
            ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.refractiveIndex;

        m_raytracingSecondaryRaysComputeShader->setParameters( 
            *m_deviceContext.Get(), 
            *rtRenderTargets.rayOrigin, 
            *rtRenderTargets.rayDirection, 
            *actor->getModel()->getMesh(), actor->getPose(), 
            bbBox.getMin(), 
            bbBox.getMax(), 
            alphaTexture, alphaMul,
            emissiveTexture, emissiveMul,
            albedoTexture, albedoMul,
            normalTexture, normalMul,
            metalnessTexture, metalnessMul,
            roughnessTexture, roughnessMul,
            indexOfRefractionTexture, indexOfRefractionMul,
            imageWidth, 
            imageHeight 
        );

        m_rendererCore.compute( groupCount );
    }

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();
    m_raytracingSecondaryRaysComputeShader->unsetParameters( *m_deviceContext.Get() );
}

void RaytraceRenderer::calculateHitDistanceToCamera( 
	InputTextures2& inputs, 
	RaytraceRenderTargets& rtRenderTargets )
{
	RenderTargets unorderedAccessTargets;

    m_rendererCore.enableComputeShader( m_sumValueComputeShader );

    unorderedAccessTargets.typeFloat.push_back( rtRenderTargets.hitDistanceToCamera );

	m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = rtRenderTargets.hitDistanceToCamera->getWidth();
    const int imageHeight = rtRenderTargets.hitDistanceToCamera->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    // #TODO: This distance doesn't account for initial depth (which should be converted to dist-to-camera).
    // Another shader should be used to calculate dist-to-camera from depth after deferred rendering.
    // #TODO: Does this summing work correctly when both refraction/reflections are enabled (but only one dit-to-camera per layer, right?)?

    // #TODO: Take depth from prev layer and calculate prevHitDist from it - then add curr hit dist. 
        /*m_sumValueComputeShader->setParameters(
            *m_deviceContext.Get(),
            *renderTargets.hitDistance
        );*/
        
    m_sumValueComputeShader->setParameters(
        *m_deviceContext.Get(),
        *inputs.prevHitDistanceToCamera,
        *rtRenderTargets.hitDistance
    );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();

    m_sumValueComputeShader->unsetParameters( *m_deviceContext.Get() );
}

void RaytraceRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_generateRaysComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateRaysShader/GenerateRays_cs.cso", device );
    m_generateFirstReflectedRaysComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateFirstReflectedRaysShader/GenerateFirstReflectedRays_cs.cso", device );
    m_generateReflectedRaysComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateReflectedRaysShader/GenerateReflectedRays_cs.cso", device );
    m_generateFirstRefractedRaysComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateFirstRefractedRaysShader/GenerateFirstRefractedRays_cs.cso", device );
    m_generateRefractedRaysComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateRefractedRaysShader/GenerateRefractedRays_cs.cso", device );
    m_raytracingPrimaryRaysComputeShader->loadAndInitialize( "Engine1/Shaders/RaytracingPrimaryRaysShader/RaytracingPrimaryRays_cs.cso", device );
    m_raytracingSecondaryRaysComputeShader->loadAndInitialize( "Engine1/Shaders/RaytracingSecondaryRaysShader/RaytracingSecondaryRays_cs.cso", device );
    m_sumValueComputeShader->loadAndInitialize( "Engine1/Shaders/SumValuesShader/SumTwoFloatValues_cs.cso", device );
}

