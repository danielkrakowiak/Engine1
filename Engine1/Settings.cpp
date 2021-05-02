#include "Settings.h"

#include <algorithm>
#include "MathUtil.h"

#include "uchar4.h"

using namespace Engine1;

Settings Settings::s_settings;
bool     Settings::s_modified = false;

const Settings& Engine1::settings()
{
    return Settings::get();
}

Settings::Settings()
{
    initializeInternal();
}

Settings::~Settings()
{}

const Settings& Settings::get() 
{ 
    if ( s_modified )
        s_settings.onChanged();

    return s_settings; 
}

Settings& Settings::modify()
{
    if ( s_modified )
        s_settings.onChanged();

    s_modified = true;

    return s_settings;
}

void Settings::initializeInternal()
{
    main.fullscreen           = false;
    main.screenDimensions     = int2( 1024 /*1920*/, 768 /*1080*/ );
    main.verticalSync         = false;
    main.limitFPS             = false;
    main.displayFrequency     = 60;
    main.screenColorDepth     = 32;
    main.zBufferDepth         = 32;
    main.backBufferFrameCount = 3;
    main.useWARP              = false;

    paths.assets                    = "Assets";
    paths.testAssets                = "TestAssets";
    paths.renderingTests.testCases  = "RenderingTests//TestCases";
    paths.renderingTests.references = "RenderingTests//References";
    paths.renderingTests.results    = "RenderingTests//Results";

    debug.debugRenderAlpha          = false;
    debug.debugWireframeMode        = false;
    debug.debugDisplayedMipmapLevel = 0;
    debug.renderFps                 = true;
    debug.renderText                = false;
    debug.renderLightSources        = true;
    debug.slowmotionMode            = false;
    debug.snappingMode              = false;

    debug.replaceSelected = true;

    debug.alphaMulChanged           = false;
    debug.emissiveMulChanged        = false;
    debug.albedoMulChanged          = false;
    debug.metalnessMulChanged       = false;
    debug.roughnessMulChanged       = false;
    debug.refractiveIndexMulChanged = false;

    debug.alphaMul                = 1.0f;
    debug.emissiveMul             = 1.0f;
    debug.emissiveBaseMul         = float3( 1.0f, 1.0f, 1.0f );
    debug.albedoMul               = float3( 1.0f, 1.0f, 1.0f );
    debug.metalnessMul            = 1.0f;
    debug.roughnessMul            = 1.0f;
    debug.refractiveIndexMul      = 1.0f;

    
    debug.lightEnabled                    = true;
    debug.lightCastShadows                = true;
    debug.lightColor                      = float3::ONE;
    debug.lightIntensity                  = 1.0f;
    debug.lightEmitterRadius              = 0.0f;
    debug.lightLinearAttenuationFactor    = 1.0f;
    debug.lightQuadraticAttenuationFactor = 0.0f;

    debug.lightEnabledChanged                    = false;
    debug.lightCastShadowsChanged                = false;
    debug.lightColorChanged                      = false;
    debug.lightIntensityChanged                  = false;
    debug.lightEmitterRadiusChanged              = false;
    debug.lightLinearAttenuationFactorChanged    = false;
    debug.lightQuadraticAttenuationFactorChanged = false;

    debug.lightColorChanged = false;

    rendering.fieldOfViewDegress = 70.0f;

    rendering.skyColor = float3(0.12f, 0.53f, 1.0f);

    rendering.optimization.useHalfFloatsForRayDirections              = true;
    rendering.optimization.useHalfFloatsForNormals                    = true;
    rendering.optimization.useHalfFLoatsForDistanceToOccluder         = false;
    rendering.optimization.useHalfFloatsForHitDistance                = false;
    rendering.optimization.distToOccluderPositionSampleMipmapLevel    = 1;
    rendering.optimization.distToOccluderNormalSampleMipmapLevel      = 1;

    rendering.optimization.blurShadowPatternShadowSampleMipmapLevel   = 0;
    rendering.optimization.blurShadowPatternPositionSampleMipmapLevel = 0;
    rendering.optimization.blurShadowPatternNormalSampleMipmapLevel   = 0;
    rendering.optimization.blurShadowsPositionSampleMipmapLevel       = 0;
    rendering.optimization.blurShadowsNormalSampleMipmapLevel         = 0;

    animation.cameraPlaybackSpeed = 1.0f;
    animation.lightsPlaybackSpeed = 1.0f;
    animation.actorsPlaybackSpeed = 1.0f;

    rendering.ambientOcclusion.assao.enabled                             = true;
    rendering.ambientOcclusion.assao.radius                              = 1.35f;
    rendering.ambientOcclusion.assao.shadowMultiplier                    = 0.65f;
    rendering.ambientOcclusion.assao.shadowPower                         = 0.75f;
    rendering.ambientOcclusion.assao.shadowClamp                         = 0.98f;
    rendering.ambientOcclusion.assao.horizonAngleThreshold               = 0.06f;
    rendering.ambientOcclusion.assao.fadeOutFrom                         = 50.0f;
    rendering.ambientOcclusion.assao.fadeOutTo                           = 300.0f;
    rendering.ambientOcclusion.assao.adaptiveQualityLimit                = 0.45f;
    rendering.ambientOcclusion.assao.qualityLevel                        = 3;
    rendering.ambientOcclusion.assao.blurPassCount                       = 2;
    rendering.ambientOcclusion.assao.sharpness                           = 0.98f;
    rendering.ambientOcclusion.assao.temporalSupersamplingAngleOffset    = 0.0f;
    rendering.ambientOcclusion.assao.temporalSupersamplingRadiusOffset   = 1.0f;
    rendering.ambientOcclusion.assao.detailShadowStrength                = 0.5f;

    rendering.shadows.enabled                       = true;
    rendering.shadows.enableAlteringRayDirection    = false;
    rendering.shadows.enableBlurShadowPattern       = false;
    rendering.shadows.useSeparableShadowPatternBlur = false;
    rendering.shadows.useSeparableShadowBlur        = true;

    rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold                     = 10.0f;
    rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold                     = 40.0f;
    rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth               = 6.0f;
    rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth               = 20.0f;
    rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth = 6.0f;
    rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth = 20.0f;

    rendering.shadows.distanceToOccluderSearch.maxDistToOccluder = 500.0f;

    rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold         = 0.2f;
    rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold           = 0.7f;
    rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight       = 10.0f;//5.0f
    rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight         = 1.0f;//1.0f
    rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow      = 8.0f;//2.0f
    rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow        = 1.0f;//1.0f
    rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel          = 2;//2
    rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider   = 4;//4

    rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold       = 0.2f;
    rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold         = 0.7f;
    rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight     = 10.0f;//10.0 
    rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight       = 1.0f;//1.0
    rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow    = 7.0f;//5.0
    rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow      = 1.0f;//1.0
    rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel        = 3;//3
    rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider = 8;//8

    rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold         = 0.2f;
    rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold           = 1.0f;
    rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight       = 10.0f;//10.0
    rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight         = 1.0f;//1.0
    rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow      = 7.0f;//7.0
    rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow        = 1.0f;//1.0
    rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel          = 4;//4
    rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider   = 16;//16

    rendering.shadows.blurPattern.hardShadows.positionThreshold   = 0.08f; 
    rendering.shadows.blurPattern.hardShadows.normalThreshold     = 0.08f; 
    rendering.shadows.blurPattern.mediumShadows.positionThreshold = 0.08f; 
    rendering.shadows.blurPattern.mediumShadows.normalThreshold   = 0.08f; 
    rendering.shadows.blurPattern.softShadows.positionThreshold   = 0.08f;
    rendering.shadows.blurPattern.softShadows.normalThreshold     = 0.08f;

    rendering.shadows.blur.radiusMultiplier = 1.0f;
        
    // Note: To avoid light leaks at shadow layer transitions,
    // it's better to keep position/normal threshold the same for all layers.
    // Otherwise shadow is blurred slightly differently for both layers, 
    // making a visible transition in the middle.
    rendering.shadows.blur.hardShadows.positionThreshold   = 0.12f; 
    rendering.shadows.blur.hardShadows.normalThreshold     = 0.08f; 
    rendering.shadows.blur.mediumShadows.positionThreshold = 0.12f; 
    rendering.shadows.blur.mediumShadows.normalThreshold   = 0.08f; 
    rendering.shadows.blur.softShadows.positionThreshold   = 0.12f;
    rendering.shadows.blur.softShadows.normalThreshold     = 0.08f;

    rendering.reflectionsRefractions.maxLevel            = 1;
    rendering.reflectionsRefractions.debugViewStage      = RenderingStage::Main;
    rendering.reflectionsRefractions.reflectionsEnabled  = false;
    rendering.reflectionsRefractions.refractionsEnabled  = false;
    rendering.reflectionsRefractions.samplingQuality     = 0.45f;
    rendering.reflectionsRefractions.roughnessBlurMul    = 30.0f;

    rendering.reflectionsRefractions.elongationMul     = 1.0f;
    rendering.reflectionsRefractions.radialBlurEnabled = true;
    rendering.reflectionsRefractions.debugHitDistPower = 0.5f;

    rendering.hitDistanceSearch.resolutionDivider          = 4;
    rendering.hitDistanceSearch.decreaseBlurForSmallValues = false;
    rendering.hitDistanceSearch.maxHitDistForDecreasedBlur = 0.15f;

    rendering.combining.positionDiffMul         = 6.0f;
    rendering.combining.normalDiffMul           = 3.0f;
    rendering.combining.positionNormalThreshold = 1.2f;

    rendering.postProcess.exposure      = 1.0f;

    rendering.postProcess.depthOfField.enabled                 = false;
    rendering.postProcess.depthOfField.apertureDiameter        = 0.05f;
    rendering.postProcess.depthOfField.focalLength             = 0.25f;
    rendering.postProcess.depthOfField.cameraFocusDist         = 1.0f;
    rendering.postProcess.depthOfField.setFocusAtClickedObject = true;
    rendering.postProcess.depthOfField.coCMul                  = 2834.0f;
    rendering.postProcess.depthOfField.maxCoC                  = 30.0f;
    rendering.postProcess.depthOfField.relativeDepthThreshold  = 0.05f;

    rendering.postProcess.bloom         = true;
    rendering.postProcess.antialiasing  = true;

    physics.fixedStepDuration = 1.0f / 60.0f;

    importer.defaultWhiteUchar4TextureFileName = "default_white_uchar4.png";

    profiling.display.enabled            = false;
    profiling.display.coloredByTimeTaken = true;
    profiling.display.startWithStage     = RenderingStage::Main;
}

