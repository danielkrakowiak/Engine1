#pragma once

#include <array>

#include "int2.h"
#include "Timer.h"

namespace Engine1
{
    class InputManager
    {
        public:

        InputManager();
        ~InputManager();

        void updateMouseState();
        
        void lockCursor( bool lock );
        bool isCursorLocked();
        bool isKeyPressed( unsigned int key );
        bool isMouseButtonPressed( unsigned int button );
        int2 getMouseMove();
        int2 getMousePos(); // Relative to top-left screen corner (not app window).

        void onKeyboardButton( int key, bool pressed );
        void onMouseButton( int key, bool pressed );

        class Keys
        {
            public:

            static const unsigned int back     = 8;
            static const unsigned int tab      = 9;
            static const unsigned int enter    = 13;
            static const unsigned int shift    = 16;
            static const unsigned int ctrl     = 17;
            static const unsigned int capsLock = 20;
            static const unsigned int esc      = 27;
            static const unsigned int left     = 37;
            static const unsigned int up       = 38;
            static const unsigned int right    = 39;
            static const unsigned int down     = 40;
            static const unsigned int zero     = 48;
            static const unsigned int one      = 49;
            static const unsigned int two      = 50;
            static const unsigned int three    = 51;
            static const unsigned int four     = 52;
            static const unsigned int five     = 53;
            static const unsigned int six      = 54;
            static const unsigned int seven    = 55;
            static const unsigned int eight    = 56;
            static const unsigned int nine     = 57;

            static const unsigned int a = 65;
            static const unsigned int b = 66;
            static const unsigned int c = 67;
            static const unsigned int d = 68;
            static const unsigned int e = 69;
            static const unsigned int f = 70;
            static const unsigned int g = 71;
            static const unsigned int h = 72;
            static const unsigned int i = 73;
            static const unsigned int j = 74;
            static const unsigned int k = 75;
            static const unsigned int l = 76;
            static const unsigned int m = 77;
            static const unsigned int n = 78;
            static const unsigned int o = 79;
            static const unsigned int p = 80;
            static const unsigned int q = 81;
            static const unsigned int r = 82;
            static const unsigned int s = 83;
            static const unsigned int t = 84;
            static const unsigned int u = 85;
            static const unsigned int v = 86;
            static const unsigned int w = 87;
            static const unsigned int x = 88;
            static const unsigned int y = 89;
            static const unsigned int z = 90;

            static const unsigned int f1 = 112;
            static const unsigned int f2 = 113;
            static const unsigned int f3 = 114;
            static const unsigned int f4 = 115;
            static const unsigned int f5 = 116;
            static const unsigned int f6 = 117;
            static const unsigned int f7 = 118;
            static const unsigned int f8 = 119;
            static const unsigned int f9 = 120;
            static const unsigned int f10 = 121;
            static const unsigned int f11 = 122;
            static const unsigned int f12 = 123;

            static const unsigned int semicolon = 186;
            static const unsigned int plus = 187;
            static const unsigned int comma = 188;
            static const unsigned int minus = 189;
            static const unsigned int dot = 190;
            static const unsigned int slash = 191;
            static const unsigned int tilde = 192;

            static const unsigned int squareOpeningBracket = 219;
            static const unsigned int backslash = 220;
            static const unsigned int squareClosingBracket = 221;
            static const unsigned int apostrophe = 222;
        };

        class MouseButtons
        {
            public:

            static const unsigned int left   = 0;
            static const unsigned int middle = 1;
            static const unsigned int right  = 2;
        };

        private:
        static const unsigned int keyboardKeyCount = 256;
        std::array<bool, keyboardKeyCount> keyboardButtonState;

        static const unsigned int mouseButtonCount = 5;
        std::array<bool, mouseButtonCount> mouseButtonState;

        int2 mouseMove;
        int2 mousePrevPos;
        int2 mousePos;

        bool limitMouseMove;
        int2 maxMouseMove;

        bool lockCursorPos;
        int2 lockedCursorPos;

        //unsigned int inputTimerId;
        //unsigned int inputTimerInterval;

        Timer awaitMouseMoveLastTick;
        double awaitMouseMoveInterval; //in ms

        // Copying is not allowed.
        InputManager( const InputManager& ) = delete;
        InputManager& operator=(const InputManager&) = delete;

    };
}

