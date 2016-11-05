#include "Renderer.h"

#include <memory>

#include "Direct3DRendererCore.h"
#include "Profiler.h"
#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "ReflectionRefractionShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "CombiningRenderer.h"
#include "TextureRescaleRenderer.h"
#include "Scene.h"
#include "Camera.h"
#include "MathUtil.h"
#include "BlockActor.h"
#include "BlockModel.h"
#include "SkeletonActor.h"
#include "SkeletonModel.h"
#include "Light.h"

#include "StagingTexture2D.h"

#include "StringUtil.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Renderer::Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler ) :
    m_rendererCore( rendererCore ),
    m_profiler( profiler ),
    m_deferredRenderer( rendererCore ),
    m_raytraceRenderer( rendererCore ),
    m_shadingRenderer( rendererCore ),
    m_reflectionRefractionShadingRenderer( rendererCore ),
    m_edgeDetectionRenderer( rendererCore ),
    m_combiningRenderer( rendererCore ),
    m_textureRescaleRenderer( rendererCore ),
	m_raytraceShadowRenderer( rendererCore ),
	m_shadowMapRenderer( rendererCore ),
    m_activeViewType( View::Final ),
    m_maxLevelCount( 1 )
{}

Renderer::~Renderer()
{}

void Renderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext, 
                         std::shared_ptr<const BlockMesh> axisMesh, std::shared_ptr<const BlockModel> lightModel )
{
    this->m_device        = device;
    this->m_deviceContext = deviceContext;

    this->m_axisMesh = axisMesh;
    this->m_lightModel = lightModel;

	m_deferredRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_raytraceRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_shadingRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_reflectionRefractionShadingRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_edgeDetectionRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_combiningRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_textureRescaleRenderer.initialize( device, deviceContext );
	m_raytraceShadowRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
	m_shadowMapRenderer.initialize( 256, 256, device, deviceContext );

    createRenderTargets( imageWidth, imageHeight, *device.Get() );
}

// Should be called at the beginning of each frame, before calling renderScene(). 
void Renderer::clear()
{
    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    m_deferredRenderer.clearRenderTargets( float4( 0.0f, 0.0f, 0.0f, 1.0f ), 1.0f ); 
}

void Renderer::clear2()
{
    m_deferredRenderer.clearRenderTargets( float4( 0.0f, 0.0f, 0.0f, 0.0f ), 1.0f ); 
}

void Renderer::renderShadowMaps( const Scene& scene )
{
	// TODO: render shadow maps for each light.
}

std::tuple< 
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderScene( const Scene& scene, const Camera& camera,
                       const bool wireframeMode,
                       const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
                       const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
                       const std::vector< std::shared_ptr< Light > >& selectedLights )
{
    bool frameReceived = false;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    const std::unordered_set< std::shared_ptr< Actor > >& actors = scene.getActors();

    const std::unordered_set< std::shared_ptr<Light> >& sceneLights = scene.getLights();
    const std::vector< std::shared_ptr< Light > >       lights( sceneLights.begin(), sceneLights.end() );
    
    std::vector< std::shared_ptr< Light > > lightsCastingShadows;
    std::vector< std::shared_ptr< Light > > lightsNotCastingShadows;
    lightsCastingShadows.reserve( lights.size() );
    lightsNotCastingShadows.reserve( lights.size() );

    for ( auto& light : lights ) {
        if ( light->isEnabled() && !light->isCastingShadows() )
            lightsNotCastingShadows.push_back( light );
        else if ( light->isEnabled() && light->isCastingShadows() )
            lightsCastingShadows.push_back( light );
    }

    // Gather block actors.
    std::vector< std::shared_ptr< const BlockActor > > blockActors;
    blockActors.reserve( actors.size() );
    for ( const std::shared_ptr< Actor > actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            blockActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
    }

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat ) 
        = renderMainImage( scene, camera, lightsCastingShadows, lightsNotCastingShadows, m_activeViewLevel, m_activeViewType, wireframeMode,
                           selectedBlockActors, selectedSkeletonActors, selectedLights );
    
    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    //OutputDebugStringW( StringUtil::widen( "------------------- \n\n\n" ).c_str() );

    std::vector< bool > renderedViewType;

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( true, 1, 0, m_maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
                                        renderedViewType, m_activeViewLevel, m_activeViewType );

    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( false, 1, 0, m_maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
                                        renderedViewType, m_activeViewLevel, m_activeViewType );

    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    return std::make_tuple( nullptr, nullptr, m_finalRenderTarget, nullptr, nullptr );
}

