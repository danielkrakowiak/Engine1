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
    m_ASSAORenderer(),
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
    m_blurShadowPatternRenderer( rendererCore ),
    m_blurShadowsRenderer( rendererCore ),
    m_combineShadowLayersRenderer( rendererCore ),
    m_utilityRenderer( rendererCore ),
    m_bokehBlurRenderer( rendererCore ),
    m_extractBrightPixelsRenderer( rendererCore ),
    m_toneMappingRenderer( rendererCore ),
    m_antialiasingRenderer( rendererCore ),
    m_debugViewType( View::Final ),
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
    m_ASSAORenderer.initialize( device, deviceContext );
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
    m_blurShadowPatternRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_blurShadowsRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_combineShadowLayersRenderer.initialize( device, deviceContext );
    m_utilityRenderer.initialize( device, deviceContext );
    m_bokehBlurRenderer.initialize( device, deviceContext );
    m_extractBrightPixelsRenderer.initialize( device, deviceContext );
    m_toneMappingRenderer.initialize( device, deviceContext );
    m_antialiasingRenderer.initialize( device, deviceContext );
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

    const auto  lightsCastingShadows    = settings().rendering.shadows.enabled 
        ? SceneUtil::filterLightsByShadowCasting( lightsEnabled, true )
        : std::vector< std::shared_ptr< Light > >();

    const auto  lightsNotCastingShadows = settings().rendering.shadows.enabled
        ? SceneUtil::filterLightsByShadowCasting( lightsEnabled, false )
        : lightsEnabled;

    const auto  blockActors             = SceneUtil::filterActorsByType< BlockActor >( actors );

    m_layersRenderTargets.reserve( settings().rendering.reflectionsRefractions.maxLevel + 1 );

    Output output; 
    output = renderPrimaryLayer( 
        scene, camera, lightsCastingShadows, lightsNotCastingShadows, 
        settings().rendering.reflectionsRefractions.debugViewStage, m_debugViewType, wireframeMode,
        selection, selectionVolumeMesh 
    );

    // Release all temporary render targets.
    m_layersRenderTargets.clear();
    
    if ( !output.isEmpty() && m_debugViewType != Renderer::View::Final )
        return output;

    assert( output.float4Image );

    m_profiler.beginEvent( Profiler::GlobalEventType::PostProcess );

    decltype(output.float4Image) currentRenderTargetHDR = output.float4Image;
    output.float4Image.reset();

    if ( settings().rendering.postProcess.depthOfField.enabled ) 
    {
        auto renderTargetHDR = m_renderTargetManager.getRenderTarget< float4 >(
            m_imageDimensions, false, "renderTargetHDR" );

        m_bokehBlurRenderer.bokehBlur( renderTargetHDR, *currentRenderTargetHDR, *output.depthUchar4Image );
        currentRenderTargetHDR = renderTargetHDR;
    }

    // Perform post-effects.
    if ( settings().rendering.postProcess.bloom )
    {
        auto renderTargetHDR = m_renderTargetManager.getRenderTarget< float4 >(
            m_imageDimensions, false, "renderTargetHDR" );

        performBloom( renderTargetHDR, currentRenderTargetHDR, m_minBrightness );
        currentRenderTargetHDR = renderTargetHDR;

        if ( m_debugViewType == View::BloomBrightPixels ) 
        {
            output.reset();
            output.float4Image = currentRenderTargetHDR;

            return output;
        }
    }

    auto currentRenderTargetLDR = m_renderTargetManager.getRenderTarget< uchar4 >(
        m_imageDimensions, false, "renderTargetLDR" );

    performToneMapping( currentRenderTargetLDR, currentRenderTargetHDR, m_exposure );

    Output finalOutput;

    if ( settings().rendering.postProcess.antialiasing )
    {
        auto finalRenderTargetLDR = m_renderTargetManager.getRenderTarget< uchar4 >( 
            m_imageDimensions, false, "finalRenderTargetLDR" );

        performAntialiasing( finalRenderTargetLDR, currentRenderTargetLDR );
        finalOutput.uchar4Image = finalRenderTargetLDR;
    }
    else
    {
        finalOutput.uchar4Image = currentRenderTargetLDR;
    }

    m_profiler.endEvent( Profiler::GlobalEventType::PostProcess );

    return finalOutput;
}

