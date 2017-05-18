#include "ControlPanel.h"

#include <d3d11.h>

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

    //TwAddVarRW( tweakBar, "Use separable shadow blur", TW_TYPE_BOOL8,  )
}

int ControlPanel::processInput( void *wnd, unsigned int msg, unsigned __int64 _W64 wParam, __int64 _W64 lParam )
{
    return TwEventWin( wnd, msg, wParam, lParam );
}

void ControlPanel::draw()
{
    TwDraw();
}