std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >
Renderer::renderText( const std::string& text, Font& font, float2 position, float4 color )
{
    m_deferredRenderer.render( text, font, position, color );

    return m_deferredRenderer.getAlbedoRenderTarget();
}

std::tuple< 
bool,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderMainImage( const Scene& scene, const Camera& camera, 
                           const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                           const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
                           const std::vector< bool >& activeViewLevel, const View activeViewType, 
                           const bool wireframeMode,
                           const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
                           const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
                           const std::vector< std::shared_ptr< Light > >& selectedLights )
{
    float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint(), camera.getPosition(), camera.getUp() );

    m_profiler.beginEvent( Profiler::GlobalEventType::DeferredRendering );

    // Render 'axises' model.
    if ( m_axisMesh )
        m_deferredRenderer.render( *m_axisMesh, float43::IDENTITY, viewMatrix );

    float4 actorSelectionEmissiveColor( 0.5f, 0.5f, 0.0f, 1.0f );

    // Render actors in the scene.
    const std::unordered_set< std::shared_ptr<Actor> >& actors = scene.getActors();
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) {
            const std::shared_ptr<BlockActor> blockActor = std::static_pointer_cast<BlockActor>(actor);
            const std::shared_ptr<BlockModel> blockModel = blockActor->getModel();

            const bool isSelected = std::find( selectedBlockActors.begin(), selectedBlockActors.end(), blockActor ) != selectedBlockActors.end();

            if ( blockModel->isInGpuMemory() )
                m_deferredRenderer.render( *blockModel, blockActor->getPose(), viewMatrix, isSelected ? actorSelectionEmissiveColor : float4::ZERO, wireframeMode );
            else if ( blockModel->getMesh() && blockModel->getMesh()->isInGpuMemory() )
                m_deferredRenderer.render( *blockModel->getMesh(), blockActor->getPose(), viewMatrix, wireframeMode );

        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const std::shared_ptr<SkeletonActor> skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
            const std::shared_ptr<SkeletonModel> skeletonModel = skeletonActor->getModel();

            const bool isSelected = std::find( selectedSkeletonActors.begin(), selectedSkeletonActors.end(), skeletonActor ) != selectedSkeletonActors.end();

            if ( skeletonModel->isInGpuMemory() )
                m_deferredRenderer.render( *skeletonModel, skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose(), isSelected ? actorSelectionEmissiveColor : float4::ZERO, wireframeMode );
            else if ( skeletonModel->getMesh() && skeletonModel->getMesh()->isInGpuMemory() )
                m_deferredRenderer.render( *skeletonModel->getMesh(), skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose(), wireframeMode );
        }
    }

    float4 lightSelectionEmissiveColor( 0.0f, 0.0f, 1.0f, 1.0f );
    float4 lightDisabledEmissiveColor( 0.0f, 1.0f, 0.0f, 1.0f );

    // Render light sources in the scene.
    if ( m_lightModel ) 
    {
        float43 lightPose( float43::IDENTITY );
        for ( const auto& light : scene.getLights() ) 
        {
            const bool isSelected = std::find( selectedLights.begin(), selectedLights.end(), light ) != selectedLights.end();

            lightPose.setTranslation( light->getPosition() );
            float4 extraEmissive = isSelected ? lightSelectionEmissiveColor : ( light->isEnabled() ? float4::ZERO : lightDisabledEmissiveColor );
            m_deferredRenderer.render( *m_lightModel, lightPose, viewMatrix, extraEmissive );
        }
    }

    m_profiler.endEvent( Profiler::GlobalEventType::DeferredRendering );

	// Gather block actors.
	std::vector< std::shared_ptr< const BlockActor > > blockActors;
	blockActors.reserve( actors.size() );
	for ( const std::shared_ptr< Actor > actor : actors )
	{
		if ( actor->getType() == Actor::Type::BlockActor )
			blockActors.push_back( std::static_pointer_cast<BlockActor>(actor) );
	}

    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

    // Generate mipmaps for normal and position g-buffers.
    m_deferredRenderer.getNormalRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );
    m_deferredRenderer.getPositionRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( m_deferredRenderer.getEmissiveRenderTarget() );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::EmissiveShading );

    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    m_shadingRenderer.performShadingNoShadows( camera, m_deferredRenderer.getPositionRenderTarget(),
                                               m_deferredRenderer.getAlbedoRenderTarget(), m_deferredRenderer.getMetalnessRenderTarget(),
                                               m_deferredRenderer.getRoughnessRenderTarget(), m_deferredRenderer.getNormalRenderTarget(), lightsNotCastingShadows );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    const int lightCount = (int)lightsCastingShadows.size();
	for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

		m_raytraceShadowRenderer.generateAndTraceShadowRays( lightsCastingShadows[ lightIdx ], m_deferredRenderer.getPositionRenderTarget(), m_deferredRenderer.getNormalRenderTarget(), nullptr, blockActors );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( camera, m_deferredRenderer.getPositionRenderTarget(),
			m_deferredRenderer.getAlbedoRenderTarget(), m_deferredRenderer.getMetalnessRenderTarget(),
			m_deferredRenderer.getRoughnessRenderTarget(), m_deferredRenderer.getNormalRenderTarget(), m_raytraceShadowRenderer.getIlluminationTexture(), *lightsCastingShadows[ lightIdx ] );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
	}

    m_profiler.beginEvent( Profiler::GlobalEventType::CopyFrameToFinalRenderTarget );

    // Copy main shaded image to final render target.
    m_rendererCore.copyTexture( m_finalRenderTarget, 0, m_shadingRenderer.getColorRenderTarget(), 0 );

    m_profiler.endEvent( Profiler::GlobalEventType::CopyFrameToFinalRenderTarget );

    if ( activeViewLevel.empty() )
    {
        switch ( activeViewType )
        {
            case View::Shaded: 
                return std::make_tuple( true, nullptr, nullptr, m_shadingRenderer.getColorRenderTarget(), nullptr, nullptr );
            case View::Depth: 
                return std::make_tuple( true, nullptr, m_deferredRenderer.getDepthRenderTarget(), nullptr, nullptr, nullptr );
            case View::Position: 
                return std::make_tuple( true, nullptr, nullptr, m_deferredRenderer.getPositionRenderTarget(), nullptr, nullptr );
            case View::Emissive: 
                return std::make_tuple( true, nullptr, m_deferredRenderer.getEmissiveRenderTarget(), nullptr, nullptr, nullptr );
            case View::Albedo: 
                return std::make_tuple( true, nullptr, m_deferredRenderer.getAlbedoRenderTarget(), nullptr, nullptr, nullptr );
            case View::Normal:
                return std::make_tuple( true, nullptr, nullptr, m_deferredRenderer.getNormalRenderTarget(), nullptr, nullptr );
            case View::Metalness:
                return std::make_tuple( true, m_deferredRenderer.getMetalnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::Roughness:
                return std::make_tuple( true, m_deferredRenderer.getRoughnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::IndexOfRefraction:
                return std::make_tuple( true, m_deferredRenderer.getIndexOfRefractionRenderTarget(), nullptr, nullptr, nullptr, nullptr );
			case View::Test:
				return std::make_tuple( true, m_raytraceShadowRenderer.getIlluminationTexture(), nullptr, nullptr, nullptr, nullptr );
        }
    }

    return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );
}