void Renderer::renderText( 
    const std::string& text, 
    Font& font, 
    float2 position, 
    float4 color,
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > colorRenderTarget )
{
    Direct3DDeferredRenderer::RenderTargets defferedRenderTargets;
    defferedRenderTargets.albedo = colorRenderTarget;

    Direct3DDeferredRenderer::Settings defferedSettings;
    defferedSettings.fieldOfView     = 0.0f; // Not used.
    defferedSettings.imageDimensions = (float2)m_imageDimensions;
    defferedSettings.wireframeMode   = false;
    defferedSettings.zNear           = 0.1f;
    defferedSettings.zFar            = 1000.0f;

    m_deferredRenderer.render( defferedRenderTargets, defferedSettings, text, font, position, color );
}

Renderer::Output Renderer::renderPrimaryLayer( 
    const Scene& scene, const Camera& camera, 
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
    const RenderingStage debugViewStage, const View debugViewType,
    const bool wireframeMode,
    const Selection& selection,
    const std::shared_ptr< BlockMesh > selectionVolumeMesh )
{
    m_profiler.beginEvent( RenderingStage::Main, Profiler::EventTypePerStage::Total_WO_Combining );

    m_layersRenderTargets.emplace_back();
    auto& layerRenderTargets = m_layersRenderTargets.back();

    layerRenderTargets.hitPosition        = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, false, "hitPosition" );
    layerRenderTargets.hitEmissive        = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, false, "hitEmissive" );
    layerRenderTargets.hitAlbedo          = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, false, "hitAlbedo" );
    layerRenderTargets.hitMetalness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitMetalness" );
    layerRenderTargets.hitRoughness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitRoughness" );
    layerRenderTargets.hitNormal          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, settings().rendering.optimization.useHalfFloatsForNormals, "hitNormal" );
    layerRenderTargets.hitRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitRefractiveIndex" );
    layerRenderTargets.depth              = m_renderTargetManager.getRenderTargetDepth( m_imageDimensions, "depth" );
    layerRenderTargets.hitShaded          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, false, "hitShaded" );
    layerRenderTargets.ambientOcclusion   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "ambientOcclusion" );

    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    layerRenderTargets.hitPosition->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitEmissive->clearRenderTargetView( *m_deviceContext.Get(), float4( settings().rendering.skyColor, 0.0f ) );
    layerRenderTargets.hitAlbedo->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitMetalness->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitRoughness->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitNormal->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.hitRefractiveIndex->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
    layerRenderTargets.depth->clearDepthStencilView( *m_deviceContext.Get(), true, 1.0f, false, 0 );

    { // Render meshes using DeferredRenderer and render Ambient Occlusion.
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
        if ( m_lightModel && settings().debug.renderLightSources ) 
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

        m_rendererCore.disableRenderTargetViews();

        m_profiler.endEvent( Profiler::GlobalEventType::DeferredRendering );
        m_profiler.beginEvent( Profiler::GlobalEventType::ASSAO );

        if ( settings().rendering.ambientOcclusion.assao.enabled )
        {
            const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation(
                defferedSettings.fieldOfView,
                defferedSettings.imageDimensions.x / defferedSettings.imageDimensions.y,
                defferedSettings.zNear,
                defferedSettings.zFar
            );

            m_ASSAORenderer.renderAmbientOcclusion( 
                layerRenderTargets.ambientOcclusion, 
                layerRenderTargets.hitNormal, 
                layerRenderTargets.depth, 
                projectionMatrix,
                viewMatrix.getScaleOrientationTranslationInverse()
            );
        }

        m_profiler.endEvent( Profiler::GlobalEventType::ASSAO );
    }
    
    m_profiler.beginEvent( RenderingStage::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

    // Generate mipmaps for normal and position g-buffers.
    layerRenderTargets.hitNormal->generateMipMapsOnGpu( *m_deviceContext.Get() );
    layerRenderTargets.hitPosition->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( RenderingStage::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );
    m_profiler.beginEvent( RenderingStage::Main, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performEmissiveShading( layerRenderTargets.hitShaded, layerRenderTargets.hitEmissive );

    m_profiler.endEvent( RenderingStage::Main, Profiler::EventTypePerStage::EmissiveShading );
    m_profiler.beginEvent( RenderingStage::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    m_shadingRenderer.performShadingNoShadows( 
        camera, 
        layerRenderTargets.hitShaded,
        layerRenderTargets.hitPosition,
        layerRenderTargets.hitAlbedo, 
        layerRenderTargets.hitMetalness,
        layerRenderTargets.hitRoughness, 
        layerRenderTargets.hitNormal, 
        settings().rendering.ambientOcclusion.assao.enabled ? layerRenderTargets.ambientOcclusion : nullptr,
        lightsNotCastingShadows 
    );

    m_profiler.endEvent( RenderingStage::Main, Profiler::EventTypePerStage::ShadingNoShadows );

    const auto blockActors = SceneUtil::filterActorsByType< BlockActor >( scene.getActorsVec() );

    auto finalDistanceToOccluderHardShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderMediumShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderSoftShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider;

    const int lightCount = (int)lightsCastingShadows.size();
	for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

        auto hardShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hardShadow" );
        auto mediumShadowRenderTarget                        = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "mediumShadow" );
        auto softShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "softShadow" );
        auto distanceToOccluderHardShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccluderHardShadow" );
        auto distanceToOccluderMediumShadowRenderTarget      = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccluderMediumShadow" );
        auto distanceToOccluderSoftShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccludeSoftShadow" );
        auto finalDistanceToOccluderHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderHardShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderHardShadow" );
        auto finalDistanceToOccluderMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderMediumShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderMediumShadow" );
        auto finalDistanceToOccluderSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderSoftShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderSoftShadow" );

        if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight 
             && std::static_pointer_cast< SpotLight >( lightsCastingShadows[ lightIdx ] )->getShadowMap() 
        )
        {
            //m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

            //m_rasterizeShadowRenderer.performShadowMapping(
            //    camera.getPosition(),
            //    lightsCastingShadows[ lightIdx ],
            //    m_deferredRenderer.getPositionRenderTarget(),
            //    m_deferredRenderer.getNormalRenderTarget()
            //);

            //m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
            //m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForPreillumination );

            ////#TODO: Should not generate all mipmaps. Maybe only two or three...
            //m_rasterizeShadowRenderer.getShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

            //m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForPreillumination );
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

        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

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

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForShadows );

        hardShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        mediumShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        softShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForShadows );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForDistanceToOccluder );

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
        
        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForDistanceToOccluder );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold,
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
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold,
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
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold,
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

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadowPattern );

        auto smoothedPatternHardShadowRenderTarget   = (settings().rendering.shadows.enableBlurShadowPattern 
            ? m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "smoothedPatternHardShadow" )
            : hardShadowRenderTarget );

        auto smoothedPatternMediumShadowRenderTarget = (settings().rendering.shadows.enableBlurShadowPattern 
            ? m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "smoothedPatternMediumShadow" )
            : mediumShadowRenderTarget);

        auto smoothedPatternSoftShadowRenderTarget   = (settings().rendering.shadows.enableBlurShadowPattern 
            ? m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "smoothedPatternSoftShadow" )
            : softShadowRenderTarget);

        if ( settings().rendering.shadows.enableBlurShadowPattern )
        {
            if ( settings().rendering.shadows.useSeparableShadowPatternBlur )
            {
                auto smoothedPatternShadowTemporaryRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "smoothedPatternShadowTemporary" );

                // Blur shadow pattern in two passes - horizontal and vertical.
                m_blurShadowPatternRenderer.blurShadowPatternHorzVert(
                    camera,
                    settings().rendering.shadows.blurPattern.hardShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.hardShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    hardShadowRenderTarget,
                    distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderHardShadowRenderTarget,
                    smoothedPatternHardShadowRenderTarget,
                    smoothedPatternShadowTemporaryRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );

                m_blurShadowPatternRenderer.blurShadowPatternHorzVert(
                    camera,
                    settings().rendering.shadows.blurPattern.mediumShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.mediumShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    mediumShadowRenderTarget,
                    distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderMediumShadowRenderTarget,
                    smoothedPatternMediumShadowRenderTarget,
                    smoothedPatternShadowTemporaryRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );

                m_blurShadowPatternRenderer.blurShadowPatternHorzVert(
                    camera,
                    settings().rendering.shadows.blurPattern.softShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.softShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    softShadowRenderTarget,
                    distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderSoftShadowRenderTarget,
                    smoothedPatternSoftShadowRenderTarget,
                    smoothedPatternShadowTemporaryRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );
            }
            else
            {
                // Blur shadow pattern in a single pass.
                m_blurShadowPatternRenderer.blurShadowPattern(
                    camera,
                    settings().rendering.shadows.blurPattern.hardShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.hardShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    hardShadowRenderTarget,
                    distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderHardShadowRenderTarget,
                    smoothedPatternHardShadowRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );

                m_blurShadowPatternRenderer.blurShadowPattern(
                    camera,
                    settings().rendering.shadows.blurPattern.mediumShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.mediumShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    mediumShadowRenderTarget,
                    distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderMediumShadowRenderTarget,
                    smoothedPatternMediumShadowRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );

                m_blurShadowPatternRenderer.blurShadowPattern(
                    camera,
                    settings().rendering.shadows.blurPattern.softShadows.positionThreshold,
                    settings().rendering.shadows.blurPattern.softShadows.normalThreshold,
                    layerRenderTargets.hitPosition,
                    layerRenderTargets.hitNormal,
                    softShadowRenderTarget,
                    distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                    finalDistanceToOccluderSoftShadowRenderTarget,
                    smoothedPatternSoftShadowRenderTarget,
                    *lightsCastingShadows[ lightIdx ]
                );
            }
        }


        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadowPattern );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForSmoothedShadowPattern );

        smoothedPatternHardShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        smoothedPatternMediumShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        smoothedPatternSoftShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForSmoothedShadowPattern );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        auto blurredHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredHardShadow" );
        auto blurredMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredMediumShadow" );
        auto blurredSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredSoftShadow" );

        if ( settings().rendering.shadows.useSeparableShadowBlur )
        {
            auto blurredShadowTemporaryRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredShadowTemporary" );

            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                settings().rendering.shadows.blur.hardShadows.positionThreshold,
                settings().rendering.shadows.blur.hardShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternHardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                settings().rendering.shadows.blur.mediumShadows.positionThreshold,
                settings().rendering.shadows.blur.mediumShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternMediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                blurredShadowTemporaryRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                settings().rendering.shadows.blur.softShadows.positionThreshold,
                settings().rendering.shadows.blur.softShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternSoftShadowRenderTarget,
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
                settings().rendering.shadows.blur.hardShadows.positionThreshold,
                settings().rendering.shadows.blur.hardShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternHardShadowRenderTarget,
                distanceToOccluderHardShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderHardShadowRenderTarget,
                blurredHardShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                settings().rendering.shadows.blur.mediumShadows.positionThreshold,
                settings().rendering.shadows.blur.mediumShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternMediumShadowRenderTarget,
                distanceToOccluderMediumShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderMediumShadowRenderTarget,
                blurredMediumShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );

            m_blurShadowsRenderer.blurShadows(
                camera,
                settings().rendering.shadows.blur.softShadows.positionThreshold,
                settings().rendering.shadows.blur.softShadows.normalThreshold,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                smoothedPatternSoftShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::CombineShadowLayers );

        auto blurredShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredShadow" );

        m_combineShadowLayersRenderer.combineShadowLayers(
            blurredShadowRenderTarget,
            *blurredHardShadowRenderTarget, 
            *blurredMediumShadowRenderTarget,
            *blurredSoftShadowRenderTarget  
        );

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::CombineShadowLayers );

        if ( debugViewStage == RenderingStage::Main ) {
            Output output;

            switch ( debugViewType ) {
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
                case View::SmoothedPatternHardShadows:
                    output.ucharImage = smoothedPatternHardShadowRenderTarget;
                    return output;
                case View::SmoothedPatternMediumShadows:
                    output.ucharImage = smoothedPatternMediumShadowRenderTarget;
                    return output;
                case View::SmoothedPatternSoftShadows:
                    output.ucharImage = smoothedPatternSoftShadowRenderTarget;
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
                case View::BlurredShadows:
                    output.ucharImage = blurredShadowRenderTarget;
                    return output;
                case View::AmbientOcclusion:
                    output.ucharImage = layerRenderTargets.ambientOcclusion;
                    return output;
            }
        }

        m_profiler.beginEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

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

        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
        m_profiler.endEvent( RenderingStage::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
	}

    layerRenderTargets.shadedCombined = layerRenderTargets.hitShaded;

    m_profiler.endEvent( RenderingStage::Main, Profiler::EventTypePerStage::Total_WO_Combining );

    if ( debugViewStage == RenderingStage::Main )
    {
        Output output = getLayerRenderTarget( debugViewType, 0 );

        if ( !output.isEmpty() )
            return output;
    }

    std::vector< bool > renderedViewType;

    Output output;

    output = renderSecondaryLayers(
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        RenderingStage::R,
        settings().rendering.reflectionsRefractions.debugViewStage, m_debugViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderSecondaryLayers(
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        RenderingStage::T,
        settings().rendering.reflectionsRefractions.debugViewStage, m_debugViewType
    );

    if ( !output.isEmpty() )
        return output;
    else
        output.float4Image = layerRenderTargets.shadedCombined;

    output.depthUchar4Image = layerRenderTargets.depth;

    return output;
}

