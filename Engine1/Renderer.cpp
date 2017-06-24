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

void Renderer::initialize( const int2 imageDimensions, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext, 
                         std::shared_ptr<const BlockMesh> axisMesh, std::shared_ptr<const BlockModel> lightModel )
{
    m_device        = device;
    m_deviceContext = deviceContext;

    m_axisMesh   = axisMesh;
    m_lightModel = lightModel;

    m_imageDimensions = imageDimensions;

	m_deferredRenderer.initialize( device, deviceContext );
    m_raytraceRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_shadingRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_reflectionRefractionShadingRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_edgeDetectionRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
    m_hitDistanceSearchRenderer.initialize( imageDimensions.x, imageDimensions.y, device, deviceContext );
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

    Output output; 
    output = renderSceneImage( 
        scene, camera, lightsCastingShadows, lightsNotCastingShadows, 
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType, wireframeMode,
        selection, selectionVolumeMesh 
    );

    // Release all temporary render targets.
    m_layersRenderTargets.clear();
    
    if ( !output.isEmpty() )
        return output;

    // Perform post-effects.
    performBloom( m_finalRenderTargetHDR, m_minBrightness );

    if ( m_activeViewType == View::BloomBrightPixels ) {
        output.reset();
        output.float4Image = m_temporaryRenderTarget2;

        return output;
    }

    performToneMapping( m_finalRenderTargetHDR, m_temporaryRenderTargetLDR, m_exposure );

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
    defferedRenderTargets.albedo = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    defferedRenderTargets.normal = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

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

Renderer::Output Renderer::renderSceneImage( 
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

    layerRenderTargets.hitPosition        = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    layerRenderTargets.hitEmissive        = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    layerRenderTargets.hitAlbedo          = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    layerRenderTargets.hitMetalness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    layerRenderTargets.hitRoughness       = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    layerRenderTargets.hitNormal          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    layerRenderTargets.hitRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    layerRenderTargets.depth              = m_renderTargetManager.getRenderTargetDepth( m_imageDimensions );
    layerRenderTargets.hitShaded          = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

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

        // Render 'axises' model.
        if ( m_axisMesh )
        {
            m_deferredRenderer.render( 
                defferedRenderTargets, 
                defferedSettings, 
                *m_axisMesh, 
                float43::IDENTITY, 
                viewMatrix 
            );
        }

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

    // #TODO: Remove.
    // FOR DEBUG - Not needed normally, because whole texture gets overwritten.
    m_blurShadowsRenderer.getShadowTexture()->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO, 0 );

    const int lightCount = (int)lightsCastingShadows.size();
	for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );

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
            lightsCastingShadows[ lightIdx ],
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            nullptr,
            //m_rasterizeShadowRenderer.getShadowTexture(),
            blockActors
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );

        m_mipmapRenderer.generateMipmapsWithSampleRejection( 
            m_raytraceShadowRenderer.getDistanceToOccluder(), 
            500.0f, 0, 3 
        );
        
        if ( activeViewLevel.empty() ) 
        {
            switch ( activeViewType ) {
                case View::DistanceToOccluder:
                {
                    // #TODO: Remove this - only for debug.
                    // Note: Not needed - only to improve visualization of debug blur-radius.
                    //m_utilityRenderer.replaceValues( illuminationMinBlurRadiusTextureInWorldSpace, 0, 500.0f, 0.0f );
                    //m_utilityRenderer.replaceValues( illuminationMinBlurRadiusTextureInWorldSpace, 1, 500.0f, 0.0f );
                    //m_utilityRenderer.replaceValues( illuminationMinBlurRadiusTextureInWorldSpace, 2, 500.0f, 0.0f );

                    Output output;
                    output.floatImage = m_raytraceShadowRenderer.getDistanceToOccluder();

                    return output;
                }
            }
        }

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            layerRenderTargets.hitPosition,
            layerRenderTargets.hitNormal,
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            *lightsCastingShadows[ lightIdx ]
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        if ( settings().rendering.shadows.useSeparableShadowBlur )
        {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                layerRenderTargets.hitPosition,
                layerRenderTargets.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
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
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
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
            m_blurShadowsRenderer.getShadowTexture(), 
            *lightsCastingShadows[ lightIdx ] 
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
	}

    m_profiler.beginEvent( Profiler::GlobalEventType::CopyFrameToFinalRenderTarget );

    // Copy main shaded image to final render target.
    // Note: I have to explicitly cast textures, because otherwise the compiler fails to deduce the template parameters for the method.
    m_rendererCore.copyTexture( 
        *std::static_pointer_cast< Texture2DSpecUsage< TexUsage::Default, float4 > >( m_finalRenderTargetHDR ), 0, 
        *std::static_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, float4 > >( layerRenderTargets.hitShaded ), 0 
    );

    m_profiler.endEvent( Profiler::GlobalEventType::CopyFrameToFinalRenderTarget );

    Output output;

    if ( activeViewLevel.empty() )
    {
        switch ( activeViewType )
        {
            case View::Shaded:
                output.float4Image = layerRenderTargets.hitShaded;
                return output;
            case View::Depth: 
                output.uchar4Image = layerRenderTargets.depth; 
                return output;
            case View::Position: 
                output.float4Image = layerRenderTargets.hitPosition;
                return output;
            case View::Emissive: 
                output.uchar4Image = layerRenderTargets.hitEmissive;
                return output;
            case View::Albedo: 
                output.uchar4Image = layerRenderTargets.hitAlbedo;
                return output;
            case View::Normal:
                output.float4Image = layerRenderTargets.hitNormal;
                return output;
            case View::Metalness:
                output.ucharImage = layerRenderTargets.hitMetalness;
                return output;
            case View::Roughness:
                output.ucharImage = layerRenderTargets.hitRoughness;
                return output;
            case View::IndexOfRefraction:
                output.ucharImage = layerRenderTargets.hitRefractiveIndex;
                return output;
			case View::Preillumination:
				output.ucharImage = m_rasterizeShadowRenderer.getShadowTexture();
                return output;
            case View::HardIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getHardShadowTexture();
                return output;
            case View::SoftIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getSoftShadowTexture();
                return output;
            case View::BlurredIllumination:
                output.ucharImage = m_blurShadowsRenderer.getShadowTexture();
                return output;
            case View::SpotlightDepth:
                if ( !lightsCastingShadows.empty() && lightsCastingShadows[0]->getType() == Light::Type::SpotLight )
                    output.floatImage = std::static_pointer_cast< SpotLight >( lightsCastingShadows[ 0 ] )->getShadowMap();
                return output;
            case View::DistanceToOccluder:
                output.floatImage = m_rasterizeShadowRenderer.getDistanceToOccluder();
                return output;
            case View::FinalDistanceToOccluder:
                output.floatImage = m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture();
                return output;
        }
    }

    std::vector< bool > renderedViewType;

    output = renderReflectionsRefractions(
        true, 1, 0,
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        renderedViewType,
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderReflectionsRefractions(
        false, 1, 0,
        settings().rendering.reflectionsRefractions.maxLevel,
        camera, blockActors, lightsCastingShadows, lightsNotCastingShadows,
        renderedViewType,
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType
    );

    return output;
}

