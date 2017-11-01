#include "Renderer.h"

#include <memory>

#include "Direct3DRendererCore.h"
#include "Profiler.h"
#include "RenderTargetManager.h"
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
#include "SpotLight.h"

#include "StagingTexture2D.h"

#include "StringUtil.h"
#include "SceneUtil.h"
#include "Selection.h"

#include "Settings.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Renderer::Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler, RenderTargetManager& renderTargetManager ) :
    m_rendererCore( rendererCore ),
    m_profiler( profiler ),
    m_renderTargetManager( renderTargetManager ),
    m_deferredRenderer( rendererCore ),
    m_raytraceRenderer( rendererCore ),
    m_shadingRenderer( rendererCore ),
    m_reflectionRefractionShadingRenderer( rendererCore ),
    m_edgeDetectionRenderer( rendererCore ),
    m_hitDistanceSearchRenderer( rendererCore ),
    m_combiningRenderer( rendererCore ),
    m_textureRescaleRenderer( rendererCore ),
    m_rasterizeShadowRenderer( rendererCore ),
	m_raytraceShadowRenderer( rendererCore ),
	m_shadowMapRenderer( rendererCore ),
    m_mipmapRenderer( rendererCore ),
    m_distanceToOccluderSearchRenderer( rendererCore ),
    m_blurShadowsRenderer( rendererCore ),
    m_utilityRenderer( rendererCore ),
    m_extractBrightPixelsRenderer( rendererCore ),
    m_toneMappingRenderer( rendererCore ),
    m_antialiasingRenderer( rendererCore ),
    m_activeViewType( View::Final ),
    m_exposure( 1.0f ),
    m_minBrightness( 1.0f )
{}

Renderer::~Renderer()
{}

void Renderer::initialize( 
    const int2 imageDimensions, 
    ComPtr< ID3D11Device3 > device, 
    ComPtr< ID3D11DeviceContext3 > deviceContext, 
    std::shared_ptr<const BlockModel> lightModel )
{
    m_device        = device;
    m_deviceContext = deviceContext;

    m_lightModel = lightModel;

    m_imageDimensions = imageDimensions;

	m_deferredRenderer.initialize( device, deviceContext );
    m_raytraceRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_shadingRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_reflectionRefractionShadingRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_edgeDetectionRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_hitDistanceSearchRenderer.initialize( device, deviceContext );
    m_combiningRenderer.initialize( device, deviceContext );
    m_textureRescaleRenderer.initialize( device, deviceContext );
    m_rasterizeShadowRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
	m_raytraceShadowRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
	m_shadowMapRenderer.initialize( device, deviceContext );
    m_mipmapRenderer.initialize( device, deviceContext );
    m_distanceToOccluderSearchRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_blurShadowsRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_utilityRenderer.initialize( device, deviceContext );
    m_extractBrightPixelsRenderer.initialize( device, deviceContext );
    m_toneMappingRenderer.initialize( device, deviceContext );
    m_antialiasingRenderer.initialize( device, deviceContext );

    createRenderTargets( imageDimensions.x, imageDimensions.y, *device.Get() );
}

// Should be called at the beginning of each frame, before calling renderScene(). 
void Renderer::clear()
{
    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    //m_deferredRenderer.clearRenderTargets( float4( 0.0f, 0.0f, 0.0f, 1.0f ), 1.0f ); 

}

void Renderer::clear2()
{
    // #TODO: Ugly - clearing some random render targets - find a more elegant way to clean render targets.
    m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions )->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions )->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
}

void Renderer::renderShadowMaps( const Scene& scene )
{
    for ( const auto& light : scene.getLights() )
    {
        if ( light->getType() != Light::Type::SpotLight )
            continue;

        SpotLight& spotLight = static_cast< SpotLight& >( *light );

        const float44 viewMatrix        = spotLight.getShadowMapViewMatrix();
        const float44 perspectiveMatrix = spotLight.getShadowMapProjectionMatrix();

        if ( spotLight.getShadowMap() )
            m_shadowMapRenderer.setRenderTarget( spotLight.getShadowMap() );
        else
            m_shadowMapRenderer.createAndSetRenderTarget( SpotLight::s_shadowMapDimensions, *m_device.Get() );

        m_shadowMapRenderer.clearRenderTarget( 1.0f ); //#TODO: What clear value?

        for ( const auto& actor : scene.getActors() )
        {
            if ( actor->getType() == Actor::Type::BlockActor )
            {
                const BlockActor& blockActor = static_cast< BlockActor& >( *actor );

                if ( !blockActor.getModel() || !blockActor.getModel()->getMesh() )
                    continue;

                m_shadowMapRenderer.render( *blockActor.getModel()->getMesh(), blockActor.getPose(), viewMatrix, perspectiveMatrix );
            }
            else if ( actor->getType() == Actor::Type::BlockActor ) 
            {
                const SkeletonActor& skeletonActor = static_cast< SkeletonActor& >( *actor );

                if ( !skeletonActor.getModel() || !skeletonActor.getModel()->getMesh() )
                    continue;

                m_shadowMapRenderer.render( *skeletonActor.getModel()->getMesh(), skeletonActor.getPose(), viewMatrix, perspectiveMatrix, skeletonActor.getSkeletonPose() );
            }
        }

        spotLight.setShadowMap( m_shadowMapRenderer.getRenderTarget() );
    }

    m_shadowMapRenderer.disableRenderTarget();
}

