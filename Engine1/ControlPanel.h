#pragma once

#include <AntTweakBar/AntTweakBar.h>

#include <wrl.h>
#include "int2.h"
#include <memory>

struct ID3D11Device;

namespace Engine1
{
    class ControlPanel
    {
        public:

        ControlPanel();
        ~ControlPanel();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device, const int2 windowDimensions );

        int processInput( void *wnd, unsigned int msg, unsigned __int64 _W64 wParam, __int64 _W64 lParam );

        void draw();

        static void TW_CALL onNextLevelReflection( void *clientData );
        static void TW_CALL onNextLevelRefraction( void *clientData );
        static void TW_CALL onPrevLevel( void *clientData );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device > m_device;

        TwBar* tweakBar;
    };
}

