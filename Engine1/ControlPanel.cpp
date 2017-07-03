#include "ControlPanel.h"

#include <d3d11.h>
#include "SceneManager.h"

#include "Settings.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ControlPanel::ControlPanel( SceneManager& sceneManager ) :
    m_sceneManager( sceneManager )
{}

ControlPanel::~ControlPanel()
{
    TwTerminate();

    mainBar = nullptr;
}

void ControlPanel::initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device, const int2 windowDimensions )
{
    TwInit( TW_DIRECT3D11, device.Get() );

    TwWindowSize( windowDimensions.x, windowDimensions.y );

    mainBar = TwNewBar("Main");

    TwAddVarRW( mainBar, "Use separable shadow blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowBlur, "" );
    TwAddVarRW( mainBar, "Max level", TW_TYPE_INT32, &Settings::s_settings.rendering.reflectionsRefractions.maxLevel, "" );
    TwAddVarRW( mainBar, "Reflections", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.reflectionsEnabled, "" );
    TwAddVarRW( mainBar, "Refractions", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.refractionsEnabled, "" );

    TwAddButton( mainBar, "Next - reflection", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( mainBar, "Next - transmission", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( mainBar, "Back", ControlPanel::onPrevLevel, nullptr, "" );

    TwAddVarRW( mainBar, "Roughness blur mul", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.roughnessBlurMul, "min=0 max=1000 step=0.2 precision=1" );

    TwAddVarRW( mainBar, "Alpha mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.alphaMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( mainBar, "Emissive mul", TW_TYPE_COLOR3F, &Settings::s_settings.debug.emissiveMul, "colormode=hls" );
    TwAddVarRW( mainBar, "Albedo mul", TW_TYPE_COLOR3F, &Settings::s_settings.debug.albedoMul, "colormode=hls" );
    TwAddVarRW( mainBar, "Metalness mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.metalnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( mainBar, "Roughness mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.roughnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarRW( mainBar, "Refractive index mul", TW_TYPE_FLOAT, &Settings::s_settings.debug.refractiveIndexMul, "min=0 max=1 step=0.001 precision=3" );

    TwAddVarRW( mainBar, "Field of view", TW_TYPE_FLOAT, &Settings::s_settings.rendering.fieldOfViewDegress, "min=30 max=180 step=0.1 precision=1" );
    TwAddVarRW( mainBar, "Exposure", TW_TYPE_FLOAT, &Settings::s_settings.rendering.exposure, "min=-10 max=10 step=0.01 precision=2" );
    TwAddVarRW( mainBar, "Antialiasing", TW_TYPE_BOOL8, &Settings::s_settings.rendering.antialiasing, "" );

    meshUtilsBar = TwNewBar("Mesh Utils");

    TwAddButton( meshUtilsBar, "Flip UVs (vertically)", ControlPanel::onFlipUVs, this, "" );
    TwAddButton( meshUtilsBar, "Flip Tangents", ControlPanel::onFlipTangents, this, "" );
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