Renderer::Output Renderer::renderScene( 
    const Scene& scene, const Camera& camera,
    const bool wireframeMode,
    const Selection& selection,
    const std::shared_ptr< BlockMesh > selectionVolumeMesh )
{
    // Render shadow maps. #TODO: Should NOT be done every frame.
    //renderShadowMaps( scene );

    const auto& actors                  = scene.getActorsVec();
    const auto& lights                  = scene.getLightsVec();
    const auto  lightsEnabled           = SceneUtil::filterLightsByState( lights, true );
    const auto  lightsCastingShadows    = SceneUtil::filterLightsByShadowCasting( lightsEnabled, true );
    const auto  lightsNotCastingShadows = SceneUtil::filterLightsByShadowCasting( lightsEnabled, false );
    const auto  blockActors             = SceneUtil::filterActorsByType< BlockActor >( actors );

    m_layersRenderTargets.reserve( settings().rendering.reflectionsRefractions.maxLevel + 1 );

    Output output; 
    output = renderPrimaryLayer( 
        scene, camera, lightsCastingShadows, lightsNotCastingShadows, 
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType, wireframeMode,
        selection, selectionVolumeMesh 
    );

    // Release all temporary render targets.
    m_layersRenderTargets.clear();
    
    if ( !output.isEmpty() && m_activeViewType != Renderer::View::Final )
        return output;

    assert( output.float4Image );

    // Perform post-effects.
    performBloom( output.float4Image, m_minBrightness );

    if ( m_activeViewType == View::BloomBrightPixels ) {
        output.reset();
        output.float4Image = m_temporaryRenderTarget2;

        return output;
    }

    performToneMapping( output.float4Image, m_temporaryRenderTargetLDR, m_exposure );

    Output finalOutput;

    if ( settings().rendering.antialiasing )
    {
        performAntialiasing( m_temporaryRenderTargetLDR, m_finalRenderTargetLDR );
        finalOutput.uchar4Image = m_finalRenderTargetLDR;
    }
    else
    {
        finalOutput.uchar4Image = m_temporaryRenderTargetLDR;
    }

    return finalOutput;
}

Renderer::Output Renderer::renderText( 
    const std::string& text, 
    Font& font, 
    float2 position, 
    float4 color )
{
    Direct3DDeferredRenderer::RenderTargets defferedRenderTargets;
    defferedRenderTargets.albedo = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "text-albedo" );
    defferedRenderTargets.normal = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "text-normal" );

    Direct3DDeferredRenderer::Settings defferedSettings;
    defferedSettings.fieldOfView     = 0.0f; // Not used.
    defferedSettings.imageDimensions = (float2)m_imageDimensions;
    defferedSettings.wireframeMode   = false;
    defferedSettings.zNear           = 0.1f;
    defferedSettings.zFar            = 1000.0f;

    m_deferredRenderer.render( defferedRenderTargets, defferedSettings, text, font, position, color );

    Renderer::Output output;
    output.uchar4Image = defferedRenderTargets.albedo;

    return output;
}

