#include "ControlPanel.h"

#include <d3d11_3.h>
#include "SceneManager.h"

#include "Settings.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ControlPanel::ControlPanel( SceneManager& sceneManager ) :
    m_mainBar( nullptr ),
    m_meshUtilsBar( nullptr ),
    m_lightBar( nullptr ),
    m_shadowsBar( nullptr ),
    m_sceneManager( sceneManager )
{}

ControlPanel::~ControlPanel()
{
    TwTerminate();

    m_mainBar      = nullptr;
    m_meshUtilsBar = nullptr;
    m_lightBar     = nullptr;
}

void ControlPanel::initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device, const int2 windowDimensions )
{
    TwInit( TW_DIRECT3D11, device.Get() );

    TwWindowSize( windowDimensions.x, windowDimensions.y );

    m_mainBar = TwNewBar("Main");
    TwDefine(" Main iconified=true ");

    TwAddVarRW( m_mainBar, "Reflections", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.reflectionsEnabled, "" );
    TwAddVarRW( m_mainBar, "Refractions", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.refractionsEnabled, "" );
    TwAddVarRW( m_mainBar, "Shadows", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.enabled, "" );

    TwAddButton( m_mainBar, "Reset camera", ControlPanel::onResetCamera, this, "" );

    TwAddVarRW( m_mainBar, "Replace selected", TW_TYPE_BOOL8, &Settings::s_settings.debug.replaceSelected, "" );

    TwAddVarRW( m_mainBar, "Sky color", TW_TYPE_COLOR3F, &Settings::s_settings.rendering.skyColor, "colormode=hls" );

    TwAddVarCB( m_mainBar, "Alpha mul", TW_TYPE_FLOAT, ControlPanel::onSetAlphaMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.alphaMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Emissive mul", TW_TYPE_FLOAT, ControlPanel::onSetEmissiveMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.emissiveMul, "min=0 max=100 step=0.01 precision=2" );
    TwAddVarCB( m_mainBar, "Emissive base mul", TW_TYPE_COLOR3F, ControlPanel::onSetEmissiveBaseMul, ControlPanel::onGetFloat3, &Settings::s_settings.debug.emissiveBaseMul, "colormode=hls" );
    TwAddVarCB( m_mainBar, "Albedo mul", TW_TYPE_COLOR3F, ControlPanel::onSetAlbedoMul, ControlPanel::onGetFloat3, &Settings::s_settings.debug.albedoMul, "colormode=hls" );
    TwAddVarCB( m_mainBar, "Metalness mul", TW_TYPE_FLOAT, ControlPanel::onSetMetalnessMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.metalnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Roughness mul", TW_TYPE_FLOAT, ControlPanel::onSetRoughnessMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.roughnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Refractive index mul", TW_TYPE_FLOAT, ControlPanel::onSetRefractiveIndexMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.refractiveIndexMul, "min=0 max=1 step=0.001 precision=3" );

    TwAddVarRW( m_mainBar, "Field of view", TW_TYPE_FLOAT, &Settings::s_settings.rendering.fieldOfViewDegress, "min=30 max=180 step=0.1 precision=1" );
    TwAddVarRW( m_mainBar, "Exposure", TW_TYPE_FLOAT, &Settings::s_settings.rendering.exposure, "min=-10 max=10 step=0.01 precision=2" );
    TwAddVarRW( m_mainBar, "Antialiasing", TW_TYPE_BOOL8, &Settings::s_settings.rendering.antialiasing, "" );

    m_meshUtilsBar = TwNewBar("Mesh_Utils");
    TwDefine(" Mesh_Utils iconified=true ");

    TwAddButton( m_meshUtilsBar, "Flip UVs (vertically)", ControlPanel::onFlipUVs, this, "" );
    TwAddButton( m_meshUtilsBar, "Flip Tangents", ControlPanel::onFlipTangents, this, "" );
    TwAddButton( m_meshUtilsBar, "Flip Normals", ControlPanel::onFlipNormals, this, "" );
    TwAddButton( m_meshUtilsBar, "Invert vertex winding order", ControlPanel::onInvertVertexWindingOrder, this, "" );

    m_lightBar = TwNewBar("Light");
    TwDefine(" Light iconified=true ");

    TwAddVarCB( m_lightBar, "Enabled", TW_TYPE_BOOL8, ControlPanel::onSetLightEnabled, ControlPanel::onGetBool, &Settings::s_settings.debug.lightEnabled, "" );
    TwAddVarCB( m_lightBar, "Cast shadows", TW_TYPE_BOOL8, ControlPanel::onSetLightCastShadows, ControlPanel::onGetBool, &Settings::s_settings.debug.lightCastShadows, "" );
    TwAddVarCB( m_lightBar, "Color", TW_TYPE_COLOR3F, ControlPanel::onSetLightColor, ControlPanel::onGetFloat3, &Settings::s_settings.debug.lightColor, "colormode=hls" );
    TwAddVarCB( m_lightBar, "Intensity", TW_TYPE_FLOAT, ControlPanel::onSetLightIntensity, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightIntensity, "min=0 max=10000 step=0.05 precision=2" );
    TwAddVarCB( m_lightBar, "Emitter radius", TW_TYPE_FLOAT, ControlPanel::onSetLightEmitterRadius, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightEmitterRadius, "min=0 max=1 step=0.002 precision=3" );
    TwAddVarCB( m_lightBar, "Linear attenuation factor", TW_TYPE_FLOAT, ControlPanel::onSetLightLinearAttenuationFactor, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightLinearAttenuationFactor, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_lightBar, "Linear quadratic factor", TW_TYPE_FLOAT, ControlPanel::onSetLightQuadraticAttenuationFactor, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightQuadraticAttenuationFactor, "min=0 max=1 step=0.001 precision=3" );

    //TwAddSeparator(m_shadowsBar, "", nullptr);

    m_reflectionRefractionBar = TwNewBar("Reflections_Refractions");
    TwDefine(" Reflections_Refractions iconified=true ");
    TwAddVarRW( m_reflectionRefractionBar, "Max level", TW_TYPE_INT32, &Settings::s_settings.rendering.reflectionsRefractions.maxLevel, "" );

    TwAddButton( m_reflectionRefractionBar, "Next - reflection", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( m_reflectionRefractionBar, "Next - transmission", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( m_reflectionRefractionBar, "Back", ControlPanel::onPrevLevel, nullptr, "" );

    TwAddVarRW( m_reflectionRefractionBar, "Combining sampling quality", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.samplingQuality, "min=0 max=1 step=0.002 precision=3" );
    TwAddVarRW( m_reflectionRefractionBar, "Roughness blur mul", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.roughnessBlurMul, "min=0 max=1000 step=0.2 precision=1" );
    TwAddVarRW( m_reflectionRefractionBar, "Reflection elongation mul", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.elongationMul, "min=0.1 max=6.0 step=0.01 precision=2" );
    TwAddVarRW( m_reflectionRefractionBar, "Reflection radial blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.radialBlurEnabled, "" );
    TwAddVarRW( m_reflectionRefractionBar, "Debug hit-dist power", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.debugHitDistPower, "min=0.01 max=2.0 step=0.01 precision=2" );

    TwAddButton( m_reflectionRefractionBar, "", nullptr, nullptr, " label='Hit-dist search' ");
    TwAddVarRW( m_reflectionRefractionBar, "Decrease blur for small values", TW_TYPE_BOOL8, &Settings::s_settings.rendering.hitDistanceSearch.decreaseBlurForSmallValues, "" );
    TwAddVarRW( m_reflectionRefractionBar, "Field of view", TW_TYPE_FLOAT, &Settings::s_settings.rendering.hitDistanceSearch.maxHitDistForDecreasedBlur, "min=0 max=1 step=0.01 precision=2" );

    m_shadowsBar = TwNewBar("Shadows");
    TwDefine(" Shadows iconified=true ");
    TwAddVarRW( m_shadowsBar, "Alter ray directions", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.enableAlteringRayDirection, "" );
    TwAddVarRW( m_shadowsBar, "Blur shadow pattern", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.enableBlurShadowPattern, "" );
    TwAddVarRW( m_shadowsBar, "Use separable shadow pattern blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowPatternBlur, "" );
    TwAddVarRW( m_shadowsBar, "Use separable shadow blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowBlur, "" );
    TwAddVarRW( m_shadowsBar, "Global blur radius multiplier", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.radiusMultiplier, "min=0 max=3 step=0.001 precision=3" );
    TwAddButton( m_shadowsBar, "", nullptr, nullptr, " label='(H - hard, M - medium, S - soft) shadows' ");

    TwAddButton( m_shadowsBar, "", nullptr, nullptr, " label='Raytracing, layers' ");
    TwAddVarRW( m_shadowsBar, "H blur radius threshold", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold, "min=0 max=30 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S blur radius threshold ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold, "min=0 max=70 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "H blur radius threshold width", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth, "min=0 max=15 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S blur radius threshold width", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth, "min=0 max=30 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "dist-to-occluder H blur radius threshold width", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth, "min=0 max=15 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "dist-to-occluder S blur radius threshold width", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth, "min=0 max=30 step=0.001 precision=3" );

    TwAddButton( m_shadowsBar, "", nullptr, nullptr, " label='Dist-to-occluder-search' ");
    TwAddVarRW( m_shadowsBar, "H pos threshold", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M pos threshold ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S pos threshold  ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddSeparator( m_shadowsBar, "", nullptr );
    TwAddVarRW( m_shadowsBar, "H normal threshold", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M normal threshold ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S normal threshold  ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );

    TwAddButton( m_shadowsBar, "", nullptr, nullptr, " label='Blur Pattern' ");
    TwAddVarRW( m_shadowsBar, "H pos threshold  ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.hardShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M pos threshold   ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.mediumShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S pos threshold    ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.softShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddSeparator( m_shadowsBar, "", nullptr );
    TwAddVarRW( m_shadowsBar, "H normal threshold  ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.hardShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M normal threshold   ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.mediumShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S normal threshold    ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blurPattern.softShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );

    TwAddButton( m_shadowsBar, "", nullptr, nullptr, " label='Blur' ");
    TwAddVarRW( m_shadowsBar, "H pos threshold     ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.hardShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M pos threshold      ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.mediumShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S pos threshold       ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.softShadows.positionThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddSeparator( m_shadowsBar, "", nullptr );
    TwAddVarRW( m_shadowsBar, "H normal threshold     ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.hardShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "M normal threshold      ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.mediumShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );
    TwAddVarRW( m_shadowsBar, "S normal threshold       ", TW_TYPE_FLOAT, &Settings::s_settings.rendering.shadows.blur.softShadows.normalThreshold, "min=0 max=3 step=0.001 precision=3" );

    m_profilingBar = TwNewBar("Profiling");
    TwDefine(" Profiling iconified=true ");
    TwAddVarRW( m_profilingBar, "Display", TW_TYPE_BOOL8, &Settings::s_settings.profiling.display.enabled, "" );
    TwAddVarRW( m_profilingBar, "Colored", TW_TYPE_BOOL8, &Settings::s_settings.profiling.display.coloredByTimeTaken, "" );
    TwAddButton( m_profilingBar, "Next - reflection", ControlPanel::onDisplayNextStageProfilingReflection, nullptr, "" );
    TwAddButton( m_profilingBar, "Next - transmission", ControlPanel::onDisplayNextStageProfilingTransmission, nullptr, "" );
    TwAddButton( m_profilingBar, "Back", ControlPanel::onDisplayPrevStageProfiling, nullptr, "" );

    m_assaoBar = TwNewBar("ASSAO");
    TwDefine(" ASSAO iconified=true ");
    TwAddVarRW( m_assaoBar, "Enabled",                              TW_TYPE_BOOL8, &Settings::s_settings.rendering.ambientOcclusion.assao.enabled, "" );
    TwAddVarRW( m_assaoBar, "Radius",                               TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.radius, "min=0 max=10 step=0.01 precision=2" );
    TwAddVarRW( m_assaoBar, "Shadow Multiplier",                    TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.shadowMultiplier, "min=0 max=5 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Shadow Power",                         TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.shadowPower, "min=0.5 max=5 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Shadow Clamp",                         TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.shadowClamp, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Horizon Angle Threshold",              TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.horizonAngleThreshold, "min=0 max=0.2 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Fade Out From",                        TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.fadeOutFrom, "min=0 max=500 step=0.1 precision=1" );
    TwAddVarRW( m_assaoBar, "Fade Out To",                          TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.fadeOutTo, "min=0 max=500 step=0.1 precision=1" );
    TwAddVarRW( m_assaoBar, "Quality Level",                        TW_TYPE_INT32, &Settings::s_settings.rendering.ambientOcclusion.assao.qualityLevel, "min=-1 max=3 step=1" );
    TwAddVarRW( m_assaoBar, "Adaptive Quality Limit",               TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.adaptiveQualityLimit, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Blur Pass Count",                      TW_TYPE_INT32, &Settings::s_settings.rendering.ambientOcclusion.assao.blurPassCount, "min=0 max=6 step=1" );
    TwAddVarRW( m_assaoBar, "Sharpness",                            TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.sharpness, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Temporal Supersampling Angle Offset",  TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.temporalSupersamplingAngleOffset, "min=0 max=3.14 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Temporal Supersampling Radius Offset", TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.temporalSupersamplingRadiusOffset, "min=0 max=2 step=0.001 precision=3" );
    TwAddVarRW( m_assaoBar, "Detail Shadow Strength",               TW_TYPE_FLOAT, &Settings::s_settings.rendering.ambientOcclusion.assao.detailShadowStrength, "min=0 max=5 step=0.001 precision=3" );

    m_optimizationBar = TwNewBar("Optimization");
    TwDefine(" Optimization iconified=true ");
    TwAddVarRW( m_optimizationBar, "Use separable shadow pattern blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowPatternBlur, "" );
    TwAddVarRW( m_optimizationBar, "Use separable shadow blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowBlur, "" );
    TwAddVarRW( m_optimizationBar, "Combining sampling quality", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.samplingQuality, "min=0 max=1 step=0.002 precision=3" );
    TwAddVarRW( m_optimizationBar, "Use half normals", TW_TYPE_BOOL8, &Settings::s_settings.rendering.optimization.useHalfFloatsForNormals, "" );
    TwAddVarRW( m_optimizationBar, "Use half ray directions", TW_TYPE_BOOL8, &Settings::s_settings.rendering.optimization.useHalfFloatsForRayDirections, "" );
    TwAddVarRW( m_optimizationBar, "Use half hit-distance", TW_TYPE_BOOL8, &Settings::s_settings.rendering.optimization.useHalfFloatsForHitDistance, "" );
    TwAddVarRW( m_optimizationBar, "Use half dist-to-occluder", TW_TYPE_BOOL8, &Settings::s_settings.rendering.optimization.useHalfFLoatsForDistanceToOccluder, "" );
    TwAddVarRW( m_optimizationBar, "Dist-to-occluder position sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.distToOccluderPositionSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Dist-to-occluder normal sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.distToOccluderNormalSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Blur shadow pattern shadow sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.blurShadowPatternShadowSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Blur shadow pattern position sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.blurShadowPatternPositionSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Blur shadow pattern normal sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.blurShadowPatternNormalSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Blur shadows position sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.blurShadowsPositionSampleMipmapLevel, "min=0 max=4" );
    TwAddVarRW( m_optimizationBar, "Blur shadows normal sampled mipmap level", TW_TYPE_INT32, &Settings::s_settings.rendering.optimization.blurShadowsNormalSampleMipmapLevel, "min=0 max=4" );

    m_animationBar = TwNewBar("Animation");
    TwDefine(" Animation iconified=true ");
    TwAddVarRW( m_animationBar, "Camera playback speed", TW_TYPE_FLOAT, &Settings::s_settings.animation.cameraPlaybackSpeed, "min=0 max=5 step=0.01 precision=2" );
    TwAddVarRW( m_animationBar, "Lights playback speed", TW_TYPE_FLOAT, &Settings::s_settings.animation.lightsPlaybackSpeed, "min=0 max=5 step=0.01 precision=2" );
    TwAddVarRW( m_animationBar, "Actors playback speed", TW_TYPE_FLOAT, &Settings::s_settings.animation.actorsPlaybackSpeed, "min=0 max=5 step=0.01 precision=2" );
}

int ControlPanel::processInput( void *wnd, unsigned int msg, unsigned __int64 _W64 wParam, __int64 _W64 lParam )
{
    const int result = TwEventWin( wnd, msg, wParam, lParam );

    // Input was processed, so notify Settings on potential change of settings.
    if ( result )
        Settings::onChanged(); 

    return result;
}

void ControlPanel::draw()
{
    TwDraw();
}

void TW_CALL ControlPanel::onNextLevelReflection( void* /*clientData*/ )
{
    Settings::modify().rendering.reflectionsRefractions.debugViewStage = 
        getNextRenderingStage( settings().rendering.reflectionsRefractions.debugViewStage, RenderingStageType::Reflection );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onNextLevelRefraction( void* /*clientData*/ )
{
    Settings::modify().rendering.reflectionsRefractions.debugViewStage = 
        getNextRenderingStage( settings().rendering.reflectionsRefractions.debugViewStage, RenderingStageType::Transmission );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onPrevLevel( void* /*clientData*/ )
{
    if ( settings().rendering.reflectionsRefractions.debugViewStage != RenderingStage::Main )
    {
        Settings::modify().rendering.reflectionsRefractions.debugViewStage = 
            getPrevRenderingStage( settings().rendering.reflectionsRefractions.debugViewStage );
    }

    Settings::onChanged();
}

void TW_CALL ControlPanel::onResetCamera( void* controlPanel )
{
    ( (ControlPanel*)controlPanel )->m_sceneManager.getCamera()->setPosition( float3::ZERO );
}

void TW_CALL ControlPanel::onSetAlphaMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.alphaMul        = *(float*)value;
    Settings::s_settings.debug.alphaMulChanged = true;
}

void TW_CALL ControlPanel::onSetEmissiveMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.emissiveMul        = *(float*)value;
    Settings::s_settings.debug.emissiveMulChanged = true;
}

void TW_CALL ControlPanel::onSetEmissiveBaseMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.emissiveBaseMul        = *(float3*)value;
    Settings::s_settings.debug.emissiveMulChanged = true;
}

void TW_CALL ControlPanel::onSetAlbedoMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.albedoMul        = *(float3*)value;
    Settings::s_settings.debug.albedoMulChanged = true;
}

void TW_CALL ControlPanel::onSetMetalnessMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.metalnessMul        = *(float*)value;
    Settings::s_settings.debug.metalnessMulChanged = true;
}

void TW_CALL ControlPanel::onSetRoughnessMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.roughnessMul        = *(float*)value;
    Settings::s_settings.debug.roughnessMulChanged = true;
}

void TW_CALL ControlPanel::onSetRefractiveIndexMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.refractiveIndexMul        = *(float*)value;
    Settings::s_settings.debug.refractiveIndexMulChanged = true;
}

void TW_CALL ControlPanel::onSetLightEnabled( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightEnabled        = *(bool*)value;
    Settings::s_settings.debug.lightEnabledChanged = true;
}

void TW_CALL ControlPanel::onSetLightCastShadows( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightCastShadows        = *(bool*)value;
    Settings::s_settings.debug.lightCastShadowsChanged = true;
}

void TW_CALL ControlPanel::onSetLightColor( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightColor        = *(float3*)value;
    Settings::s_settings.debug.lightColorChanged = true;
}

void TW_CALL ControlPanel::onSetLightIntensity( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightIntensity        = *(float*)value;
    Settings::s_settings.debug.lightIntensityChanged = true;
}

void TW_CALL ControlPanel::onSetLightEmitterRadius( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightEmitterRadius        = *(float*)value;
    Settings::s_settings.debug.lightEmitterRadiusChanged = true;
}

void TW_CALL ControlPanel::onSetLightLinearAttenuationFactor( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightLinearAttenuationFactor        = *(float*)value;
    Settings::s_settings.debug.lightLinearAttenuationFactorChanged = true;
}

void TW_CALL ControlPanel::onSetLightQuadraticAttenuationFactor( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.lightQuadraticAttenuationFactor        = *(float*)value;
    Settings::s_settings.debug.lightQuadraticAttenuationFactorChanged = true;
}

void TW_CALL ControlPanel::onGetBool( void* value, void* boolVal )
{
    *(float*)value = *(float*)boolVal;
}

void TW_CALL ControlPanel::onGetFloat( void* value, void* floatVal )
{
    *(float*)value = *(float*)floatVal;
}

void TW_CALL ControlPanel::onGetFloat3( void* value, void* float3Val )
{
    *(float3*)value = *(float3*)float3Val;
}

void TW_CALL ControlPanel::onFlipUVs( void* controlPanel )
{
    if (!controlPanel)
        return;

    ((ControlPanel*)controlPanel)->m_sceneManager.flipTexcoordsVerticallyAndResaveMesh();
}

void TW_CALL ControlPanel::onFlipTangents( void* controlPanel )
{
    if ( !controlPanel )
        return;

    ( (ControlPanel*)controlPanel )->m_sceneManager.flipTangentsAndResaveMesh();
}

void TW_CALL ControlPanel::onFlipNormals( void* controlPanel )
{
    if ( !controlPanel )
        return;

    ( (ControlPanel*)controlPanel )->m_sceneManager.flipNormalsAndResaveMesh();
}

void TW_CALL ControlPanel::onInvertVertexWindingOrder( void* controlPanel )
{
    if ( !controlPanel )
        return;

    ( (ControlPanel*)controlPanel )->m_sceneManager.invertVertexWindingOrderAndResaveMesh();
}

void TW_CALL ControlPanel::onDisplayNextStageProfilingReflection( void* clientData )
{
    clientData; // Unused.

    Settings::modify().profiling.display.startWithStage = 
        getNextRenderingStage( settings().profiling.display.startWithStage, RenderingStageType::Reflection );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onDisplayNextStageProfilingTransmission( void* clientData )
{
    clientData; // Unused.

    Settings::modify().profiling.display.startWithStage = 
        getNextRenderingStage( settings().profiling.display.startWithStage, RenderingStageType::Transmission );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onDisplayPrevStageProfiling( void* clientData )
{
    clientData; // Unused.

    Settings::modify().profiling.display.startWithStage = 
        getPrevRenderingStage( settings().profiling.display.startWithStage );

    Settings::onChanged();
}
