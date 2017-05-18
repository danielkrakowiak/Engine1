#pragma once

#include "int2.h"

namespace Engine1
{
    class Settings
    {
        // Only Application class can modify the settings.
        friend class Application;

        public:

        Settings();
        ~Settings();

        static const Settings& get();

        struct Main
        {
            bool fullscreen;
            int2 screenDimensions;
            bool verticalSync;
            bool limitFPS;
            int  displayFrequency;
            char screenColorDepth;
            char zBufferDepth;
        } main;

        struct Debug
        {
            bool debugRenderAlpha;
            bool debugWireframeMode;
            int  debugDisplayedMipmapLevel;
            bool renderText;
            bool renderFps;

            bool slowmotionMode;
            bool snappingMode;
        } debug;

        struct Rendering 
        {
            struct Shadows
            {
                bool useSeparableShadowBlur;
            } shadows;

            struct ReflectionsRefractions
            {
                int maxLevel; // 0 - no reflections or refractions.
            } reflectionsRefractions;
        } rendering;

        private:

        void setDefault();

        static Settings s_settings;
    };

    // A global, shortcut method to get settings more easily.
    const Settings& settings();
}

