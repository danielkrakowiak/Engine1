#include "ControlPanel.h"

#include <d3d11.h>

#include "Settings.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ControlPanel::ControlPanel()
{}

ControlPanel::~ControlPanel()
{
    TwTerminate();

    tweakBar = nullptr;
}

void ControlPanel::initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device, const int2 windowDimensions )
{
    TwInit( TW_DIRECT3D11, device.Get() );

    TwWindowSize( windowDimensions.x, windowDimensions.y );

    tweakBar = TwNewBar("main");

    TwAddVarRW( tweakBar, "Use separable shadow blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowBlur, "" );
    TwAddVarRW( tweakBar, "Max level", TW_TYPE_INT32, &Settings::s_settings.rendering.reflectionsRefractions.maxLevel, "" );

    TwAddButton( tweakBar, "Next - reflection", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( tweakBar, "Next - transmission", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( tweakBar, "Back", ControlPanel::onPrevLevel, nullptr, "" );

    TwAddVarRW( tweakBar, "Alpha mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.alphaMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( tweakBar, "Emissive mul", TW_TYPE_COLOR3F, &Settings::s_settings.debug.emissiveMul, "colormode=hls" );
    TwAddVarRW( tweakBar, "Albedo mul", TW_TYPE_COLOR3F, &Settings::s_settings.debug.albedoMul, "colormode=hls" );
    TwAddVarRW( tweakBar, "Metalness mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.metalnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( tweakBar, "Roughness mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.roughnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( tweakBar, "Refractive index mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.refractiveIndexMul, "min=0 max=1 step=0.001 precision=3" );

    TwAddVarRW( tweakBar, "Field of view", TW_TYPE_FLOAT, &Settings::s_settings.rendering.fieldOfViewDegress, "min=30 max=180 step=0.1 precision=1" );
    TwAddVarRW( tweakBar, "Exposure", TW_TYPE_FLOAT, &Settings::s_settings.rendering.exposure, "min=-10 max=10 step=0.01 precision=2" );
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
    Settings::modify().rendering.reflectionsRefractions.activeView.push_back( true );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onNextLevelRefraction( void* /*clientData*/ )
{
    Settings::modify().rendering.reflectionsRefractions.activeView.push_back( false );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onPrevLevel( void* /*clientData*/ )
{
    if ( !settings().rendering.reflectionsRefractions.activeView.empty() )
        Settings::modify().rendering.reflectionsRefractions.activeView.pop_back();

    Settings::onChanged();
}
