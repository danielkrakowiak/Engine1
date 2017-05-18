#include "Settings.h"

using namespace Engine1;

Settings Settings::s_settings;

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
    return s_settings; 
}

void Settings::setDefault()
{
    main.fullscreen       = false;
    main.screenDimensions = int2( 1024 /*1920*/, 768 /*1080*/ );
    main.verticalSync     = false;
    main.limitFPS         = false;
    main.displayFrequency = 60;
    main.screenColorDepth = 32;
    main.zBufferDepth     = 32;

    debug.debugRenderAlpha          = false;
    debug.debugWireframeMode        = false;
    debug.debugDisplayedMipmapLevel = 0;
    debug.renderText                = true;
    debug.renderFps                 = true;
    debug.slowmotionMode            = false;
    debug.snappingMode              = false;

    rendering.shadows.useSeparableShadowBlur = true;

    rendering.reflectionsRefractions.maxLevel = 1;
}