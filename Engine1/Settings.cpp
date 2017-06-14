#include "Settings.h"

#include <algorithm>
#include "MathUtil.h"

using namespace Engine1;

Settings Settings::s_settings;
bool     Settings::s_modified = false;

const Settings& Engine1::settings()
{
    return Settings::get();
}

Settings::Settings()
{
    setDefault();
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

void Settings::setDefault()
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

    s_settings.debug.alphaMul           = 1.0f;
    s_settings.debug.emissiveMul        = float3( 1.0f, 1.0f, 1.0f );
    s_settings.debug.albedoMul          = float3( 1.0f, 1.0f, 1.0f );
    s_settings.debug.metalnessMul       = 1.0f;
    s_settings.debug.roughnessMul       = 1.0f;
    s_settings.debug.refractiveIndexMul = 1.0f;

    s_settings.rendering.fieldOfViewDegress = 70.0f;
    s_settings.rendering.exposure           = 0.5f;
    s_settings.rendering.antialiasing       = true;

    s_settings.rendering.shadows.useSeparableShadowBlur = true;

    s_settings.rendering.reflectionsRefractions.maxLevel = 1;
}

void Settings::onChanged()
{
    // Active-level should not exceed max-level.
    while ( s_settings.rendering.reflectionsRefractions.activeView.size() > s_settings.rendering.reflectionsRefractions.maxLevel )
        s_settings.rendering.reflectionsRefractions.activeView.pop_back();

    // Max-level has to be >= 0.
    s_settings.rendering.reflectionsRefractions.maxLevel
        = std::max( 0, s_settings.rendering.reflectionsRefractions.maxLevel );

    // Max-level has to be <= 5.
    s_settings.rendering.reflectionsRefractions.maxLevel
        = std::min( 5, s_settings.rendering.reflectionsRefractions.maxLevel );

    s_modified = false;
}