Renderer::Output Renderer::renderPrimaryLayer( 
    const Scene& scene, const Camera& camera, 
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
    const std::vector< bool >& activeViewLevel, const View activeViewType,
    const bool wireframeMode,
    const Selection& selection,
    const std::shared_ptr< BlockMesh > selectionVolumeMesh )
{
    m_layersRenderTargets.emplace_back();
    auto& layerRenderTargets = m_layersRenderTargets.back();

    layerRenderTargets.hitPosition        = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitPosition" );
    layerRenderTargets.hitEmissive        = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "hitEmissive" );
    layerRenderTargets.hitAlbedo          = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "hitAlbedo" );
    layerRenderTargets.hitMetalness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitMetalness" );
    layerRenderTargets.hitRoughness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitRoughness" );
    layerRenderTargets.hitNormal          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitNormal" );
    layerRenderTargets.hitRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitRefractiveIndex" );
    layerRenderTargets.depth              = m_renderTargetManager.getRenderTargetDepth( m_imageDimensions, "depth" );
    layerRenderTargets.hitShaded          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitShaded" );

    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    layerRenderTargets.hitPosition->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitEmissive->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitAlbedo->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitMetalness->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitRoughness->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitNormal->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitRefractiveIndex->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.depth->clearDepthStencilView( *m_deviceContext.Get(), true, 1.0f, false, 0 );

    { // Render meshes using DeferredRenderer.
        Direct3DDeferredRenderer::RenderTargets defferedRenderTargets;
        defferedRenderTargets.position        = layerRenderTargets.hitPosition;
        defferedRenderTargets.emissive        = layerRenderTargets.hitEmissive;
        defferedRenderTargets.albedo          = layerRenderTargets.hitAlbedo;
        defferedRenderTargets.metalness       = layerRenderTargets.hitMetalness;
        defferedRenderTargets.roughness       = layerRenderTargets.hitRoughness;
        defferedRenderTargets.normal          = layerRenderTargets.hitNormal;
        defferedRenderTargets.refractiveIndex = layerRenderTargets.hitRefractiveIndex;
        defferedRenderTargets.depth           = layerRenderTargets.depth;

        Direct3DDeferredRenderer::Settings defferedSettings;
        defferedSettings.fieldOfView     = camera.getFieldOfView();
        defferedSettings.imageDimensions = (float2)m_imageDimensions;
        defferedSettings.wireframeMode   = wireframeMode;
        defferedSettings.zNear           = 0.1f;
        defferedSettings.zFar            = 1000.0f;

        float44 viewMatrix = MathUtil::lookAtTransformation( 
            camera.getLookAtPoint(), 
            camera.getPosition(), 
            camera.getUp() 
        );

        m_profiler.beginEvent( Profiler::GlobalEventType::DeferredRendering );

        float4 actorSelectionEmissiveColor( 0.1f, 0.1f, 0.0f, 1.0f );

        // Render actors in the scene.
        const std::unordered_set< std::shared_ptr<Actor> >& actors = scene.getActors();
        for ( const std::shared_ptr<Actor> actor : actors ) 
        {
            if ( actor->getType() == Actor::Type::BlockActor ) 
            {
                const auto& blockActor = std::static_pointer_cast<BlockActor>(actor);
                const auto& blockModel = blockActor->getModel();

                const bool isSelected = selection.contains( blockActor );

                if ( !blockModel->isInGpuMemory() )
                    continue;

                const float4 extraEmissive = isSelected ? actorSelectionEmissiveColor : float4::ZERO;
                
                m_deferredRenderer.render( 
                    defferedRenderTargets,
                    defferedSettings,
                    *blockModel, 
                    blockActor->getPose(), 
                    viewMatrix, 
                    extraEmissive
                );

            } 
            else if ( actor->getType() == Actor::Type::SkeletonActor ) 
            {
                const auto& skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
                const auto& skeletonModel = skeletonActor->getModel();

                const bool isSelected = selection.contains( skeletonActor );

                if ( skeletonModel->isInGpuMemory() )
                    continue;

                const float4 extraEmissive = isSelected ? actorSelectionEmissiveColor : float4::ZERO;

                m_deferredRenderer.render( 
                    defferedRenderTargets,
                    defferedSettings,
                    *skeletonModel, 
                    skeletonActor->getPose(), 
                    viewMatrix, 
                    skeletonActor->getSkeletonPose(), 
                    extraEmissive
                );
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
                const bool isSelected = selection.contains( light );

                lightPose.setTranslation( light->getPosition() );

                float4 extraEmissive = isSelected ?
                    lightSelectionEmissiveColor :
                    ( light->isEnabled() ? float4::ZERO : lightDisabledEmissiveColor );

                m_deferredRenderer.render( 
                    defferedRenderTargets,
                    defferedSettings, 
                    *m_lightModel, 
                    lightPose, 
                    viewMatrix, 
                    extraEmissive 
                );
            }
        }

        // Render selection volume.
        if ( selectionVolumeMesh )
        {
            m_deferredRenderer.renderEmissive( 
                defferedRenderTargets,
                defferedSettings, 
                *selectionVolumeMesh, 
                float43::IDENTITY, 
                viewMatrix
            );
        }
    }

    m_profiler.endEvent( Profiler::GlobalEventType::DeferredRendering );
    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

    // #TODO: Do we need these mipmaps? I don't think so anymore...
    // Generate mipmaps for normal and position g-buffers.
    layerRenderTargets.hitNormal->generateMipMapsOnGpu( *m_deviceContext.Get() );
    layerRenderTargets.hitPosition->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );
    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( layerRenderTargets.hitShaded, layerRenderTargets.hitEmissive );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::EmissiveShading );
    m_profiler.beginEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    m_shadingRenderer.performShadingNoShadows( 
        camera, 
        layerRenderTargets.hitShaded,
        layerRenderTargets.hitPosition,
        layerRenderTargets.hitAlbedo, 
        layerRenderTargets.hitMetalness,
        layerRenderTargets.hitRoughness, 
        layerRenderTargets.hitNormal, 
        lightsNotCastingShadows 
    );

    m_profiler.endEvent( Profiler::StageType::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    const auto blockActors = SceneUtil::filterActorsByType< BlockActor >( scene.getActorsVec() );

    auto finalDistanceToOccluderHardShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderMediumShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderSoftShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    const int lightCount = (int)lightsCastingShadows.size();
	for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

        auto hardShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hardShadow" );
        auto mediumShadowRenderTarget                        = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "mediumShadow" );
        auto softShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "softShadow" );
        auto distanceToOccluderHardShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccluderHardShadow" );
        auto distanceToOccluderMediumShadowRenderTarget      = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccluderMediumShadow" );
        auto distanceToOccluderSoftShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccludeSoftShadow" );
        auto finalDistanceToOccluderHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderHardShadowImageDimensions, "finalDistanceToOccluderHardShadow" );
        auto finalDistanceToOccluderMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderMediumShadowImageDimensions, "finalDistanceToOccluderMediumShadow" );
        auto finalDistanceToOccluderSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderSoftShadowImageDimensions, "finalDistanceToOccluderSoftShadow" );

        if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight 
             && std::static_pointer_cast< SpotLight >( lightsCastingShadows[ lightIdx ] )->getShadowMap() 
        )
        {
            //m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

            //m_rasterizeShadowRenderer.performShadowMapping(
            //    camera.getPosition(),
            //    lightsCastingShadows[ lightIdx ],
            //    m_deferredRenderer.getPositionRenderTarget(),
            //    m_deferredRenderer.getNormalRenderTarget()
            //);

            //m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
            //m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForPreillumination );

            ////#TODO: Should not generate all mipmaps. Maybe only two or three...
            //m_rasterizeShadowRenderer.getShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

            //m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForPreillumination );
        }

        // #TODO: Should be profiled.
        // Fill shadow texture with pre-shadow data.
        /*m_rendererCore.copyTexture( 
            *std::static_pointer_cast< Texture2DSpecUsage< TexUsage::Default, unsigned char > >( m_raytraceShadowRenderer.getHardShadowTexture() ), 0,
            *std::static_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >( m_rasterizeShadowRenderer.getShadowTexture() ), 0 
        );

        m_rendererCore.copyTexture(
            *std::static_pointer_cast< Texture2DSpecUsage< TexUsage::Default, unsigned char > >( m_raytraceShadowRenderer.getSoftShadowTexture() ), 0,
            *std::static_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >( m_rasterizeShadowRenderer.getShadowTexture() ), 0
        );*/

        //auto distanceToOccluder = m_rasterizeShadowRenderer.getDistanceToOccluder();

        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

        m_raytraceShadowRenderer.generateAndTraceShadowRays(
            camera,
            lightsCastingShadows[ lightIdx ],
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            nullptr,
            hardShadowRenderTarget,
            mediumShadowRenderTarget,
            softShadowRenderTarget,
            distanceToOccluderHardShadowRenderTarget,
            distanceToOccluderMediumShadowRenderTarget,
            distanceToOccluderSoftShadowRenderTarget,
            //m_rasterizeShadowRenderer.getShadowTexture(),
            blockActors
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );

        hardShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        mediumShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        softShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );

        m_mipmapRenderer.generateMipmapsWithSampleRejection( 
            distanceToOccluderHardShadowRenderTarget, 
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder,
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel
        );

        m_mipmapRenderer.generateMipmapsWithSampleRejection( 
            distanceToOccluderMediumShadowRenderTarget, 
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder, 
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel 
        );

        m_mipmapRenderer.generateMipmapsWithSampleRejection( 
            distanceToOccluderSoftShadowRenderTarget, 
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder,
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel 
        );
        
        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel,
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            distanceToOccluderHardShadowRenderTarget,
            finalDistanceToOccluderHardShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel,
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            distanceToOccluderMediumShadowRenderTarget,
            finalDistanceToOccluderMediumShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel,
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            distanceToOccluderSoftShadowRenderTarget,
            finalDistanceToOccluderSoftShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        auto blurredHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredHardShadow" );
        auto blurredMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredMediumShadow" );
        auto blurredSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredSoftShadow" );

        if ( settings().rendering.shadows.useSeparableShadowBlur )
        {
            auto blurredShadowTemporaryRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredShadowTemporary" );

            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                hardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                mediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                softShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );
        }
        else
        {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                hardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                mediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                softShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );
        }

        if ( activeViewLevel.empty() ) {
            Output output;

            switch ( activeViewType ) {
                case View::HardShadow:
                    output.ucharImage = hardShadowRenderTarget;
                    return output;
                case View::MediumShadow:
                    output.ucharImage = mediumShadowRenderTarget;
                    return output;
                case View::SoftShadow:
                    output.ucharImage = softShadowRenderTarget;
                    return output;
                case View::DistanceToOccluderHardShadow:
                    output.floatImage = distanceToOccluderHardShadowRenderTarget;
                    return output;
                case View::DistanceToOccluderMediumShadow:
                    output.floatImage = distanceToOccluderMediumShadowRenderTarget;
                    return output;
                case View::DistanceToOccluderSoftShadow:
                    output.floatImage = distanceToOccluderSoftShadowRenderTarget;
                    return output;
                case View::FinalDistanceToOccluderHardShadow:
                    output.floatImage = finalDistanceToOccluderHardShadowRenderTarget;
                    return output;
                case View::FinalDistanceToOccluderMediumShadow:
                    output.floatImage = finalDistanceToOccluderMediumShadowRenderTarget;
                    return output;
                case View::FinalDistanceToOccluderSoftShadow:
                    output.floatImage = finalDistanceToOccluderSoftShadowRenderTarget;
                    return output;
                case View::BlurredHardShadows:
                    output.ucharImage = blurredHardShadowRenderTarget;
                    return output;
                case View::BlurredMediumShadows:
                    output.ucharImage = blurredMediumShadowRenderTarget;
                    return output;
                case View::BlurredSoftShadows:
                    output.ucharImage = blurredSoftShadowRenderTarget;
                    return output;
            }
        }

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        // #TODO: Profile this stage.
        auto blurredShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredShadow" );

        m_utilityRenderer.sumValues(
            blurredShadowRenderTarget,
            blurredHardShadowRenderTarget, 
            blurredMediumShadowRenderTarget,
            blurredSoftShadowRenderTarget  
        );

        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( 
            camera, 
            layerRenderTargets.hitShaded,
            layerRenderTargets.hitPosition,
			layerRenderTargets.hitAlbedo, 
            layerRenderTargets.hitMetalness,
			layerRenderTargets.hitRoughness, 
            layerRenderTargets.hitNormal, 
            blurredShadowRenderTarget, 
            *lightsCastingShadows[ lightIdx ] 
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
	}

    layerRenderTargets.shadedCombined = layerRenderTargets.hitShaded;

    if ( activeViewLevel.empty() )
    {
        Output output = getLayerRenderTarget( activeViewType, 0 );

        if ( !output.isEmpty() )
            return output;
    }

    std::vector< bool > renderedViewType;

    Output output;

    output = renderSecondaryLayers(
        true, 1, 0,
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        renderedViewType,
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderSecondaryLayers(
        false, 1, 0,
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        renderedViewType,
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType
    );

    if ( !output.isEmpty() )
        return output;
    else
        output.float4Image = layerRenderTargets.shadedCombined;

    return output;
}