Renderer::Output Renderer::renderReflectionsRefractions( 
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
        Output output;

        switch ( activeViewType )
        {
            case View::Shaded: 
                output.float4Image = m_layersRenderTargets.at( level ).hitShaded;
                break;
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
                output.uchar4Image = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 );
                break;
            case View::CurrentRefractiveIndex:
                output.ucharImage = m_layersRenderTargets.at( level ).currentRefractiveIndex;
                break;
            case View::Preillumination:
                output.ucharImage = m_rasterizeShadowRenderer.getShadowTexture();
                break;
            case View::HardIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getHardShadowTexture();
                break;
            case View::HitDistance:
                output.floatImage = m_layersRenderTargets.at( level ).hitDistance;
                break;
            case View::FinalHitDistance:
                output.floatImage = m_layersRenderTargets.at( level ).hitDistanceBlurred;
                break;
            case View::HitDistanceToCamera:
                output.floatImage = m_layersRenderTargets.at( level ).hitDistanceToCamera;
                break;
        }

        return output;
    }

    //#TODO: We can clear some deferred render targets here if level 1 is finished.
    // Only has to keep depth.

    Output output;
    output = renderReflectionsRefractions( 
        true, level + 1, refractionLevel + (int)(!reflectionFirst), 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        renderedViewLevel, activeViewLevel, activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    output = renderReflectionsRefractions( 
        false, level + 1, refractionLevel + (int)(!reflectionFirst), 
        maxLevelCount, camera, blockActors, lightsCastingShadows, lightsNotCastingShadows, 
        renderedViewLevel, activeViewLevel, activeViewType
    );

    if ( !output.isEmpty() )
        return output;

    renderedViewLevel.pop_back();

    return Output();
}