std::tuple<
bool,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderReflectionsRefractions( const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
                                        const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                        const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                        const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
                                        std::vector< bool >& renderedViewLevel,
                                        const std::vector< bool >& activeViewLevel,
                                        const View activeViewType )
{
    if ( level > maxLevelCount )
        return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );

    bool frameReceived = false;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    renderedViewLevel.push_back( reflectionFirst );

    if ( reflectionFirst ) 
    {
        //OutputDebugStringW( StringUtil::widen( "Reflection - " + std::to_string( level ) + "\n" ).c_str() );

        if ( level == 1 )
            renderFirstReflections( camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );
        else
            renderReflections( level, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );
    }
    else 
    {
        //OutputDebugStringW( StringUtil::widen( "Refraction - " + std::to_string( level ) + "\n" ).c_str() );

        if ( level == 1 )
            renderFirstRefractions( camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );
        else
            renderRefractions( level, refractionLevel, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );
    }

    if ( renderedViewLevel == activeViewLevel )
    {
        switch ( activeViewType )
        {
            case View::Shaded: 
                return std::make_tuple( true, nullptr, nullptr, m_shadingRenderer.getColorRenderTarget(), nullptr, nullptr );
            case View::Position: 
                return std::make_tuple( true, nullptr, nullptr, m_raytraceRenderer.getRayHitPositionTexture( level - 1 ), nullptr, nullptr );
            case View::Emissive: 
                return std::make_tuple( true, nullptr, m_raytraceRenderer.getRayHitEmissiveTexture( level - 1 ), nullptr, nullptr, nullptr );
            case View::Albedo: 
                return std::make_tuple( true, nullptr, m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ), nullptr, nullptr, nullptr );
            case View::Normal:
                return std::make_tuple( true, nullptr, nullptr, m_raytraceRenderer.getRayHitNormalTexture( level - 1 ), nullptr, nullptr );
            case View::Metalness:
                return std::make_tuple( true, m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::Roughness:
                return std::make_tuple( true, m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::IndexOfRefraction:
                return std::make_tuple( true, m_raytraceRenderer.getCurrentRefractiveIndexTextures().at( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::RayDirections: 
                return std::make_tuple( true, nullptr, nullptr, m_raytraceRenderer.getRayDirectionsTexture( level - 1 ), nullptr, nullptr );
            case View::Contribution: 
                return std::make_tuple( true, nullptr, m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), nullptr, nullptr, nullptr );
            case View::CurrentRefractiveIndex:
                return std::make_tuple( true, m_raytraceRenderer.getCurrentRefractiveIndexTextures().at( level - 1 ), nullptr, nullptr, nullptr, nullptr );
			case View::Test:
				return std::make_tuple( true, m_raytraceShadowRenderer.getIlluminationTexture(), nullptr, nullptr, nullptr, nullptr );
        }
    }

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( true, level + 1, refractionLevel + (int)(!reflectionFirst), maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, renderedViewLevel, activeViewLevel, activeViewType );

    if ( frameReceived )
        return std::make_tuple( true, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( false, level + 1, refractionLevel + (int)(!reflectionFirst), maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, renderedViewLevel, activeViewLevel, activeViewType );

    if ( frameReceived )
        return std::make_tuple( true, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    //OutputDebugStringW( StringUtil::widen( "\n" ).c_str() );

    renderedViewLevel.pop_back();

    return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );
}

void Renderer::renderFirstReflections( const Camera& camera, 
                                       const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                       const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                       const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    // Perform reflection shading.
    m_reflectionRefractionShadingRenderer.performFirstReflectionShading( camera, m_deferredRenderer.getPositionRenderTarget(), m_deferredRenderer.getNormalRenderTarget(), m_deferredRenderer.getAlbedoRenderTarget(),
                                                                  m_deferredRenderer.getMetalnessRenderTarget(), m_deferredRenderer.getRoughnessRenderTarget() );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ReflectionTransmissionShading );
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::Raytracing );

    // Perform ray tracing.
    m_raytraceRenderer.generateAndTraceFirstReflectedRays( camera, m_deferredRenderer.getPositionRenderTarget(), m_deferredRenderer.getNormalRenderTarget(),
                                                            m_deferredRenderer.getRoughnessRenderTarget(), m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), blockActors );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::Raytracing );
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( m_raytraceRenderer.getRayHitEmissiveTexture( 0 ) );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::EmissiveShading );

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ShadingNoShadows );

    m_shadingRenderer.performShadingNoShadows( m_raytraceRenderer.getRayOriginsTexture( 0 ), 
                                               m_raytraceRenderer.getRayHitPositionTexture( 0 ),
                                               m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
                                               m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
                                               m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
                                               m_raytraceRenderer.getRayHitNormalTexture( 0 ), 
                                               lightsNotCastingShadows );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ShadingNoShadows );

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{

        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

		m_raytraceShadowRenderer.generateAndTraceShadowRays( 
			lightsCastingShadows[ lightIdx ], 
			m_raytraceRenderer.getRayHitPositionTexture( 0 ), 
			m_raytraceRenderer.getRayHitNormalTexture( 0 ), 
			m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
			blockActors 
		);

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( 
			m_raytraceRenderer.getRayOriginsTexture( 0 ),
			m_raytraceRenderer.getRayHitPositionTexture( 0 ),
			m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
			m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
			m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
			m_raytraceRenderer.getRayHitNormalTexture( 0 ),
			m_raytraceShadowRenderer.getIlluminationTexture(),
			*lightsCastingShadows[ lightIdx ] 
		);

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
	}

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    // Generate mipmaps for the shaded, reflected image.
    m_shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    const int contributionTextureFillWidth  = m_deferredRenderer.getPositionRenderTarget()->getWidth();
    const int contributionTextureFillHeight = m_deferredRenderer.getPositionRenderTarget()->getHeight();

    const int colorTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( 0 )->getWidth();
    const int colorTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( 0 )->getHeight();

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::CombiningWithMainImage );

    // Combine main image with reflections.
    m_combiningRenderer.combine( m_finalRenderTarget, 
                                 m_shadingRenderer.getColorRenderTarget(), 
                                 m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                                 m_deferredRenderer.getNormalRenderTarget(), 
                                 m_deferredRenderer.getPositionRenderTarget(), 
                                 m_deferredRenderer.getDepthRenderTarget(), 
                                 m_raytraceRenderer.getRayHitDistanceTexture( 0 ),
                                 camera.getPosition(),
                                 contributionTextureFillWidth, contributionTextureFillHeight,
                                 colorTextureFillWidth, colorTextureFillHeight );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::CombiningWithMainImage );
}