Renderer::Output Renderer::renderSecondaryLayers( 
    const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
    std::vector< bool >& renderedViewLevel,
    const std::vector< bool >& activeViewLevel,
    const View activeViewType )
{
    if ( level > maxLevelCount )
        return Output();

    if ( ( reflectionFirst && !settings().rendering.reflectionsRefractions.reflectionsEnabled ) ||
       ( !reflectionFirst && !settings().rendering.reflectionsRefractions.refractionsEnabled ) )
        return Output();

    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    renderedViewLevel.push_back( reflectionFirst );

    if ( reflectionFirst )
        renderSecondaryLayer( LayerType::Reflection, level, refractionLevel, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );
    else
        renderSecondaryLayer( LayerType::Refraction, level, refractionLevel, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );

    if ( renderedViewLevel == activeViewLevel )
    {
        Output output = getLayerRenderTarget( activeViewType, level );

        if ( !output.isEmpty() )
            return output;
    }

    //#TODO: We can clear some deferred render targets here if level 1 is finished.
    // Only has to keep depth.

    Output output;
    output = renderSecondaryLayers( 
        true, level + 1, refractionLevel + (int)(!reflectionFirst), 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        renderedViewLevel, activeViewLevel, activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderSecondaryLayers( 
        false, level + 1, refractionLevel + (int)(!reflectionFirst), 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        renderedViewLevel, activeViewLevel, activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    // Combine current layer with previous layer.
    combineLayers( level, camera );

    auto& currLayerRTs = m_layersRenderTargets.at( level );

    if ( activeViewLevel == renderedViewLevel && activeViewType == View::ShadedCombined )
        output.float4Image = currLayerRTs.shadedCombined;

    renderedViewLevel.pop_back();

    return output;
}

void Renderer::renderSecondaryLayer(
    const LayerType layerType, const int level, const int refractionLevel, const Camera& camera,
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    if ( m_layersRenderTargets.size() <= level )
        m_layersRenderTargets.emplace_back();

    auto& prevLayerRTs  = m_layersRenderTargets.at( level - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( level );     // Current layer render targets.

    const int2 hitDistSearchDimensions = m_imageDimensions / settings().rendering.hitDistanceSearch.resolutionDivider;

    currLayerRTs.contributionRoughness  = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "contributionRoughness" );
    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "rayOrigin" );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "rayDirection" );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitPosition" );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "hitEmissive" );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, "hitAlbedo" );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitMetalness" );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitRoughness" );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitNormal" );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hitRefractiveIndex" );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "currentRefractiveIndex" );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "hitDistance" );
    currLayerRTs.hitDistanceBlurred     = m_renderTargetManager.getRenderTarget< float >( hitDistSearchDimensions, "hitDistanceBlurred" );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "hitDistanceToCamera" );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, "hitShaded" );
    currLayerRTs.shadedCombined         = nullptr; // It's important to reset this - it may contain results from same layer reflection/refraction.

    if (layerType == Renderer::LayerType::Reflection)
    {
        if ( level <= 1 )
        {
            m_reflectionRefractionShadingRenderer.performFirstReflectionShading(
                camera,
                prevLayerRTs.hitPosition,
                prevLayerRTs.hitNormal,
                prevLayerRTs.hitAlbedo,
                prevLayerRTs.hitMetalness,
                prevLayerRTs.hitRoughness,
                currLayerRTs.contributionRoughness
            );
        }
        else
        {
            m_reflectionRefractionShadingRenderer.performReflectionShading(
                prevLayerRTs.rayOrigin,
                prevLayerRTs.hitPosition,
                prevLayerRTs.hitNormal,
                prevLayerRTs.hitAlbedo,
                prevLayerRTs.hitMetalness,
                prevLayerRTs.hitRoughness,
                prevLayerRTs.contributionRoughness,
                currLayerRTs.contributionRoughness
            );
        }
    }
    else if (layerType == Renderer::LayerType::Refraction)
    {
        if ( level <= 1 )
        {
            m_reflectionRefractionShadingRenderer.performFirstRefractionShading(
                camera,
                prevLayerRTs.hitPosition,
                prevLayerRTs.hitNormal,
                prevLayerRTs.hitAlbedo,
                prevLayerRTs.hitMetalness,
                prevLayerRTs.hitRoughness,
                currLayerRTs.contributionRoughness
            );
        }
        else
        {
            m_reflectionRefractionShadingRenderer.performRefractionShading(
                prevLayerRTs.rayOrigin,
                prevLayerRTs.hitPosition,
                prevLayerRTs.hitNormal,
                prevLayerRTs.hitAlbedo,
                prevLayerRTs.hitMetalness,
                prevLayerRTs.hitRoughness,
                prevLayerRTs.contributionRoughness,
                currLayerRTs.contributionRoughness
            );
        }
    }

    RaytraceRenderer::InputTextures2 raytracerInputs;
    raytracerInputs.contribution                   = currLayerRTs.contributionRoughness;
    raytracerInputs.prevHitPosition                = prevLayerRTs.hitPosition;
    raytracerInputs.prevHitNormal                  = prevLayerRTs.hitNormal;
    raytracerInputs.prevHitRoughness               = prevLayerRTs.hitRoughness;
    raytracerInputs.prevRayDirection               = prevLayerRTs.rayDirection;
    raytracerInputs.prevHitDistanceToCamera        = prevLayerRTs.hitDistanceToCamera;
    raytracerInputs.prevHitRefractiveIndex         = prevLayerRTs.hitRefractiveIndex;
    raytracerInputs.prevCurrentRefractiveIndex     = prevLayerRTs.currentRefractiveIndex;
    raytracerInputs.prevPrevCurrentRefractiveIndex = refractionLevel - 2 >= 0 ? m_layersRenderTargets.at( refractionLevel - 2 ).currentRefractiveIndex : nullptr;

    RaytraceRenderer::RenderTargets raytracerRTs;
    raytracerRTs.rayOrigin              = currLayerRTs.rayOrigin;
    raytracerRTs.rayDirection           = currLayerRTs.rayDirection;
    raytracerRTs.hitPosition            = currLayerRTs.hitPosition;
    raytracerRTs.hitEmissive            = currLayerRTs.hitEmissive;
    raytracerRTs.hitAlbedo              = currLayerRTs.hitAlbedo;
    raytracerRTs.hitMetalness           = currLayerRTs.hitMetalness;
    raytracerRTs.hitRoughness           = currLayerRTs.hitRoughness;
    raytracerRTs.hitNormal              = currLayerRTs.hitNormal;
    raytracerRTs.hitRefractiveIndex     = currLayerRTs.hitRefractiveIndex;
    raytracerRTs.currentRefractiveIndex = currLayerRTs.currentRefractiveIndex;
    raytracerRTs.hitDistance            = currLayerRTs.hitDistance;
    raytracerRTs.hitDistanceToCamera    = currLayerRTs.hitDistanceToCamera;

    if ( layerType == Renderer::LayerType::Reflection )
    {
        if ( level <= 1 )
        {
            m_raytraceRenderer.generateAndTraceFirstReflectedRays(
                camera,
                raytracerInputs,
                raytracerRTs,
                blockActors
            );
        }
        else
        {
            m_raytraceRenderer.generateAndTraceReflectedRays(
                raytracerInputs,
                raytracerRTs,
                blockActors
            );
        }
    }
    else if ( layerType == Renderer::LayerType::Refraction )
    {
        if ( level <= 1 )
        {
            m_raytraceRenderer.generateAndTraceFirstRefractedRays(
                camera,
                raytracerInputs,
                raytracerRTs,
                blockActors
            );
        }
        else
        {
            m_raytraceRenderer.generateAndTraceRefractedRays(
                refractionLevel,
                raytracerInputs,
                raytracerRTs,
                blockActors
            );
        }
    }

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( 
        currLayerRTs.hitShaded, 
        currLayerRTs.hitEmissive 
    );

    m_shadingRenderer.performShadingNoShadows(
        currLayerRTs.hitShaded,
        currLayerRTs.rayOrigin,
        currLayerRTs.hitPosition,
        currLayerRTs.hitAlbedo,
        currLayerRTs.hitMetalness,
        currLayerRTs.hitRoughness,
        currLayerRTs.hitNormal,
        lightsNotCastingShadows
    );

    auto finalDistanceToOccluderHardShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderMediumShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderSoftShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider;

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx ) 
    {
        auto hardShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "hardShadow" );
        auto mediumShadowRenderTarget                        = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "mediumShadow" );
        auto softShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "softShadow" );
        auto distanceToOccluderHardShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccluderHardShadow" );
        auto distanceToOccluderMediumShadowRenderTarget      = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccluderMediumShadow" );
        auto distanceToOccluderSoftShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, "distanceToOccluderSoftShadow" );
        auto finalDistanceToOccluderHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderHardShadowImageDimensions, "finalDistanceToOccluderHardShadow" );
        auto finalDistanceToOccluderMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderMediumShadowImageDimensions, "finalDistanceToOccluderMediumShadow" );
        auto finalDistanceToOccluderSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderSoftShadowImageDimensions, "finalDistanceToOccluderSoftShadow" );

        //if ( light->getType() == Light::Type::SpotLight
        //     && std::static_pointer_cast<SpotLight>( light )->getShadowMap()
        //) 
        //{
        //    m_rasterizeShadowRenderer.performShadowMapping(
        //        camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
        //        light,
        //        m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
        //        m_raytraceRenderer.getRayHitNormalTexture( level - 1 )
        //    );
        //}

        m_raytraceShadowRenderer.generateAndTraceShadowRays(
            camera,
            lightsCastingShadows[ lightIdx ],
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            currLayerRTs.contributionRoughness,
            hardShadowRenderTarget,
            mediumShadowRenderTarget,
            softShadowRenderTarget,
            distanceToOccluderHardShadowRenderTarget,
            distanceToOccluderMediumShadowRenderTarget,
            distanceToOccluderSoftShadowRenderTarget,
            blockActors
        );

        hardShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        mediumShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        softShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            distanceToOccluderHardShadowRenderTarget,
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder, 
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel
        );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            distanceToOccluderMediumShadowRenderTarget,
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder,
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel
        );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            distanceToOccluderSoftShadowRenderTarget,
            settings().rendering.shadows.distanceToOccluderSearch.maxDistToOccluder,
            0, 
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel
        );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            distanceToOccluderHardShadowRenderTarget,
            finalDistanceToOccluderHardShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            distanceToOccluderMediumShadowRenderTarget,
            finalDistanceToOccluderMediumShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            distanceToOccluderSoftShadowRenderTarget,
            finalDistanceToOccluderSoftShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );

        auto blurredHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredHardShadow" );
        auto blurredMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredMediumShadow" );
        auto blurredSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredSoftShadow" );
        
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) 
        {
            auto blurredShadowTemporaryRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );

            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                hardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                mediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                softShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

        } else {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                hardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                mediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                softShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );
        }

        auto blurredShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, "blurredShadow" );

        m_utilityRenderer.sumValues(
            blurredShadowRenderTarget,
            blurredHardShadowRenderTarget, 
            blurredMediumShadowRenderTarget,
            blurredSoftShadowRenderTarget  
        );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            currLayerRTs.hitShaded,
            currLayerRTs.rayOrigin,
            currLayerRTs.hitPosition,
            currLayerRTs.hitAlbedo,
            currLayerRTs.hitMetalness,
            currLayerRTs.hitRoughness,
            currLayerRTs.hitNormal,
            blurredShadowRenderTarget,
            *lightsCastingShadows[ lightIdx ]
        );
    }

    const int contributionTextureFillWidth = prevLayerRTs.hitAlbedo->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.hitAlbedo->getHeight();

    const int colorTextureFillWidth = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        currLayerRTs.hitDistance,
        currLayerRTs.hitDistanceBlurred
    );
}

