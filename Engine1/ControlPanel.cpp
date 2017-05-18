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

void TW_CALL ControlPanel::onNextLevelReflection( void *clientData )
{
    Settings::modify().rendering.reflectionsRefractions.activeView.push_back( true );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onNextLevelRefraction( void *clientData )
{
    Settings::modify().rendering.reflectionsRefractions.activeView.push_back( false );

    Settings::onChanged();
}

void TW_CALL ControlPanel::onPrevLevel( void *clientData )
{
    if ( !settings().rendering.reflectionsRefractions.activeView.empty() )
        Settings::modify().rendering.reflectionsRefractions.activeView.pop_back();

    Settings::onChanged();
}
