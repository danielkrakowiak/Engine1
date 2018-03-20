#include "SettingsHelper.h"

#include "Settings.h"

using namespace Engine1;

std::string SettingsHelper::compareSettings( const Settings& settings1, const Settings& settings2 )
{
    std::string text;
    text.reserve( 250 );

    text += (settings1.main.fullscreen                  != settings2.main.fullscreen)                  ? std::string("main.fullscreen = ")                  + (settings1.main.fullscreen ? "true" : "false") + "\n" : "";
    text += (settings1.main.screenDimensions            != settings2.main.screenDimensions)            ? std::string("main.screenDimensions = ")            + settings1.main.screenDimensions.toString() + "\n" : "";
    text += (settings1.main.verticalSync                != settings2.main.verticalSync)                ? std::string("main.verticalSync = ")                + (settings1.main.verticalSync ? "true" : "false") + "\n" : "";
    text += (settings1.main.limitFPS                    != settings2.main.limitFPS)                    ? std::string("main.limitFPS = ")                    + (settings1.main.limitFPS ? "true" : "false") + "\n" : "";
    text += (settings1.main.displayFrequency            != settings2.main.displayFrequency)            ? std::string("main.displayFrequency = ")            + std::to_string(settings1.main.displayFrequency) + "\n" : "";
    text += (settings1.main.screenColorDepth            != settings2.main.screenColorDepth)            ? std::string("main.screenColorDepth = ")            + std::to_string(settings1.main.screenColorDepth) + "\n" : "";
    text += (settings1.main.zBufferDepth                != settings2.main.zBufferDepth)                ? std::string("main.zBufferDepth = ")                + std::to_string(settings1.main.zBufferDepth) + "\n" : "";

    text += (settings1.debug.debugRenderAlpha           != settings2.debug.debugRenderAlpha)           ? std::string("debug.debugRenderAlpha = ")           + (settings1.debug.debugRenderAlpha ? "true" : "false") + "\n" : "";
    text += (settings1.debug.debugWireframeMode         != settings2.debug.debugWireframeMode)         ? std::string("debug.debugWireframeMode = ")         + (settings1.debug.debugWireframeMode ? "true" : "false") + "\n" : "";
    text += (settings1.debug.debugDisplayedMipmapLevel  != settings2.debug.debugDisplayedMipmapLevel)  ? std::string("debug.debugDisplayedMipmapLevel = ")  + std::to_string(settings1.debug.debugDisplayedMipmapLevel) + "\n" : "";
    text += (settings1.debug.renderText                 != settings2.debug.renderText)                 ? std::string("debug.renderText = ")                 + (settings1.debug.renderText ? "true" : "false") + "\n" : "";
    text += (settings1.debug.renderFps                  != settings2.debug.renderFps)                  ? std::string("debug.renderFps = ")                  + (settings1.debug.renderFps ? "true" : "false") + "\n" : "";
    text += (settings1.debug.slowmotionMode             != settings2.debug.slowmotionMode)             ? std::string("debug.slowmotionMode = ")             + (settings1.debug.slowmotionMode ? "true" : "false") + "\n" : "";
    text += (settings1.debug.snappingMode               != settings2.debug.snappingMode)               ? std::string("debug.snappingMode = ")               + (settings1.debug.snappingMode ? "true" : "false") + "\n" : "";

    text += (settings1.rendering.fieldOfViewDegress     != settings2.rendering.fieldOfViewDegress)     ? std::string("rendering.fieldOfViewDegress = ")     + std::to_string(settings1.rendering.fieldOfViewDegress) + "\n" : "";
    text += (settings1.rendering.exposure               != settings2.rendering.exposure)               ? std::string("rendering.exposure = ")               + std::to_string(settings1.rendering.exposure) + "\n" : "";
    text += (settings1.rendering.antialiasing           != settings2.rendering.antialiasing)           ? std::string("rendering.antialiasing = ")           + (settings1.rendering.antialiasing ? "true" : "false") + "\n" : "";

    text += (settings1.rendering.shadows.enabled                != settings2.rendering.shadows.enabled)                 ? std::string("rendering.shadows.enabled = ")                + (settings1.rendering.shadows.enabled ? "true" : "false") + "\n" : "";
    text += (settings1.rendering.shadows.useSeparableShadowBlur != settings2.rendering.shadows.useSeparableShadowBlur)  ? std::string("rendering.shadows.useSeparableShadowBlur = ") + (settings1.rendering.shadows.useSeparableShadowBlur ? "true" : "false") + "\n" : "";

    text += (settings1.rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold                        != settings2.rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold)                     ? std::string("rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold = ")                      + std::to_string(settings1.rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold) + "\n" : "";
    text += (settings1.rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold                        != settings2.rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold)                     ? std::string("rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold = ")                      + std::to_string(settings1.rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold) + "\n" : "";
    text += (settings1.rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth                  != settings2.rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth)               ? std::string("rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth = ")                + std::to_string(settings1.rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth) + "\n" : "";
    text += (settings1.rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth                  != settings2.rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth)               ? std::string("rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth = ")                + std::to_string(settings1.rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth) + "\n" : "";
    text += (settings1.rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth    != settings2.rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth) ? std::string("rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth = ")  + std::to_string(settings1.rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth) + "\n" : "";
    text += (settings1.rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth    != settings2.rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth) ? std::string("rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth = ")  + std::to_string(settings1.rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth) + "\n" : "";
                                                                                                               
    settings1.rendering.shadows.distanceToOccluderSearch.maxDistToOccluder;

    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel;
    settings1.rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider;

    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel;
    settings1.rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider;

    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel;
    settings1.rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider;

    settings1.rendering.shadows.blur.hardShadows.positionThreshold;
    settings1.rendering.shadows.blur.hardShadows.normalThreshold;
    settings1.rendering.shadows.blur.mediumShadows.positionThreshold;
    settings1.rendering.shadows.blur.mediumShadows.normalThreshold;
    settings1.rendering.shadows.blur.softShadows.positionThreshold;
    settings1.rendering.shadows.blur.softShadows.normalThreshold;

    text += (settings1.rendering.reflectionsRefractions.maxLevel           != settings2.rendering.reflectionsRefractions.maxLevel)           ? std::string("rendering.reflectionsRefractions.maxLevel = ")           + std::to_string(settings1.rendering.reflectionsRefractions.maxLevel) + "\n" : "";
    text += (settings1.rendering.reflectionsRefractions.reflectionsEnabled != settings2.rendering.reflectionsRefractions.reflectionsEnabled) ? std::string("rendering.reflectionsRefractions.reflectionsEnabled = ") + (settings1.rendering.reflectionsRefractions.reflectionsEnabled ? "true" : "false") + "\n" : "";
    text += (settings1.rendering.reflectionsRefractions.refractionsEnabled != settings2.rendering.reflectionsRefractions.refractionsEnabled) ? std::string("rendering.reflectionsRefractions.refractionsEnabled = ") + (settings1.rendering.reflectionsRefractions.refractionsEnabled ? "true" : "false") + "\n" : "";
    text += (settings1.rendering.reflectionsRefractions.samplingQuality    != settings2.rendering.reflectionsRefractions.samplingQuality)    ? std::string("rendering.reflectionsRefractions.samplingQuality = ")    + std::to_string(settings1.rendering.reflectionsRefractions.samplingQuality) + "\n" : "";
    text += (settings1.rendering.reflectionsRefractions.roughnessBlurMul   != settings2.rendering.reflectionsRefractions.roughnessBlurMul)   ? std::string("rendering.reflectionsRefractions.roughnessBlurMul = ")   + std::to_string(settings1.rendering.reflectionsRefractions.roughnessBlurMul) + "\n" : "";

    settings1.rendering.reflectionsRefractions.elongationMul;
    settings1.rendering.reflectionsRefractions.radialBlurEnabled;

    settings1.rendering.hitDistanceSearch.resolutionDivider;
    settings1.rendering.hitDistanceSearch.decreaseBlurForSmallValues;
    settings1.rendering.hitDistanceSearch.maxHitDistForDecreasedBlur;

    settings1.rendering.combining.positionDiffMul;
    settings1.rendering.combining.normalDiffMul;
    settings1.rendering.combining.positionNormalThreshold;

    settings1.physics.fixedStepDuration;

    settings1.importer.defaultWhiteUchar4TextureFileName;

    return text;
}