void Renderer::renderFirstReflections( const Camera& camera, 
                                       const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
                                       const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                       const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    const int currLevel = 1;

    if ( m_layersRenderTargets.size() <= currLevel )
        m_layersRenderTargets.emplace_back();
    
    auto& firstLayerRTs = m_layersRenderTargets.front();
    auto& prevLayerRTs  = m_layersRenderTargets.at( currLevel - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( currLevel );     // Current layer render targets.

    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    //currLayerRTs.hitDistanceBlurred     = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    // Perform reflection shading.
    m_reflectionRefractionShadingRenderer.performFirstReflectionShading( 
        camera, 
        prevLayerRTs.hitPosition, 
        prevLayerRTs.hitNormal, 
        prevLayerRTs.hitAlbedo,
        prevLayerRTs.hitMetalness, 
        prevLayerRTs.hitRoughness 
    );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ReflectionTransmissionShading );
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::Raytracing );

    RaytraceRenderer::InputTextures1 raytracerInputs;
    raytracerInputs.contribution     = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 );
    raytracerInputs.prevHitPosition  = prevLayerRTs.hitPosition;
    raytracerInputs.prevHitNormal    = prevLayerRTs.hitNormal;
    raytracerInputs.prevHitRoughness = prevLayerRTs.hitRoughness;

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

    // Perform ray tracing.
    m_raytraceRenderer.generateAndTraceFirstReflectedRays( 
        camera, 
        raytracerInputs,
        raytracerRTs,
        blockActors 
    );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::Raytracing );
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( currLayerRTs.hitShaded, currLayerRTs.hitEmissive );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::EmissiveShading );
    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ShadingNoShadows );

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

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::ShadingNoShadows );

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

        //if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight
        //     && std::static_pointer_cast<SpotLight>( lightsCastingShadows[ lightIdx ] )->getShadowMap()
        //) 
        //{
        //    m_rasterizeShadowRenderer.performShadowMapping(
        //        camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
        //        lightsCastingShadows[ lightIdx ],
        //        m_raytraceRenderer.getRayHitPositionTexture( 0 ),
        //        m_raytraceRenderer.getRayHitNormalTexture( 0 )
        //    );
        //}

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

		m_raytraceShadowRenderer.generateAndTraceShadowRays( 
			lightsCastingShadows[ lightIdx ], 
			currLayerRTs.hitPosition, 
			currLayerRTs.hitNormal, 
			m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
            //nullptr, //#TODO: Should I use a pre-illumination?
			blockActors 
		);

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            500.0f, 0, 3
        );

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            *lightsCastingShadows[ lightIdx ]
        );

        // For debug display.
        currLayerRTs.hitDistanceBlurred = m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture();

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) 
        {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        } else {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( 
            currLayerRTs.hitShaded,
			currLayerRTs.rayOrigin,
			currLayerRTs.hitPosition,
			currLayerRTs.hitAlbedo,
			currLayerRTs.hitMetalness,
			currLayerRTs.hitRoughness,
			currLayerRTs.hitNormal,
			m_blurShadowsRenderer.getShadowTexture(),
			*lightsCastingShadows[ lightIdx ] 
		);

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
        
	}

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    // Generate mipmaps for the shaded, reflected image.
    currLayerRTs.hitShaded->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    const int contributionTextureFillWidth  = prevLayerRTs.hitPosition->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.hitPosition->getHeight();

    const int colorTextureFillWidth  = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    m_profiler.beginEvent( Profiler::StageType::R, Profiler::EventTypePerStage::CombiningWithMainImage );

    //#TODO: Should be profiled separately.
    
    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    // Search for hit distance.
    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera, 
        firstLayerRTs.hitPosition, 
        firstLayerRTs.hitNormal, 
        currLayerRTs.hitDistance 
    );

    // Combine main image with reflections.
    m_combiningRenderer.combine( 
        m_finalRenderTargetHDR, 
        currLayerRTs.hitShaded, 
        m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
        firstLayerRTs.hitNormal, 
        firstLayerRTs.hitPosition, 
        firstLayerRTs.depth, 
        m_hitDistanceSearchRenderer.getFinalHitDistanceTexture(),
        camera.getPosition(),
        contributionTextureFillWidth, 
        contributionTextureFillHeight,
        colorTextureFillWidth, 
        colorTextureFillHeight 
    );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::CombiningWithMainImage );
}