void Settings::initialize(ID3D11Device3& device)
{
    { // Create default textures.
        std::vector< unsigned char > dataAlpha = { 255 };
        std::vector< unsigned char > dataMetalness = { 255 };
        std::vector< unsigned char > dataRoughness = { 255 };
        std::vector< unsigned char > dataIndexOfRefraction = { 255 };
        std::vector< uchar4 >        dataEmissive = { uchar4( 255, 255, 255, 255 ) };
        std::vector< uchar4 >        dataAlbedo = { uchar4( 255, 255, 255, 255 ) };
        std::vector< uchar4 >        dataNormal = { uchar4( 128, 128, 255, 255 ) };

        textures.defaults.alpha = 
            std::make_shared< ImmutableTexture2D< unsigned char > >
            ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        textures.defaults.metalness = 
            std::make_shared< ImmutableTexture2D< unsigned char > >
            ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        textures.defaults.roughness = 
            std::make_shared< ImmutableTexture2D< unsigned char > >
            ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        textures.defaults.refractiveIndex = 
            std::make_shared< ImmutableTexture2D< unsigned char > >
            ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        textures.defaults.emissive = 
            std::make_shared< ImmutableTexture2D< uchar4 > >
            ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

        textures.defaults.albedo = 
            std::make_shared< ImmutableTexture2D< uchar4 > >
            ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

        textures.defaults.normal = 
            std::make_shared< ImmutableTexture2D< uchar4 > >
            ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
    }
}

void Settings::onChanged()
{
    // Active-level should not exceed max-level.
    while ( getRenderingStageLevel( rendering.reflectionsRefractions.debugViewStage ) > rendering.reflectionsRefractions.maxLevel )
    {
        rendering.reflectionsRefractions.debugViewStage =
            getPrevRenderingStage( rendering.reflectionsRefractions.debugViewStage );
    }

    // Max-level has to be >= 0.
    rendering.reflectionsRefractions.maxLevel
        = std::max( 0, rendering.reflectionsRefractions.maxLevel );

    // Max-level has to be <= 8.
    rendering.reflectionsRefractions.maxLevel
        = std::min( 8, rendering.reflectionsRefractions.maxLevel );

    s_modified = false;
}