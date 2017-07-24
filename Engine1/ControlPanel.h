#pragma once

#include <AntTweakBar/AntTweakBar.h>

#include <wrl.h>
#include "int2.h"
#include <memory>

struct ID3D11Device3;

namespace Engine1
{
    class SceneManager;

    class ControlPanel
    {
        public:

        ControlPanel( SceneManager& sceneManager );
        ~ControlPanel();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device, const int2 windowDimensions );

        int processInput( void *wnd, unsigned int msg, unsigned __int64 _W64 wParam, __int64 _W64 lParam );

        void draw();

        static void TW_CALL onNextLevelReflection( void* clientData );
        static void TW_CALL onNextLevelRefraction( void* clientData );
        static void TW_CALL onPrevLevel( void* clientData );
        static void TW_CALL onFlipUVs( void* controlPanel );
        static void TW_CALL onFlipTangents( void* controlPanel );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device3 > m_device;

        TwBar* mainBar;
        TwBar* meshUtilsBar;

        SceneManager& m_sceneManager;
    };
}