void Renderer::renderFirstRefractions( const Camera& camera, 
                                       const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
                                       const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                       const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    const int currLevel = 1;

    if ( m_layersRenderTargets.size() <= currLevel )
        m_layersRenderTargets.emplace_back();

    auto& firstLayerRTs = m_layersRenderTargets.front();
    auto& prevLayerRTs  = m_layersRenderTargets.at( currLevel - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( currLevel );     // Current layer render targets.

    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ReflectionTransmissionShading );

    // Perform refraction shading.
    m_reflectionRefractionShadingRenderer.performFirstRefractionShading( 
        camera, 
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        prevLayerRTs.hitAlbedo,
        prevLayerRTs.hitMetalness,
        prevLayerRTs.hitRoughness 
    );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ReflectionTransmissionShading );
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::Raytracing );

    RaytraceRenderer::InputTextures1 raytracerInputs;
    raytracerInputs.contribution           = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 );
    raytracerInputs.prevHitPosition        = prevLayerRTs.hitPosition;
    raytracerInputs.prevHitNormal          = prevLayerRTs.hitNormal;
    raytracerInputs.prevHitRoughness       = prevLayerRTs.hitRoughness;
    raytracerInputs.prevHitRefractiveIndex = prevLayerRTs.hitRefractiveIndex;

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

    // Perform ray tracing.
    m_raytraceRenderer.generateAndTraceFirstRefractedRays( 
        camera, 
        raytracerInputs,
        raytracerRTs,
        blockActors 
    );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::Raytracing );
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::EmissiveShading );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( currLayerRTs.hitShaded, currLayerRTs.hitEmissive );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::EmissiveShading );
    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ShadingNoShadows );

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

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::ShadingNoShadows );

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
	{
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

        //if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight
        //     && std::static_pointer_cast<SpotLight>( lightsCastingShadows[ lightIdx ] )->getShadowMap()
        //) 
        //{
        //    m_rasterizeShadowRenderer.performShadowMapping(
        //        camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
        //        lightsCastingShadows[ lightIdx ],
        //        m_raytraceRenderer.getRayHitPositionTexture( 0 ),
        //        m_raytraceRenderer.getRayHitNormalTexture( 0 )
        //    );
        //}

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

		m_raytraceShadowRenderer.generateAndTraceShadowRays(
			lightsCastingShadows[ lightIdx ],
			currLayerRTs.hitPosition,
			currLayerRTs.hitNormal, 
			m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ),
            //nullptr, //#TODO: Should I use a pre-illumination?
			blockActors
		);

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            500.0f, 0, 3
        );

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            *lightsCastingShadows[ lightIdx ]
        );

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        } else {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            currLayerRTs.hitShaded,
            currLayerRTs.rayOrigin,
            currLayerRTs.hitPosition,
            currLayerRTs.hitAlbedo,
            currLayerRTs.hitMetalness,
            currLayerRTs.hitRoughness,
            currLayerRTs.hitNormal,
            m_blurShadowsRenderer.getShadowTexture(),
            *lightsCastingShadows[ lightIdx ]
        );

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
	}

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    // Generate mipmaps for the shaded, reflected image.
    currLayerRTs.hitShaded->generateMipMapsOnGpu( *m_deviceContext.Get() );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::MipmapGenerationForShadedImage );

    const int contributionTextureFillWidth  = prevLayerRTs.hitPosition->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.hitPosition->getHeight();

    const int colorTextureFillWidth  = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    m_profiler.beginEvent( Profiler::StageType::T, Profiler::EventTypePerStage::CombiningWithMainImage );

    //#TODO: Should be profiled separately.

    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    // Search for hit distance.
    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        currLayerRTs.hitDistance
    );

    // Combine main image with reflections.
    m_combiningRenderer.combine(
        m_finalRenderTargetHDR,
        currLayerRTs.hitShaded,
        m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ),
        prevLayerRTs.hitNormal,
        prevLayerRTs.hitPosition,
        prevLayerRTs.depth,
        m_hitDistanceSearchRenderer.getFinalHitDistanceTexture(),
        camera.getPosition(),
        contributionTextureFillWidth,
        contributionTextureFillHeight,
        colorTextureFillWidth,
        colorTextureFillHeight
    );

    m_profiler.endEvent( Profiler::StageType::T, Profiler::EventTypePerStage::CombiningWithMainImage );
}