Renderer::Output Renderer::renderSecondaryLayers( 
    const int maxLevelCount, const Camera& camera,
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
    const RenderingStage renderingStage,
    const RenderingStage debugViewStage,
    const View debugViewType )
{
    const auto renderingStageType  = getLastRenderingStageType(renderingStage);
    const auto renderingStageLevel = getRenderingStageLevel( renderingStage );

    if ( renderingStageLevel > maxLevelCount )
        return Output();

    if ( ( renderingStageType == RenderingStageType::Reflection && !settings().rendering.reflectionsRefractions.reflectionsEnabled ) ||
       ( renderingStageType == RenderingStageType::Transmission && !settings().rendering.reflectionsRefractions.refractionsEnabled ) )
        return Output();

    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    renderSecondaryLayer( renderingStage, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows );

    if ( renderingStage == debugViewStage )
    {
        Output output = getLayerRenderTarget( debugViewType, renderingStageLevel );

        if ( !output.isEmpty() )
            return output;
    }

    //#TODO: We can clear some deferred render targets here if level 1 is finished.
    // Only has to keep depth.

    Output output;
    output = renderSecondaryLayers( 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        getNextRenderingStage (renderingStage, RenderingStageType::Reflection ), 
        debugViewStage, debugViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderSecondaryLayers( 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        getNextRenderingStage( renderingStage, RenderingStageType::Transmission ), 
        debugViewStage, debugViewType
    );

    if ( !output.isEmpty() )
        return output;

    // Combine current layer with previous layer.
    combineLayers( renderingStage, camera );

    auto& currLayerRTs = m_layersRenderTargets.at( renderingStageLevel );
    auto& prevLayerRTs = m_layersRenderTargets.at( renderingStageLevel - 1 );

    if ( debugViewStage == renderingStage && debugViewType == View::Shaded )
        output.float4Image = currLayerRTs.shadedCombined;
    else if ( debugViewStage == renderingStage && debugViewType == View::ShadedCombined )
        output.float4Image = prevLayerRTs.shadedCombined;

    return output;
}

void Renderer::renderSecondaryLayer(
    const RenderingStage renderingStage, const Camera& camera,
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::Total_WO_Combining );

    const auto renderingStageType                 = getLastRenderingStageType( renderingStage );
    const auto renderingStageLevel                = getRenderingStageLevel( renderingStage );
    const auto renderingStageRefractionLevelCount = getRenderingStageRefractionLevelCount( renderingStage );

    if ( m_layersRenderTargets.size() <= renderingStageLevel )
        m_layersRenderTargets.emplace_back();

    auto& prevLayerRTs  = m_layersRenderTargets.at( renderingStageLevel - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( renderingStageLevel );     // Current layer render targets.

    const int2 hitDistSearchDimensions = m_imageDimensions / settings().rendering.hitDistanceSearch.resolutionDivider;

    currLayerRTs.contributionRoughness  = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, false, "contributionRoughness" );
    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, false, "rayOrigin" );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, settings().rendering.optimization.useHalfFloatsForRayDirections, "rayDirection" );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, false, "hitPosition" );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, false, "hitEmissive" );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions, false, "hitAlbedo" );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitMetalness" );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitRoughness" );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, settings().rendering.optimization.useHalfFloatsForNormals, "hitNormal" );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hitRefractiveIndex" );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "currentRefractiveIndex" );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFloatsForHitDistance, "hitDistance" );
    currLayerRTs.hitDistanceBlurred     = m_renderTargetManager.getRenderTarget< float >( hitDistSearchDimensions, settings().rendering.optimization.useHalfFloatsForHitDistance, "hitDistanceBlurred" );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, false, "hitDistanceToCamera" );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions, false, "hitShaded" );
    currLayerRTs.shadedCombined         = nullptr; // It's important to reset this - it may contain results from same layer reflection/refraction.

    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    if (renderingStageType == RenderingStageType::Reflection)
    {
        if ( renderingStageLevel <= 1 )
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
    else if (renderingStageType == RenderingStageType::Transmission)
    {
        if ( renderingStageLevel <= 1 )
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

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    RaytraceRenderer::InputTextures2 raytracerInputs;
    raytracerInputs.contribution                   = currLayerRTs.contributionRoughness;
    raytracerInputs.prevHitPosition                = prevLayerRTs.hitPosition;
    raytracerInputs.prevHitNormal                  = prevLayerRTs.hitNormal;
    raytracerInputs.prevHitRoughness               = prevLayerRTs.hitRoughness;
    raytracerInputs.prevRayDirection               = prevLayerRTs.rayDirection;
    raytracerInputs.prevHitDistanceToCamera        = prevLayerRTs.hitDistanceToCamera;
    raytracerInputs.prevHitRefractiveIndex         = prevLayerRTs.hitRefractiveIndex;
    raytracerInputs.prevCurrentRefractiveIndex     = prevLayerRTs.currentRefractiveIndex;
    raytracerInputs.prevPrevCurrentRefractiveIndex = (renderingStageRefractionLevelCount - 2 >= 0 
        ? m_layersRenderTargets.at( renderingStageRefractionLevelCount - 2 ).currentRefractiveIndex 
        : nullptr);

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

    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::RaytracingReflectedRefractedRays );

    if ( renderingStageType == RenderingStageType::Reflection )
    {
        if ( renderingStageLevel <= 1 )
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
    else if ( renderingStageType == RenderingStageType::Transmission )
    {
        if ( renderingStageLevel <= 1 )
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
                renderingStageRefractionLevelCount,
                raytracerInputs,
                raytracerRTs,
                blockActors
            );
        }
    }

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::RaytracingReflectedRefractedRays );
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

    // Generate mipmaps for normal and position g-buffers.
    currLayerRTs.hitNormal->generateMipMapsOnGpu( *m_deviceContext.Get() );
    currLayerRTs.hitPosition->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performEmissiveShading( 
        currLayerRTs.hitShaded, 
        currLayerRTs.hitEmissive 
    );

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::EmissiveShading );
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::ShadingNoShadows );

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

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::ShadingNoShadows );

    auto finalDistanceToOccluderHardShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderMediumShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    auto finalDistanceToOccluderSoftShadowImageDimensions = 
        m_imageDimensions / settings().rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider;

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx ) 
    {
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

        auto hardShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "hardShadow" );
        auto mediumShadowRenderTarget                        = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "mediumShadow" );
        auto softShadowRenderTarget                          = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "softShadow" );
        auto distanceToOccluderHardShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccluderHardShadow" );
        auto distanceToOccluderMediumShadowRenderTarget      = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccluderMediumShadow" );
        auto distanceToOccluderSoftShadowRenderTarget        = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "distanceToOccluderSoftShadow" );
        auto finalDistanceToOccluderHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderHardShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderHardShadow" );
        auto finalDistanceToOccluderMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderMediumShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderMediumShadow" );
        auto finalDistanceToOccluderSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< float >( finalDistanceToOccluderSoftShadowImageDimensions, settings().rendering.optimization.useHalfFLoatsForDistanceToOccluder, "finalDistanceToOccluderSoftShadow" );

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

        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

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

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForShadows );

        hardShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        mediumShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );
        softShadowRenderTarget->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForShadows );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForDistanceToOccluder );

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

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForDistanceToOccluder );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold,
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
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold,
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
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold,
            settings().rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold,
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

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        auto blurredHardShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredHardShadow" );
        auto blurredMediumShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredMediumShadow" );
        auto blurredSoftShadowRenderTarget   = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredSoftShadow" );
        
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) 
        {
            auto blurredShadowTemporaryRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false );

            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                settings().rendering.shadows.blur.hardShadows.positionThreshold,
                settings().rendering.shadows.blur.hardShadows.normalThreshold,
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
                settings().rendering.shadows.blur.mediumShadows.positionThreshold,
                settings().rendering.shadows.blur.mediumShadows.normalThreshold,
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
                settings().rendering.shadows.blur.softShadows.positionThreshold,
                settings().rendering.shadows.blur.softShadows.normalThreshold,
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
                settings().rendering.shadows.blur.hardShadows.positionThreshold,
                settings().rendering.shadows.blur.hardShadows.normalThreshold,
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
                settings().rendering.shadows.blur.mediumShadows.positionThreshold,
                settings().rendering.shadows.blur.mediumShadows.normalThreshold,
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
                settings().rendering.shadows.blur.softShadows.positionThreshold,
                settings().rendering.shadows.blur.softShadows.normalThreshold,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                softShadowRenderTarget,
                distanceToOccluderSoftShadowRenderTarget, //#TODO: Not needed anymore?
                finalDistanceToOccluderSoftShadowRenderTarget,
                blurredSoftShadowRenderTarget,
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::CombineShadowLayers );

        auto blurredShadowRenderTarget = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions, false, "blurredShadow" );

        m_combineShadowLayersRenderer.combineShadowLayers(
            blurredShadowRenderTarget,
            *blurredHardShadowRenderTarget, 
            *blurredMediumShadowRenderTarget,
            *blurredSoftShadowRenderTarget  
        );

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::CombineShadowLayers );
        m_profiler.beginEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

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

        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
        m_profiler.endEvent( renderingStage, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
    }

    const int contributionTextureFillWidth = prevLayerRTs.hitAlbedo->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.hitAlbedo->getHeight();

    const int colorTextureFillWidth = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForHitDistance );

    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForHitDistance );
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::HitDistanceSearch );

    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        currLayerRTs.hitDistance,
        currLayerRTs.hitDistanceBlurred
    );

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::HitDistanceSearch );
    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::Total_WO_Combining );
}