void Renderer::combineLayers( const int currentLevel, const Camera& camera )
{
    if ( currentLevel < 1 || currentLevel >= m_layersRenderTargets.size() )
        throw std::exception( "Renderer::combineLayers - given level is 0 (nothing to combine then) or higher-equal to number of layers available." );

    auto& prevLayerRTs = m_layersRenderTargets.at( currentLevel - 1 ); // Previous layer render targets.
    auto& currLayerRTs = m_layersRenderTargets.at( currentLevel );     // Current layer render targets.

    if ( !currLayerRTs.shadedCombined ) 
    {
        currLayerRTs.shadedCombined = currLayerRTs.hitShaded;
        currLayerRTs.hitShaded = nullptr;
    }

    if ( !prevLayerRTs.shadedCombined ) 
    {
        prevLayerRTs.shadedCombined = prevLayerRTs.hitShaded;
        prevLayerRTs.hitShaded = nullptr;
    }

    // Generate mipmaps for the current layer shaded-combined image.
    currLayerRTs.shadedCombined->generateMipMapsOnGpu( *m_deviceContext.Get() );

    if ( currentLevel == 1 ) {
        m_combiningRenderer.combine(
            prevLayerRTs.shadedCombined,
            currLayerRTs.shadedCombined,
            currLayerRTs.contributionRoughness,
            prevLayerRTs.hitNormal,
            prevLayerRTs.hitPosition,
            prevLayerRTs.depth, // #TODO: Blurred or not?
            currLayerRTs.hitDistanceBlurred,
            camera.getPosition(),
            currLayerRTs.contributionRoughness->getWidth(),
            currLayerRTs.contributionRoughness->getHeight(),
            m_finalRenderTargetHDR->getWidth(),
            m_finalRenderTargetHDR->getHeight()
        );
    } else {
        m_combiningRenderer.combine(
            prevLayerRTs.shadedCombined,
            currLayerRTs.shadedCombined,
            currLayerRTs.contributionRoughness,
            prevLayerRTs.hitNormal,
            prevLayerRTs.hitPosition,
            prevLayerRTs.hitDistance, // #TODO: IMPORTANT: Blurred or not?
            currLayerRTs.hitDistanceBlurred,
            prevLayerRTs.rayOrigin,  //#TODO: IMPORTANT: previous or current??
            currLayerRTs.contributionRoughness->getWidth(),
            currLayerRTs.contributionRoughness->getHeight(),
            m_finalRenderTargetHDR->getWidth(),
            m_finalRenderTargetHDR->getHeight()
        );
    }
}