void Renderer::renderFirstRefractions( const Camera& camera, 
                                       const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                       const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                       const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    // Perform refraction shading.
    m_reflectionRefractionShadingRenderer.performFirstRefractionShading( camera, 
                                                                       m_deferredRenderer.getPositionRenderTarget(), 
                                                                       m_deferredRenderer.getNormalRenderTarget(), 
                                                                       m_deferredRenderer.getAlbedoRenderTarget(),
                                                                       m_deferredRenderer.getMetalnessRenderTarget(), 
                                                                       m_deferredRenderer.getRoughnessRenderTarget() );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ReflectionTransmissionShading );
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::Raytracing );

    // Perform ray tracing.
    m_raytraceRenderer.generateAndTraceFirstRefractedRays( camera, 
                                                         m_deferredRenderer.getPositionRenderTarget(), 
                                                         m_deferredRenderer.getNormalRenderTarget(),
                                                         m_deferredRenderer.getRoughnessRenderTarget(), 
                                                         m_deferredRenderer.getIndexOfRefractionRenderTarget(),
                                                         m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                                                         blockActors );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::Raytracing );
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( m_raytraceRenderer.getRayHitEmissiveTexture( 0 ) );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::EmissiveShading );

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ShadingNoShadows );

    m_shadingRenderer.performShadingNoShadows( m_raytraceRenderer.getRayOriginsTexture( 0 ),
                                               m_raytraceRenderer.getRayHitPositionTexture( 0 ),
                                               m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
                                               m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
                                               m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
                                               m_raytraceRenderer.getRayHitNormalTexture( 0 ),
                                               lightsNotCastingShadows );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ShadingNoShadows );

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

		m_raytraceShadowRenderer.generateAndTraceShadowRays(
			lightsCastingShadows[ lightIdx ],
			m_raytraceRenderer.getRayHitPositionTexture( 0 ),
			m_raytraceRenderer.getRayHitNormalTexture( 0 ), 
			m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ),
			blockActors
		);

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading(
			m_raytraceRenderer.getRayOriginsTexture( 0 ),
			m_raytraceRenderer.getRayHitPositionTexture( 0 ),
			m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
			m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
			m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
			m_raytraceRenderer.getRayHitNormalTexture( 0 ),
			m_raytraceShadowRenderer.getIlluminationTexture(),
			*lightsCastingShadows[ lightIdx ]
		);

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
	}

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    // Generate mipmaps for the shaded, reflected image.
    m_shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    const int contributionTextureFillWidth  = m_deferredRenderer.getPositionRenderTarget()->getWidth();
    const int contributionTextureFillHeight = m_deferredRenderer.getPositionRenderTarget()->getHeight();

    const int colorTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( 0 )->getWidth();
    const int colorTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( 0 )->getHeight();

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::CombiningWithMainImage );

    // Combine main image with refractions.
    m_combiningRenderer.combine( m_finalRenderTarget, 
                               m_shadingRenderer.getColorRenderTarget(), 
                               m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                               m_deferredRenderer.getNormalRenderTarget(), 
                               m_deferredRenderer.getPositionRenderTarget(), 
                               m_deferredRenderer.getDepthRenderTarget(), 
                               m_raytraceRenderer.getRayHitDistanceTexture( 0 ),
                               camera.getPosition(),
                               contributionTextureFillWidth, contributionTextureFillHeight,
                               colorTextureFillWidth, colorTextureFillHeight );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::CombiningWithMainImage );
}

