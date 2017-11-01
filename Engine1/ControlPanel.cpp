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

    TwAddVarRW( m_mainBar, "Use separable shadow blur", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.useSeparableShadowBlur, "" );
    TwAddVarRW( m_mainBar, "Max level", TW_TYPE_INT32, &Settings::s_settings.rendering.reflectionsRefractions.maxLevel, "" );
    TwAddVarRW( m_mainBar, "Reflections", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.reflectionsEnabled, "" );
    TwAddVarRW( m_mainBar, "Refractions", TW_TYPE_BOOL8, &Settings::s_settings.rendering.reflectionsRefractions.refractionsEnabled, "" );
    TwAddVarRW( m_mainBar, "Shadows", TW_TYPE_BOOL8, &Settings::s_settings.rendering.shadows.enabled, "" );

    TwAddButton( m_mainBar, "Next - reflection", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( m_mainBar, "Next - transmission", ControlPanel::onNextLevelReflection, nullptr, "" );
    TwAddButton( m_mainBar, "Back", ControlPanel::onPrevLevel, nullptr, "" );

    TwAddButton( m_mainBar, "Reset camera", ControlPanel::onResetCamera, this, "" );

    TwAddVarRW( m_mainBar, "Roughness blur mul", TW_TYPE_FLOAT, &Settings::s_settings.rendering.reflectionsRefractions.roughnessBlurMul, "min=0 max=1000 step=0.2 precision=1" );

    TwAddVarRW( m_mainBar, "Replace selected", TW_TYPE_BOOL8, &Settings::s_settings.debug.replaceSelected, "" );

    TwAddVarCB( m_mainBar, "Alpha mul", TW_TYPE_FLOAT, ControlPanel::onSetAlphaMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.alphaMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Emissive mul", TW_TYPE_COLOR3F, ControlPanel::onSetEmissiveMul, ControlPanel::onGetFloat3, &Settings::s_settings.debug.emissiveMul, "colormode=hls" );
    TwAddVarCB( m_mainBar, "Albedo mul", TW_TYPE_COLOR3F, ControlPanel::onSetAlbedoMul, ControlPanel::onGetFloat3, &Settings::s_settings.debug.albedoMul, "colormode=hls" );
    TwAddVarCB( m_mainBar, "Metalness mul", TW_TYPE_FLOAT, ControlPanel::onSetMetalnessMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.metalnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Roughness mul", TW_TYPE_FLOAT, ControlPanel::onSetRoughnessMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.roughnessMul, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_mainBar, "Refractive index mul", TW_TYPE_FLOAT, ControlPanel::onSetRefractiveIndexMul, ControlPanel::onGetFloat, &Settings::s_settings.debug.refractiveIndexMul, "min=0 max=1 step=0.001 precision=3" );

    TwAddVarRW( m_mainBar, "Field of view", TW_TYPE_FLOAT, &Settings::s_settings.rendering.fieldOfViewDegress, "min=30 max=180 step=0.1 precision=1" );
    TwAddVarRW( m_mainBar, "Exposure", TW_TYPE_FLOAT, &Settings::s_settings.rendering.exposure, "min=-10 max=10 step=0.01 precision=2" );
    TwAddVarRW( m_mainBar, "Antialiasing", TW_TYPE_BOOL8, &Settings::s_settings.rendering.antialiasing, "" );

    m_meshUtilsBar = TwNewBar("Mesh Utils");

    TwAddButton( m_meshUtilsBar, "Flip UVs (vertically)", ControlPanel::onFlipUVs, this, "" );
    TwAddButton( m_meshUtilsBar, "Flip Tangents", ControlPanel::onFlipTangents, this, "" );
    TwAddButton( m_meshUtilsBar, "Flip Normals", ControlPanel::onFlipNormals, this, "" );
    TwAddButton( m_meshUtilsBar, "Invert vertex winding order", ControlPanel::onInvertVertexWindingOrder, this, "" );

    m_lightBar = TwNewBar("Light");

    TwAddVarCB( m_lightBar, "Enabled", TW_TYPE_BOOL8, ControlPanel::onSetLightEnabled, ControlPanel::onGetBool, &Settings::s_settings.debug.lightEnabled, "" );
    TwAddVarCB( m_lightBar, "Cast shadows", TW_TYPE_BOOL8, ControlPanel::onSetLightCastShadows, ControlPanel::onGetBool, &Settings::s_settings.debug.lightCastShadows, "" );
    TwAddVarCB( m_lightBar, "Color", TW_TYPE_COLOR3F, ControlPanel::onSetLightColor, ControlPanel::onGetFloat3, &Settings::s_settings.debug.lightColor, "colormode=hls" );
    TwAddVarCB( m_lightBar, "Intensity", TW_TYPE_FLOAT, ControlPanel::onSetLightIntensity, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightIntensity, "min=0 max=10000 step=0.05 precision=2" );
    TwAddVarCB( m_lightBar, "Emitter radius", TW_TYPE_FLOAT, ControlPanel::onSetLightEmitterRadius, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightEmitterRadius, "min=0 max=1 step=0.01 precision=2" );
    TwAddVarCB( m_lightBar, "Linear attenuation factor", TW_TYPE_FLOAT, ControlPanel::onSetLightLinearAttenuationFactor, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightLinearAttenuationFactor, "min=0 max=1 step=0.001 precision=3" );
    TwAddVarCB( m_lightBar, "Linear quadratic factor", TW_TYPE_FLOAT, ControlPanel::onSetLightQuadraticAttenuationFactor, ControlPanel::onGetFloat, &Settings::s_settings.debug.lightQuadraticAttenuationFactor, "min=0 max=1 step=0.001 precision=3" );
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

void TW_CALL ControlPanel::onResetCamera( void* controlPanel )
{
    ( (ControlPanel*)controlPanel )->m_sceneManager.getCamera().setPosition( float3::ZERO );
}

void TW_CALL ControlPanel::onSetAlphaMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.alphaMul        = *(float*)value;
    Settings::s_settings.debug.alphaMulChanged = true;
}

void TW_CALL ControlPanel::onSetEmissiveMul( const void* value, void* /*clientData*/ )
{
    Settings::s_settings.debug.emissiveMul        = *(float3*)value;
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