void Renderer::combineLayers( const RenderingStage renderingStage, const Camera& camera )
{
    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::CombiningWithPreviousLayer );
    const int currentLevel = getRenderingStageLevel( renderingStage );

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

    m_profiler.beginEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForShadedLayer );

    // Generate mipmaps for the current layer shaded-combined image.
    currLayerRTs.shadedCombined->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::MipmapGenerationForShadedLayer );

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
            m_imageDimensions.x,
            m_imageDimensions.y
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
            m_imageDimensions.x,
            m_imageDimensions.y
        );
    }

    m_profiler.endEvent( renderingStage, Profiler::EventTypePerStage::CombiningWithPreviousLayer );
}

void Renderer::performBloom( 
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > destTexture, 
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture,
    const float minBrightness )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::Bloom );

    auto tempRenderTarget1 = m_renderTargetManager.getRenderTarget< float4 >(
        m_imageDimensions, false, "tempRenderTarget1" );

    auto tempRenderTarget2 = m_renderTargetManager.getRenderTarget< float4 >(
        m_imageDimensions, false, "tempRenderTarget2" );

    m_extractBrightPixelsRenderer.extractBrightPixels( colorTexture, tempRenderTarget1, minBrightness );

    const int mipmapCount = tempRenderTarget1->getMipMapCountOnGpu();
    const int skipLastMipmapsCount = 6;
    const int maxMipmapLevel = std::max( 0, mipmapCount - skipLastMipmapsCount ); 

	for (int mipmapLevel = 0; mipmapLevel <= maxMipmapLevel; ++mipmapLevel)
	{
		m_utilityRenderer.blurValues( tempRenderTarget2, mipmapLevel, tempRenderTarget1, mipmapLevel );
		m_mipmapRenderer.resampleTexture( tempRenderTarget1, mipmapLevel + 1, tempRenderTarget2, mipmapLevel );
	}

    m_utilityRenderer.mergeMipmapsValues( destTexture, tempRenderTarget2, 0, maxMipmapLevel );

    m_profiler.endEvent( Profiler::GlobalEventType::Bloom );
}