void Renderer::renderReflections( const int level, const Camera& camera, 
                                  const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                  const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                  const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{

    m_reflectionRefractionShadingRenderer.performReflectionShading( level - 1, 
                                                                  m_raytraceRenderer.getRayOriginsTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitAlbedoTexture( level - 2 ),
                                                                  m_raytraceRenderer.getRayHitMetalnessTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitRoughnessTexture( level - 2 ) );

    m_raytraceRenderer.generateAndTraceReflectedRays( level - 1,
                                                      m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                                                      blockActors );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( m_raytraceRenderer.getRayHitEmissiveTexture( level - 1 ) );

    m_shadingRenderer.performShadingNoShadows( m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
                                               lightsNotCastingShadows );

    for ( const std::shared_ptr< Light >& light : lightsCastingShadows ) 
    {
        m_raytraceShadowRenderer.generateAndTraceShadowRays(
            light,
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
            blockActors
            );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
            m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_raytraceShadowRenderer.getIlluminationTexture(),
            *light
            );
    }

    // Generate mipmaps for the shaded, reflected image.
    m_shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );

    const int contributionTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( level - 2 )->getWidth();
    const int contributionTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( level - 2 )->getHeight();

    const int colorTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( 0 )->getWidth();
    const int colorTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( 0 )->getHeight();

    m_combiningRenderer.combine( m_finalRenderTarget, 
                               m_shadingRenderer.getColorRenderTarget(), 
                               m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                               m_raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                               m_raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                               m_deferredRenderer.getDepthRenderTarget(), 
                               m_raytraceRenderer.getRayHitDistanceTexture( level - 1 ),
                               camera.getPosition(),
                               contributionTextureFillWidth, contributionTextureFillHeight,
                               colorTextureFillWidth, colorTextureFillHeight );
}