void Renderer::performBloom( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture, const float minBrightness )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::Bloom );

    m_extractBrightPixelsRenderer.extractBrightPixels( colorTexture, m_temporaryRenderTarget1, minBrightness );

    const int mipmapCount = m_temporaryRenderTarget1->getMipMapCountOnGpu();
    const int skipLastMipmapsCount = 6;
    const int maxMipmapLevel = std::max( 0, mipmapCount - skipLastMipmapsCount ); 

	for (int mipmapLevel = 0; mipmapLevel <= maxMipmapLevel; ++mipmapLevel)
	{
		m_utilityRenderer.blurValues( m_temporaryRenderTarget2, mipmapLevel, m_temporaryRenderTarget1, mipmapLevel );
		m_mipmapRenderer.resampleTexture( m_temporaryRenderTarget1, mipmapLevel + 1, m_temporaryRenderTarget2, mipmapLevel );
	}

    m_utilityRenderer.mergeMipmapsValues( m_finalRenderTargetHDR, m_temporaryRenderTarget2, 0, maxMipmapLevel );

    m_profiler.endEvent( Profiler::GlobalEventType::Bloom );
}

void Renderer::performToneMapping( 
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture, 
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture,
    const float exposure )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::ToneMapping );

    m_toneMappingRenderer.performToneMapping( srcTexture, dstTexture, exposure );

    m_profiler.endEvent( Profiler::GlobalEventType::ToneMapping );
}