void Renderer::performToneMapping( 
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture,
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture, 
    const float exposure )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::ToneMapping );

    m_toneMappingRenderer.performToneMapping( srcTexture, dstTexture, exposure );

    m_profiler.endEvent( Profiler::GlobalEventType::ToneMapping );
}

void Renderer::performAntialiasing( 
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture,
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > srcTexture )
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
    m_debugViewType = view;
}

Renderer::View Renderer::getActiveViewType() const
{
    return m_debugViewType;
}

float Renderer::getExposure()
{
    return m_exposure;
}

void Renderer::setExposure( const float exposure )
{
    m_exposure = std::max( -10.0f, std::min( 10.0f, exposure ) );
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
            //output.uchar4Image = m_layersRenderTargets.at( level ).depth;
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
        case View::Final:                                    return "Final";
        case View::ShadedCombined:                           return "ShadedCombined";
        case View::Shaded:                                   return "Shaded";
        case View::Depth:                                    return "Depth";
        case View::Position:                                 return "Position";
        case View::Emissive:                                 return "Emissive";
        case View::Albedo:                                   return "Albedo";
        case View::Normal:                                   return "Normal";
        case View::Metalness:                                return "Metalness";
        case View::Roughness:                                return "Roughness";
        case View::IndexOfRefraction:                        return "IndexOfRefraction";
        case View::AmbientOcclusion:                         return "AmbientOcclusion";
        case View::RayDirections:                            return "RayDirections";
        case View::Contribution:                             return "Contribution";
        case View::CurrentRefractiveIndex:                   return "CurrentRefractiveIndex";
        case View::Preillumination:                          return "Preillumination";
        case View::HardShadow:                               return "HardShadow";
        case View::MediumShadow:                             return "MediumShadow";
        case View::SoftShadow:                               return "SoftShadow";
        case View::SmoothedPatternHardShadows:               return "SmoothedPatternHardShadows";
        case View::SmoothedPatternMediumShadows:             return "SmoothedPatternMediumShadows";
        case View::SmoothedPatternSoftShadows:               return "SmoothedPatternSoftShadows";
        case View::BlurredHardShadows:                       return "BlurredHardShadows";
        case View::BlurredMediumShadows:                     return "BlurredMediumShadows";
        case View::BlurredSoftShadows:                       return "BlurredSoftShadows";
        case View::BlurredShadows:                           return "BlurredShadows";
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
