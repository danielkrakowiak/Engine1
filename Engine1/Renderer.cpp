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
#include "SpotLight.h"

#include "StagingTexture2D.h"

#include "StringUtil.h"
#include "SceneUtil.h"

#include "Settings.h"

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
    m_activeViewType( View::Final ),
    m_exposure( 1.0f ),
    m_minBrightness( 1.0f )
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
    m_hitDistanceSearchRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_combiningRenderer.initialize( device, deviceContext );
    m_textureRescaleRenderer.initialize( device, deviceContext );
    m_rasterizeShadowRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
	m_raytraceShadowRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
	m_shadowMapRenderer.initialize( device, deviceContext );
    m_mipmapRenderer.initialize( device, deviceContext );
    m_distanceToOccluderSearchRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_blurShadowsRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    m_utilityRenderer.initialize( device, deviceContext );
    m_extractBrightPixelsRenderer.initialize( device, deviceContext );
    m_toneMappingRenderer.initialize( device, deviceContext );

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
    const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
    const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
    const std::vector< std::shared_ptr< Light > >& selectedLights,
    const std::shared_ptr< BlockMesh > selectionVolumeMesh )
{
    // Render shadow maps. #TODO: Should NOT be done every frame.
    //renderShadowMaps( scene );

    const auto& actors                  = scene.getActorsVec();
    const auto& lights                  = scene.getLightsVec();
    const auto  lightsCastingShadows    = SceneUtil::filterLightsByShadowCasting( lights, true );
    const auto  lightsNotCastingShadows = SceneUtil::filterLightsByShadowCasting( lights, false );
    const auto  blockActors             = SceneUtil::filterActorsByType< BlockActor >( actors );

    Output output; 
    output = renderMainImage( 
        scene, camera, lightsCastingShadows, lightsNotCastingShadows, 
        settings().rendering.reflectionsRefractions.activeView, m_activeViewType, wireframeMode,
        selectedBlockActors, selectedSkeletonActors, selectedLights, selectionVolumeMesh 
    );
    
    if ( !output.isEmpty() )
        return output;

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
    
    // Perform post-effects.
    performToneMapping( m_finalRenderTarget, m_exposure );
    
    performBloom( m_finalRenderTarget, m_minBrightness );

    if ( m_activeViewType == View::BloomBrightPixels )
    {
        Output output;
        output.float4Image = m_temporaryRenderTarget2;

        return output;
    }

    if ( !output.isEmpty() )
        return output;

    Output finalOutput;
    finalOutput.float4Image = m_finalRenderTarget;

    return finalOutput;
}

std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >
Renderer::renderText( const std::string& text, Font& font, float2 position, float4 color )
{
    m_deferredRenderer.render( text, font, position, color );

    return m_deferredRenderer.getAlbedoRenderTarget();
}

