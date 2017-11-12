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
    if (s_modified)
        onChanged();

    return s_settings; 
}

Settings& Settings::modify()
{
    if ( s_modified )
        onChanged();

    s_modified = true;

    return s_settings;
}

void Settings::initializeInternal()
{
    s_settings.main.fullscreen       = false;
    s_settings.main.screenDimensions = int2( 1024 /*1920*/, 768 /*1080*/ );
    s_settings.main.verticalSync     = false;
    s_settings.main.limitFPS         = false;
    s_settings.main.displayFrequency = 60;
    s_settings.main.screenColorDepth = 32;
    s_settings.main.zBufferDepth     = 32;

    s_settings.debug.debugRenderAlpha          = false;
    s_settings.debug.debugWireframeMode        = false;
    s_settings.debug.debugDisplayedMipmapLevel = 0;
    s_settings.debug.renderText                = true;
    s_settings.debug.renderFps                 = true;
    s_settings.debug.slowmotionMode            = false;
    s_settings.debug.snappingMode              = false;

    s_settings.debug.replaceSelected = true;

    s_settings.debug.alphaMulChanged           = false;
    s_settings.debug.emissiveMulChanged        = false;
    s_settings.debug.albedoMulChanged          = false;
    s_settings.debug.metalnessMulChanged       = false;
    s_settings.debug.roughnessMulChanged       = false;
    s_settings.debug.refractiveIndexMulChanged = false;

    s_settings.debug.alphaMul                = 1.0f;
    s_settings.debug.emissiveMul             = float3( 1.0f, 1.0f, 1.0f );
    s_settings.debug.albedoMul               = float3( 1.0f, 1.0f, 1.0f );
    s_settings.debug.metalnessMul            = 1.0f;
    s_settings.debug.roughnessMul            = 1.0f;
    s_settings.debug.refractiveIndexMul      = 1.0f;

    
    s_settings.debug.lightEnabled                    = true;
    s_settings.debug.lightCastShadows                = true;
    s_settings.debug.lightColor                      = float3::ONE;
    s_settings.debug.lightIntensity                  = 1.0f;
    s_settings.debug.lightEmitterRadius              = 0.0f;
    s_settings.debug.lightLinearAttenuationFactor    = 1.0f;
    s_settings.debug.lightQuadraticAttenuationFactor = 0.0f;

    s_settings.debug.lightEnabledChanged                    = false;
    s_settings.debug.lightCastShadowsChanged                = false;
    s_settings.debug.lightColorChanged                      = false;
    s_settings.debug.lightIntensityChanged                  = false;
    s_settings.debug.lightEmitterRadiusChanged              = false;
    s_settings.debug.lightLinearAttenuationFactorChanged    = false;
    s_settings.debug.lightQuadraticAttenuationFactorChanged = false;

    s_settings.debug.lightColorChanged = false;

    s_settings.rendering.fieldOfViewDegress = 70.0f;
    s_settings.rendering.exposure           = 1.0f;
    s_settings.rendering.antialiasing       = true;

    s_settings.rendering.shadows.enabled                = true;
    s_settings.rendering.shadows.useSeparableShadowBlur = true;

    s_settings.rendering.shadows.distanceToOccluderSearch.maxDistToOccluder = 500.0f;

    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.positionThreshold         = 0.2f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.normalThreshold           = 0.7f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight       = 5.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight         = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow      = 2.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow        = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel          = 2;
    s_settings.rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider   = 4;

    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.positionThreshold       = 0.2f;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.normalThreshold         = 0.7f;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight     = 10.0f; 
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight       = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow    = 5.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow      = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel        = 3;
    s_settings.rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider = 8;

    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.positionThreshold         = 0.2f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.normalThreshold           = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight       = 10.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight         = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow      = 7.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow        = 1.0f;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel          = 4;
    s_settings.rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider   = 16;

    s_settings.rendering.shadows.blur.hardShadows.positionThreshold   = 0.05f;
    s_settings.rendering.shadows.blur.hardShadows.normalThreshold     = 0.08f;
    s_settings.rendering.shadows.blur.mediumShadows.positionThreshold = 0.08f;
    s_settings.rendering.shadows.blur.mediumShadows.normalThreshold   = 0.08f;
    s_settings.rendering.shadows.blur.softShadows.positionThreshold   = 0.12f;
    s_settings.rendering.shadows.blur.softShadows.normalThreshold     = 0.08f;

    s_settings.rendering.reflectionsRefractions.maxLevel           = 1;
    s_settings.rendering.reflectionsRefractions.reflectionsEnabled = false;
    s_settings.rendering.reflectionsRefractions.refractionsEnabled = false;
    s_settings.rendering.reflectionsRefractions.roughnessBlurMul   = 60.0f;

    s_settings.rendering.hitDistanceSearch.resolutionDivider = 4;

    s_settings.rendering.combining.positionDiffMul         = 6.0f;
    s_settings.rendering.combining.normalDiffMul           = 3.0f;
    s_settings.rendering.combining.positionNormalThreshold = 1.2f;

    s_settings.physics.fixedStepDuration = 1.0f / 60.0f;

    s_settings.importer.defaultWhiteUchar4TextureFileName = "default_white_uchar4.png";
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

        s_settings.textures.defaults.alpha = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
            ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        s_settings.textures.defaults.metalness = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
            ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        s_settings.textures.defaults.roughness = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
            ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        s_settings.textures.defaults.refractiveIndex = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
            ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

        s_settings.textures.defaults.emissive = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
            ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

        s_settings.textures.defaults.albedo = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
            ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

        s_settings.textures.defaults.normal = 
            std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
            ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
    }
}

void Settings::onChanged()
{
    // Active-level should not exceed max-level.
    while ( s_settings.rendering.reflectionsRefractions.activeView.size() > s_settings.rendering.reflectionsRefractions.maxLevel )
        s_settings.rendering.reflectionsRefractions.activeView.pop_back();

    // Max-level has to be >= 0.
    s_settings.rendering.reflectionsRefractions.maxLevel
        = std::max( 0, s_settings.rendering.reflectionsRefractions.maxLevel );

    // Max-level has to be <= 8.
    s_settings.rendering.reflectionsRefractions.maxLevel
        = std::min( 8, s_settings.rendering.reflectionsRefractions.maxLevel );

    s_modified = false;
}