void Renderer::performAntialiasing( 
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > srcTexture,
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::CalculateLuminance );

    m_antialiasingRenderer.calculateLuminance( srcTexture );

    m_profiler.endEvent( Profiler::GlobalEventType::CalculateLuminance );
    m_profiler.beginEvent( Profiler::GlobalEventType::Antialiasing );

    m_antialiasingRenderer.performAntialiasing( srcTexture, dstTexture );

    m_profiler.endEvent( Profiler::GlobalEventType::Antialiasing );
}

void Renderer::setActiveViewType( const View view )
{
    m_activeViewType = view;
}

Renderer::View Renderer::getActiveViewType() const
{
    return m_activeViewType;
}

float Renderer::getExposure()
{
    return m_exposure;
}

void Renderer::setExposure( const float exposure )
{
    m_exposure = std::max( -10.0f, std::min( 10.0f, exposure ) );
}

void Renderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device3& device )
{
    m_finalRenderTargetLDR = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_temporaryRenderTargetLDR = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_finalRenderTargetHDR = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_temporaryRenderTarget1 = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_temporaryRenderTarget2 = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > > Renderer::debugGetCurrentRefractiveIndexTextures()
{
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > > textures;

    for ( auto& layerRTs : m_layersRenderTargets )
        textures.push_back( layerRTs.currentRefractiveIndex );

    return textures;
}

Renderer::Output Renderer::getLayerRenderTarget( View view, int level )
{
    Output output;

    if ( level >= m_layersRenderTargets.size() )
        return output;

    switch ( view ) 
    {
        case View::Shaded:
            output.float4Image = m_layersRenderTargets.at( level ).hitShaded;
            break;
        case View::Depth:
            output.uchar4Image = m_layersRenderTargets.at( level ).depth;
            return output;
        case View::Position:
            output.float4Image = m_layersRenderTargets.at( level ).hitPosition;
            break;
        case View::Emissive:
            output.uchar4Image = m_layersRenderTargets.at( level ).hitEmissive;
            break;
        case View::Albedo:
            output.uchar4Image = m_layersRenderTargets.at( level ).hitAlbedo;
            break;
        case View::Normal:
            output.float4Image = m_layersRenderTargets.at( level ).hitNormal;
            break;
        case View::Metalness:
            output.ucharImage = m_layersRenderTargets.at( level ).hitMetalness;
            break;
        case View::Roughness:
            output.ucharImage = m_layersRenderTargets.at( level ).hitRoughness;
            break;
        case View::IndexOfRefraction:
            output.ucharImage = m_layersRenderTargets.at( level ).hitRefractiveIndex;
            break;
        case View::RayDirections:
            output.float4Image = m_layersRenderTargets.at( level ).rayDirection;
            break;
        case View::Contribution:
            output.uchar4Image = m_layersRenderTargets.at( level ).contributionRoughness;
            break;
        case View::CurrentRefractiveIndex:
            output.ucharImage = m_layersRenderTargets.at( level ).currentRefractiveIndex;
            break;
        case View::Preillumination:
            output.ucharImage = m_rasterizeShadowRenderer.getShadowTexture();
            break;
        case View::HitDistance:
            output.floatImage = m_layersRenderTargets.at( level ).hitDistance;
            break;
        case View::HitDistanceBlurred:
            output.floatImage = m_layersRenderTargets.at( level ).hitDistanceBlurred;
            break;
        case View::HitDistanceToCamera:
            output.floatImage = m_layersRenderTargets.at( level ).hitDistanceToCamera;
            break;
    }

    return output;
}

std::string Renderer::viewToString( const View view )
{
    switch ( view ) {
        case View::ShadedCombined:                           return "ShadedCombined";
        case View::Final:                                    return "Final";
        case View::Shaded:                                   return "Shaded";
        case View::Depth:                                    return "Depth";
        case View::Position:                                 return "Position";
        case View::Emissive:                                 return "Emissive";
        case View::Albedo:                                   return "Albedo";
        case View::Normal:                                   return "Normal";
        case View::Metalness:                                return "Metalness";
        case View::Roughness:                                return "Roughness";
        case View::IndexOfRefraction:                        return "IndexOfRefraction";
        case View::RayDirections:                            return "RayDirections";
        case View::Contribution:                             return "Contribution";
        case View::CurrentRefractiveIndex:                   return "CurrentRefractiveIndex";
        case View::Preillumination:                          return "Preillumination";
        case View::HardShadow:                               return "HardShadow";
        case View::MediumShadow:                             return "MediumShadow";
        case View::SoftShadow:                               return "SoftShadow";
        case View::BlurredHardShadows:                       return "BlurredHardShadows";
        case View::BlurredMediumShadows:                     return "BlurredMediumShadows";
        case View::BlurredSoftShadows:                       return "BlurredSoftShadows";
        case View::SpotlightDepth:                           return "SpotlightDepth";
        case View::DistanceToOccluderHardShadow:             return "DistanceToOccluderHardShadow";
        case View::DistanceToOccluderMediumShadow:           return "DistanceToOccluderMediumShadow";
        case View::DistanceToOccluderSoftShadow:             return "DistanceToOccluderSoftShadow";
        case View::FinalDistanceToOccluderHardShadow:        return "FinalDistanceToOccluderHardShadow";
        case View::FinalDistanceToOccluderMediumShadow:      return "FinalDistanceToOccluderMediumShadow";
        case View::FinalDistanceToOccluderSoftShadow:        return "FinalDistanceToOccluderSoftShadow";
        case View::BloomBrightPixels:                        return "BloomBrightPixels";
        case View::HitDistance:                              return "HitDistance";
        case View::HitDistanceBlurred:                       return "HitDistanceBlurred";
        case View::HitDistanceToCamera:                      return "HitDistanceToCamera";
        case View::Test:                                     return "Test";
    }

    return "";
}