void Renderer::renderReflections( 
    const int level, const Camera& camera, 
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    if ( m_layersRenderTargets.size() <= level )
        m_layersRenderTargets.emplace_back();

    auto& firstLayerRTs = m_layersRenderTargets.front();
    auto& prevLayerRTs  = m_layersRenderTargets.at( level - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( level );     // Current layer render targets.

    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

    m_reflectionRefractionShadingRenderer.performReflectionShading( 
        level - 1, 
        prevLayerRTs.rayOrigin,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        prevLayerRTs.hitAlbedo,
        prevLayerRTs.hitMetalness,
        prevLayerRTs.hitRoughness 
    );

    RaytraceRenderer::InputTextures2 raytracerInputs;
    raytracerInputs.contribution                   = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 );
    raytracerInputs.prevHitPosition                = prevLayerRTs.hitPosition;
    raytracerInputs.prevHitNormal                  = prevLayerRTs.hitNormal;
    raytracerInputs.prevHitRoughness               = prevLayerRTs.hitRoughness;
    raytracerInputs.prevRayDirection               = prevLayerRTs.rayDirection;
    raytracerInputs.prevHitDistanceToCamera        = prevLayerRTs.hitDistanceToCamera;
    raytracerInputs.prevHitRefractiveIndex         = prevLayerRTs.hitRefractiveIndex;

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

    m_raytraceRenderer.generateAndTraceReflectedRays( 
        level - 1,
        raytracerInputs,
        raytracerRTs,
        blockActors 
    );

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

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx ) 
    {
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
            lightsCastingShadows[ lightIdx ],
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
            //nullptr, //#TODO: Should I use a pre-illumination?
            blockActors
        );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            500.0f, 0, 3
        );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            *lightsCastingShadows[ lightIdx ]
        );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        } else {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            currLayerRTs.hitShaded,
            currLayerRTs.rayOrigin,
            currLayerRTs.hitPosition,
            currLayerRTs.hitAlbedo,
            currLayerRTs.hitMetalness,
            currLayerRTs.hitRoughness,
            currLayerRTs.hitNormal,
            m_blurShadowsRenderer.getShadowTexture(),
            *lightsCastingShadows[ lightIdx ] 
            );
    }

    // Generate mipmaps for the shaded, reflected image.
    currLayerRTs.hitShaded->generateMipMapsOnGpu( *m_deviceContext.Get() );

    const int contributionTextureFillWidth  = prevLayerRTs.rayOrigin->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.rayOrigin->getHeight();

    const int colorTextureFillWidth  = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    // Search for hit distance.
    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        currLayerRTs.hitDistance
    );

    m_combiningRenderer.combine( 
        m_finalRenderTargetHDR, 
        currLayerRTs.hitShaded,
        m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
        prevLayerRTs.hitNormal,
        prevLayerRTs.hitPosition,
        firstLayerRTs.depth,
        m_hitDistanceSearchRenderer.getFinalHitDistanceTexture(),
        camera.getPosition(),
        contributionTextureFillWidth, contributionTextureFillHeight,
        colorTextureFillWidth, colorTextureFillHeight 
    );
}

