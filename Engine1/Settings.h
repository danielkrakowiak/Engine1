#pragma once

#include "int2.h"
#include "float3.h"
#include <vector>

namespace Engine1
{
    class Settings
    {
        // Only Application and ControlPanel classes can modify the settings.
        friend class Application;
        friend class ControlPanel;

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

            float  alphaMul;
            float3 emissiveMul;
            float3 albedoMul;
            float  metalnessMul;
            float  roughnessMul;
            float  refractiveIndexMul;
        } debug;

        struct Rendering 
        {
            float fieldOfViewDegress;
            float exposure;
            bool  antialiasing;

            struct Shadows
            {
                // Debug option to enable/disable blurring shadows in two passes - horizontal and vertical.
                // It reduces blurring complexity from n^2 to 2n, where n is blurring kernel size.
                // But it's not mathematically correct (because of variable levels of blur per pixel) so may lead to some artifacts.
                bool useSeparableShadowBlur;
            } shadows;

            struct ReflectionsRefractions
            {
                int maxLevel; // 0 - no reflections or refractions.
                // true - reflection, false - refraction. Number of elements defines the current view level.
                std::vector< bool > activeView; 
                bool reflectionsEnabled;
                bool refractionsEnabled;
            } reflectionsRefractions;
        } rendering;

        private:

        // Use this method to modify settings - it marks settings as changed.
        // Note: A bit wasteful - calling it many times causes recalculation 
        // of settings each time - because when we modify them we can also read them and they need to be up-to-date.
        static Settings& modify();

        static void setDefault();
        static void onChanged();

        // These should not be changed from outside of this class - even by friend classes.
        // Instead "modify()" method should be used to change settings.
        static Settings s_settings;
        static bool     s_modified;
    };

    // A global, shortcut method to get settings more easily.
    const Settings& settings();
}