Renderer::Output Renderer::renderMainImage( 
    const Scene& scene, const Camera& camera, 
    const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
    const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
    const std::vector< bool >& activeViewLevel, const View activeViewType,
    const bool wireframeMode,
    const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
    const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
    const std::vector< std::shared_ptr< Light > >& selectedLights,
    const std::shared_ptr< BlockMesh > selectionVolumeMesh )
{
    float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint(), camera.getPosition(), camera.getUp() );

    m_profiler.beginEvent( Profiler::GlobalEventType::DeferredRendering );

    // Render 'axises' model.
    if ( m_axisMesh )
        m_deferredRenderer.render( *m_axisMesh, float43::IDENTITY, viewMatrix );

    float4 actorSelectionEmissiveColor( 0.2f, 0.2f, 0.0f, 1.0f );

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

    // Render selection volume.
    if ( selectionVolumeMesh )
        m_deferredRenderer.renderEmissive( *selectionVolumeMesh, float43::IDENTITY, viewMatrix, false );

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

    // #TODO: Remove.
    // FOR DEBUG - Not needed normally, because whole texture gets overwritten.
    m_blurShadowsRenderer.getShadowTexture()->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 0.0f, 0.0f, 0.0f, 1.0f ), 0 );

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

        auto distanceToOccluder = m_rasterizeShadowRenderer.getDistanceToOccluder();

        // Clear shadow raytracing render targets.
        // Note: Same thing is done in rasterize-shadow-renderer - take care to avoid duplication of actions.
        // #TODO: Should be profiled.
        m_raytraceShadowRenderer.getHardShadowTexture()->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
        m_raytraceShadowRenderer.getSoftShadowTexture()->clearRenderTargetView( *m_deviceContext.Get(), float4::ZERO );
        m_raytraceShadowRenderer.getDistanceToOccluder()->clearRenderTargetView( *m_deviceContext.Get(), float4( 1000.0f ) );

        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

        m_raytraceShadowRenderer.generateAndTraceShadowRays(
            camera.getPosition(),
            lightsCastingShadows[ lightIdx ],
            m_deferredRenderer.getPositionRenderTarget(),
            m_deferredRenderer.getNormalRenderTarget(),
            nullptr,
            m_rasterizeShadowRenderer.getShadowTexture(),
            distanceToOccluder,
            blockActors
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );

        m_raytraceShadowRenderer.getHardShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );
        m_raytraceShadowRenderer.getSoftShadowTexture()->generateMipMapsOnGpu( *m_deviceContext.Get() );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapGenerationForIllumination );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );

        m_mipmapRenderer.generateMipmapsWithSampleRejection( 
            distanceToOccluder, 
            m_deferredRenderer.getPositionRenderTarget(), 
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
                    output.floatImage = m_rasterizeShadowRenderer.getDistanceToOccluder();

                    return output;
                }
            }
        }

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        
        // Distance to occluder search.
        m_distanceToOccluderSearchRenderer.performDistanceToOccluderSearch(
            camera,
            m_deferredRenderer.getPositionRenderTarget(),
            m_deferredRenderer.getNormalRenderTarget(),
            distanceToOccluder,
            *lightsCastingShadows[ lightIdx ]
        );

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::DistanceToOccluderSearch );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );

        if ( settings().rendering.shadows.useSeparableShadowBlur )
        {
            // Blur shadows in two passes - horizontal and vertical.
            m_blurShadowsRenderer.blurShadowsHorzVert(
                camera,
                m_deferredRenderer.getPositionRenderTarget(),
                m_deferredRenderer.getNormalRenderTarget(),
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                distanceToOccluder,
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }
        else
        {
            // Blur shadows in a single pass.
            m_blurShadowsRenderer.blurShadows(
                camera,
                m_deferredRenderer.getPositionRenderTarget(),
                m_deferredRenderer.getNormalRenderTarget(),
                //m_rasterizeShadowRenderer.getIlluminationTexture(),     // TEMP: Enabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getHardShadowTexture(), // TEMP: Disabled for Shadow Mapping tests.
                m_raytraceShadowRenderer.getSoftShadowTexture(),
                distanceToOccluder,
                m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture(),
                *lightsCastingShadows[ lightIdx ]
            );
        }

        m_profiler.endEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::BlurShadows );
        m_profiler.beginEvent( Profiler::StageType::Main, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( 
            camera, 
            m_deferredRenderer.getPositionRenderTarget(),
			m_deferredRenderer.getAlbedoRenderTarget(), 
            m_deferredRenderer.getMetalnessRenderTarget(),
			m_deferredRenderer.getRoughnessRenderTarget(), 
            m_deferredRenderer.getNormalRenderTarget(), 
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
        *std::static_pointer_cast< Texture2DSpecUsage< TexUsage::Default, float4 > >( m_finalRenderTarget ), 0, 
        *std::static_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, float4 > >( m_shadingRenderer.getColorRenderTarget() ), 0 
    );

    m_profiler.endEvent( Profiler::GlobalEventType::CopyFrameToFinalRenderTarget );

    if ( activeViewLevel.empty() )
    {
        Output output;

        switch ( activeViewType )
        {
            case View::Shaded: 
                output.float4Image = m_shadingRenderer.getColorRenderTarget(); 
                break;
            case View::Depth: 
                output.uchar4Image = m_deferredRenderer.getDepthRenderTarget(); 
                break;
            case View::Position: 
                output.float4Image = m_deferredRenderer.getPositionRenderTarget();
                break;
            case View::Emissive: 
                output.uchar4Image = m_deferredRenderer.getEmissiveRenderTarget();
                break;
            case View::Albedo: 
                output.uchar4Image = m_deferredRenderer.getAlbedoRenderTarget();
                break;
            case View::Normal:
                output.float4Image = m_deferredRenderer.getNormalRenderTarget();
                break;
            case View::Metalness:
                output.ucharImage = m_deferredRenderer.getMetalnessRenderTarget();
                break;
            case View::Roughness:
                output.ucharImage = m_deferredRenderer.getRoughnessRenderTarget();
                break;
            case View::IndexOfRefraction:
                output.ucharImage = m_deferredRenderer.getIndexOfRefractionRenderTarget();
                break;
			case View::Preillumination:
				output.ucharImage = m_rasterizeShadowRenderer.getShadowTexture();
                break;
            case View::HardIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getHardShadowTexture();
                break;
            case View::SoftIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getSoftShadowTexture();
                break;
            case View::BlurredIllumination:
                output.ucharImage = m_blurShadowsRenderer.getShadowTexture();
                break;
            case View::SpotlightDepth:
                if ( !lightsCastingShadows.empty() && lightsCastingShadows[0]->getType() == Light::Type::SpotLight )
                    output.floatImage = std::static_pointer_cast< SpotLight >( lightsCastingShadows[ 0 ] )->getShadowMap();
                break;
            case View::DistanceToOccluder:
                output.floatImage = m_rasterizeShadowRenderer.getDistanceToOccluder();
                break;
            case View::FinalDistanceToOccluder:
                output.floatImage = m_distanceToOccluderSearchRenderer.getFinalDistanceToOccluderTexture();
                break;
        }

        return output;
    }

    return Output();
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
        Output output;

        switch ( activeViewType )
        {
            case View::Shaded: 
                output.float4Image = m_shadingRenderer.getColorRenderTarget();
                break;
            case View::Position: 
                output.float4Image = m_raytraceRenderer.getRayHitPositionTexture( level - 1 );
                break;
            case View::Emissive: 
                output.uchar4Image = m_raytraceRenderer.getRayHitEmissiveTexture( level - 1 );
                break;
            case View::Albedo: 
                output.uchar4Image = m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 );
                break;
            case View::Normal:
                output.float4Image = m_raytraceRenderer.getRayHitNormalTexture( level - 1 );
                break;
            case View::Metalness:
                output.ucharImage = m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 );
                break;
            case View::Roughness:
                output.ucharImage = m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 );
                break;
            case View::IndexOfRefraction:
                output.ucharImage = m_raytraceRenderer.getCurrentRefractiveIndexTextures().at( level - 1 );
                break;
            case View::RayDirections: 
                output.float4Image = m_raytraceRenderer.getRayDirectionsTexture( level - 1 );
                break;
            case View::Contribution: 
                output.uchar4Image = m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 );
                break;
            case View::CurrentRefractiveIndex:
                output.ucharImage = m_raytraceRenderer.getCurrentRefractiveIndexTextures().at( level - 1 );
                break;
            case View::Preillumination:
                output.ucharImage = m_rasterizeShadowRenderer.getShadowTexture();
                break;
            case View::HardIllumination:
                output.ucharImage = m_raytraceShadowRenderer.getHardShadowTexture();
                break;
            case View::HitDistance:
                output.floatImage = m_raytraceRenderer.getRayHitDistanceTexture( level - 1 );
                break;
            case View::FinalHitDistance:
                output.floatImage = m_hitDistanceSearchRenderer.getFinalHitDistanceTexture();
                break;
        }

        return output;
    }

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
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

        if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight
             && std::static_pointer_cast<SpotLight>( lightsCastingShadows[ lightIdx ] )->getShadowMap()
        ) 
        {
            m_rasterizeShadowRenderer.performShadowMapping(
                camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
                lightsCastingShadows[ lightIdx ],
                m_raytraceRenderer.getRayHitPositionTexture( 0 ),
                m_raytraceRenderer.getRayHitNormalTexture( 0 )
            );
        }

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );

		//m_raytraceShadowRenderer.generateAndTraceShadowRays( 
		//	lightsCastingShadows[ lightIdx ], 
		//	m_raytraceRenderer.getRayHitPositionTexture( 0 ), 
		//	m_raytraceRenderer.getRayHitNormalTexture( 0 ), 
		//	m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
  //          nullptr, //#TODO: Should I use a pre-illumination?
		//	blockActors 
		//);

        m_profiler.endEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::RaytracingShadows );
        m_profiler.beginEvent( Profiler::StageType::R, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading( 
			m_raytraceRenderer.getRayOriginsTexture( 0 ),
			m_raytraceRenderer.getRayHitPositionTexture( 0 ),
			m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
			m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
			m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
			m_raytraceRenderer.getRayHitNormalTexture( 0 ),
			m_rasterizeShadowRenderer.getShadowTexture()/*m_raytraceShadowRenderer.getIlluminationTexture()*/,
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

    auto rayHitDistanceTexture = m_raytraceRenderer.getRayHitDistanceTexture( 0 );

    //#TODO: Should be profiled separately.
    
    //#TODO: Fill pixels for which all samples were rejected with max value.
    m_mipmapRenderer.generateMipmapsWithSampleRejection(
        rayHitDistanceTexture,
        m_deferredRenderer.getPositionRenderTarget(),
        50.0f, 0, 0
    );

    // Search for hit distance.
    m_hitDistanceSearchRenderer.performHitDistanceSearch(
        camera, 
        m_deferredRenderer.getPositionRenderTarget(), 
        m_deferredRenderer.getNormalRenderTarget(), 
        rayHitDistanceTexture 
    );

    // Combine main image with reflections.
    m_combiningRenderer.combine( m_finalRenderTarget, 
                                 m_shadingRenderer.getColorRenderTarget(), 
                                 m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                                 m_deferredRenderer.getNormalRenderTarget(), 
                                 m_deferredRenderer.getPositionRenderTarget(), 
                                 m_deferredRenderer.getDepthRenderTarget(), 
                                 m_hitDistanceSearchRenderer.getFinalHitDistanceTexture(),
                                 camera.getPosition(),
                                 contributionTextureFillWidth, contributionTextureFillHeight,
                                 colorTextureFillWidth, colorTextureFillHeight );

    m_profiler.endEvent( Profiler::StageType::R, Profiler::EventTypePerStage::CombiningWithMainImage );
}

void Renderer::renderFirstRefractions( const Camera& camera, 
                                       const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
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
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

        if ( lightsCastingShadows[ lightIdx ]->getType() == Light::Type::SpotLight
             && std::static_pointer_cast<SpotLight>( lightsCastingShadows[ lightIdx ] )->getShadowMap()
        ) 
        {
            m_rasterizeShadowRenderer.performShadowMapping(
                camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
                lightsCastingShadows[ lightIdx ],
                m_raytraceRenderer.getRayHitPositionTexture( 0 ),
                m_raytraceRenderer.getRayHitNormalTexture( 0 )
            );
        }

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );

		//m_raytraceShadowRenderer.generateAndTraceShadowRays(
		//	lightsCastingShadows[ lightIdx ],
		//	m_raytraceRenderer.getRayHitPositionTexture( 0 ),
		//	m_raytraceRenderer.getRayHitNormalTexture( 0 ), 
		//	m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ),
  //          nullptr, //#TODO: Should I use a pre-illumination?
		//	blockActors
		//);

        m_profiler.endEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::ShadowsMapping );
        m_profiler.beginEvent( Profiler::StageType::T, lightIdx, Profiler::EventTypePerStagePerLight::Shading );

		// Perform shading on the main image.
		m_shadingRenderer.performShading(
			m_raytraceRenderer.getRayOriginsTexture( 0 ),
			m_raytraceRenderer.getRayHitPositionTexture( 0 ),
			m_raytraceRenderer.getRayHitAlbedoTexture( 0 ),
			m_raytraceRenderer.getRayHitMetalnessTexture( 0 ),
			m_raytraceRenderer.getRayHitRoughnessTexture( 0 ),
			m_raytraceRenderer.getRayHitNormalTexture( 0 ),
			m_rasterizeShadowRenderer.getShadowTexture()/*m_raytraceShadowRenderer.getIlluminationTexture()*/,
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
                                  const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
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
        if ( light->getType() == Light::Type::SpotLight
             && std::static_pointer_cast<SpotLight>( light )->getShadowMap()
        ) 
        {
            m_rasterizeShadowRenderer.performShadowMapping(
                camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
                light,
                m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
                m_raytraceRenderer.getRayHitNormalTexture( level - 1 )
            );
        }

        //m_raytraceShadowRenderer.generateAndTraceShadowRays(
        //    light,
        //    m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
        //    m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
        //    m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
        //    nullptr, //#TODO: Should I use a pre-illumination?
        //    blockActors
        //    );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
            m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_rasterizeShadowRenderer.getShadowTexture()/*m_raytraceShadowRenderer.getIlluminationTexture()*/,
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
                                  const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
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
        if ( light->getType() == Light::Type::SpotLight
             && std::static_pointer_cast<SpotLight>( light )->getShadowMap()
        ) 
        {
            m_rasterizeShadowRenderer.performShadowMapping(
                camera.getPosition(), // #TODO: THIS IS NOT OCRRECT - another version of this shader should be used which takes prev layer ray origin as camera position in each pixel.
                light,
                m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
                m_raytraceRenderer.getRayHitNormalTexture( level - 1 )
            );
        }

        //m_raytraceShadowRenderer.generateAndTraceShadowRays(
        //    light,
        //    m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
        //    m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
        //    m_reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
        //    nullptr, //#TODO: Should I use a pre-illumination?
        //    blockActors
        //    );

        // Perform shading on the main image.
        m_shadingRenderer.performShading(
            m_raytraceRenderer.getRayOriginsTexture( level - 1 ),
            m_raytraceRenderer.getRayHitPositionTexture( level - 1 ),
            m_raytraceRenderer.getRayHitAlbedoTexture( level - 1 ),
            m_raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitRoughnessTexture( level - 1 ),
            m_raytraceRenderer.getRayHitNormalTexture( level - 1 ),
            m_rasterizeShadowRenderer.getShadowTexture()/*m_raytraceShadowRenderer.getIlluminationTexture()*/,
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

    m_utilityRenderer.mergeMipmapsValues( m_finalRenderTarget, m_temporaryRenderTarget2, 0, maxMipmapLevel );

    m_profiler.endEvent( Profiler::GlobalEventType::Bloom );
}

void Renderer::performToneMapping( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > texture, const float exposure )
{
    m_profiler.beginEvent( Profiler::GlobalEventType::ToneMapping );

    m_toneMappingRenderer.performToneMapping( texture, exposure );

    m_profiler.endEvent( Profiler::GlobalEventType::ToneMapping );
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
    m_finalRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_temporaryRenderTarget1 = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_temporaryRenderTarget2 = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& Renderer::debugGetCurrentRefractiveIndexTextures()
{
    return m_raytraceRenderer.getCurrentRefractiveIndexTextures();
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
        case View::Test:                                     return "Test";
    }

    return "";
}