void Renderer::renderRefractions( 
    const int level, const int refractionLevel, const Camera& camera, 
    const std::vector< std::shared_ptr< BlockActor > >& blockActors,
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows )
{
    if ( m_layersRenderTargets.size() <= level )
        m_layersRenderTargets.emplace_back();

    auto& firstLayerRTs = m_layersRenderTargets.front();
    auto& prevLayerRTs  = m_layersRenderTargets.at( level - 1 ); // Previous layer render targets.
    auto& currLayerRTs  = m_layersRenderTargets.at( level );     // Current layer render targets.

    currLayerRTs.rayOrigin              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.rayDirection           = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitPosition            = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitEmissive            = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitAlbedo              = m_renderTargetManager.getRenderTarget< uchar4 >( m_imageDimensions );
    currLayerRTs.hitMetalness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitRoughness           = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitNormal              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );
    currLayerRTs.hitRefractiveIndex     = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.currentRefractiveIndex = m_renderTargetManager.getRenderTarget< unsigned char >( m_imageDimensions );
    currLayerRTs.hitDistance            = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitDistanceToCamera    = m_renderTargetManager.getRenderTarget< float >( m_imageDimensions );
    currLayerRTs.hitShaded              = m_renderTargetManager.getRenderTarget< float4 >( m_imageDimensions );

    m_reflectionRefractionShadingRenderer.performRefractionShading( 
        level - 1, 
        prevLayerRTs.rayOrigin,
        prevLayerRTs.hitPosition,
        prevLayerRTs.hitNormal,
        prevLayerRTs.hitAlbedo,
        prevLayerRTs.hitMetalness,
        prevLayerRTs.hitRoughness 
    );

    RaytraceRenderer::InputTextures2 raytracerInputs;
    raytracerInputs.contribution                   = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 );
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

    m_raytraceRenderer.generateAndTraceRefractedRays( 
        level - 1, 
        refractionLevel,
        raytracerInputs,
        raytracerRTs,
        blockActors 
    );

    // Initialize shading output with emissive color.
    m_shadingRenderer.performShading( currLayerRTs.hitShaded, currLayerRTs.hitEmissive );

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

    const int lightCount = (int)lightsCastingShadows.size();
    for ( int lightIdx = 0; lightIdx < lightCount; ++lightIdx )
    {
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
            lightsCastingShadows[ lightIdx ],
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
            //nullptr, //#TODO: Should I use a pre-illumination?
            blockActors
        );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_mipmapRenderer.generateMipmapsWithSampleRejection(
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            500.0f, 0, 3
        );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Distance to occluder shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            currLayerRTs.hitPosition,
            currLayerRTs.hitNormal,
            m_raytraceShadowRenderer.getDistanceToOccluder(),
            *lightsCastingShadows[ lightIdx ]
        );

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        // #TODO RENDER: Blur shadow shader need to calculate blur radius in screen-space so it need to account for distance along the ray + dist to camera.
        // Modify shader of pass appropriate accumulated distance to camera to shader.
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
        if ( settings().rendering.shadows.useSeparableShadowBlur ) {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        } else {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                currLayerRTs.hitPosition,
                currLayerRTs.hitNormal,
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                m_raytraceShadowRenderer.getDistanceToOccluder(),
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            currLayerRTs.hitShaded,
            currLayerRTs.rayOrigin,
            currLayerRTs.hitPosition,
            currLayerRTs.hitAlbedo,
            currLayerRTs.hitMetalness,
            currLayerRTs.hitRoughness,
            currLayerRTs.hitNormal,
            m_blurShadowsRenderer.getShadowTexture(),
            *lightsCastingShadows[ lightIdx ]
        );
    }


    // Generate mipmaps for the shaded, reflected image.
    currLayerRTs.hitShaded->generateMipMapsOnGpu( *m_deviceContext.Get() );

    const int contributionTextureFillWidth  = prevLayerRTs.rayOrigin->getWidth();
    const int contributionTextureFillHeight = prevLayerRTs.rayOrigin->getHeight();

    const int colorTextureFillWidth  = currLayerRTs.rayOrigin->getWidth();
    const int colorTextureFillHeight = currLayerRTs.rayOrigin->getHeight();

    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        currLayerRTs.hitDistance,
        50.0f, 0, 0
    );

    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera,
        currLayerRTs.rayOrigin,
        currLayerRTs.hitNormal,
        currLayerRTs.hitDistance
    );

    m_combiningRenderer.combine( 
        m_finalRenderTargetHDR, 
        currLayerRTs.hitShaded,
        m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
        currLayerRTs.hitNormal,
        currLayerRTs.hitPosition,
        firstLayerRTs.depth,
        m_hitDistanceSearchRenderer.getFinalHitDistanceTexture(),
        camera.getPosition(),
        contributionTextureFillWidth, 
        contributionTextureFillHeight,
        colorTextureFillWidth, 
        colorTextureFillHeight 
    );
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

void Renderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
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

std::string Renderer::viewToString( const View view )
{
    switch ( view ) {
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
        case View::HardIllumination:                         return "HardIllumination";
        case View::SoftIllumination:                         return "SoftIllumination";
        case View::BlurredIllumination:                      return "BlurredIllumination";
        case View::SpotlightDepth:                           return "SpotlightDepth";
        case View::DistanceToOccluder:                       return "DistanceToOccluder";
        case View::FinalDistanceToOccluder:                  return "FinalDistanceToOccluder";
        case View::BloomBrightPixels:                        return "BloomBrightPixels";
        case View::HitDistance:                              return "HitDistance";
        case View::FinalHitDistance:                         return "FinalHitDistance";
        case View::HitDistanceToCamera:                      return "HitDistanceToCamera";
        case View::Test:                                     return "Test";
    }

    return "";
}