void Renderer::renderRefractions( const int level, const int refractionLevel, const Camera& camera, 
                                  const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                  const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                  const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    m_reflectionRefractionShadingRenderer.performRefractionShading( level - 1, 
                                                                  m_raytraceRenderer.getRayOriginsTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitAlbedoTexture( level - 2 ),
                                                                  m_raytraceRenderer.getRayHitMetalnessTexture( level - 2 ), 
                                                                  m_raytraceRenderer.getRayHitRoughnessTexture( level - 2 ) );

    m_raytraceRenderer.generateAndTraceRefractedRays( level - 1, refractionLevel,
                                                      m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                                                      blockActors );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( m_raytraceRenderer.getRayHitEmissiveTexture( level - 1 ) );

    m_shadingRenderer.performShadingNoShadows( m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
                                               m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
                                               lightsNotCastingShadows );

    for ( const std::shared_ptr< Light >& light : lightsCastingShadows ) 
    {
        m_raytraceShadowRenderer.generateAndTraceShadowRays(
            light,
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
            blockActors
            );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
            m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_raytraceShadowRenderer.getIlluminationTexture(),
            *light
            );
    }


    // Generate mipmaps for the shaded, reflected image.
    m_shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *m_deviceContext.Get() );

    const int contributionTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( level - 2 )->getWidth();
    const int contributionTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( level - 2 )->getHeight();

    const int colorTextureFillWidth  = m_raytraceRenderer.getRayOriginsTexture( 0 )->getWidth();
    const int colorTextureFillHeight = m_raytraceRenderer.getRayOriginsTexture( 0 )->getHeight();

    m_combiningRenderer.combine( m_finalRenderTarget, 
                               m_shadingRenderer.getColorRenderTarget(), 
                               m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
                               m_raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                               m_raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                               m_deferredRenderer.getDepthRenderTarget(), 
                               m_raytraceRenderer.getRayHitDistanceTexture( level - 1 ),
                               camera.getPosition(),
                               contributionTextureFillWidth, contributionTextureFillHeight,
                               colorTextureFillWidth, colorTextureFillHeight );
}

void Renderer::setActiveViewType( const View view )
{
    m_activeViewType = view;
}

Renderer::View Renderer::getActiveViewType() const
{
    return m_activeViewType;
}

void Renderer::activateNextViewLevel( const bool reflection )
{
    if ( m_activeViewLevel.size() < m_maxLevelCount )
        m_activeViewLevel.push_back( reflection );
}

void Renderer::activatePrevViewLevel()
{
    if ( !m_activeViewLevel.empty() )
        m_activeViewLevel.pop_back();
}

const std::vector< bool >& Renderer::getActiveViewLevel() const
{
    return m_activeViewLevel;
}

void Renderer::setMaxLevelCount( const int levelCount )
{
    m_maxLevelCount = std::max( 0, levelCount );

    if ( m_activeViewLevel.size() >= m_maxLevelCount )
        m_activeViewLevel.resize( m_maxLevelCount );
}

int Renderer::getMaxLevelCount() const
{
    return m_maxLevelCount;
}

void Renderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    m_finalRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& Renderer::debugGetCurrentRefractiveIndexTextures()
{
    return m_raytraceRenderer.getCurrentRefractiveIndexTextures();
}
