#pragma once

#include <vector>

#include "int2.h"
#include "float3.h"

#include "Texture2D.h"

struct ID3D11Device3;

namespace Engine1
{
    template< TexUsage, TexBind, typename > class Texture2D;

    class Settings
    {
        // Only Application and ControlPanel classes can modify the settings.
        friend class Application;
        friend class EngineApplication;
        friend class ControlPanel;

        public:

        Settings();
        ~Settings();

        static const Settings& get();

        // A second initialization that may require some info from outside of Settings.
        static void initialize(ID3D11Device3& device);

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

            // Option used to avoid replacing textures/meshes
            // on selected models when you drag&drop an asset.
            // The drag&dropped texture will be applied only to models 
            // which don't have any textures of that type.
            bool replaceSelected;

            bool alphaMulChanged;
            bool emissiveMulChanged;
            bool albedoMulChanged;
            bool metalnessMulChanged;
            bool roughnessMulChanged;
            bool refractiveIndexMulChanged;

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
                bool enabled;

                // Debug option to enable/disable blurring shadows in two passes - horizontal and vertical.
                // It reduces blurring complexity from n^2 to 2n, where n is blurring kernel size.
                // But it's not mathematically correct (because of variable levels of blur per pixel) so may lead to some artifacts.
                bool useSeparableShadowBlur;

                // Shadow processing is split into 3 layers - hard shadows, medium shadows, soft shadows -
                // depending on the screen-space blur radius. For each layer shadow distance-to-occluder 
                // is first blurred and spread from shadow areas to lit areas. 
                // Below params define what is the blur/spread radius and what is the distance between samples.
                // Increasing search-radius increases blur, spread distance from shadow to lit areas and decreases performance.
                // Search-step is used to control performance - for larger search-radius larger search step can be used to save performance.
                // Small search-radius small search-step is required to avoid under sampling artifacts.
                // Unit: pixels.
                float distanceToOccluderSearchRadiusForHardShadows;
                float distanceToOccluderSearchRadiusForMediumShadows;
                float distanceToOccluderSearchRadiusForSoftShadows;

                float distanceToOccluderSearchStepForHardShadows;
                float distanceToOccluderSearchStepForMediumShadows;
                float distanceToOccluderSearchStepForSoftShadows;
            } shadows;

            struct ReflectionsRefractions
            {
                int maxLevel; // 0 - no reflections or refractions.
                // true - reflection, false - refraction. Number of elements defines the current view level.
                std::vector< bool > activeView; 
                bool reflectionsEnabled;
                bool refractionsEnabled;

                // Multiplier deciding how surface roughness translates to reflection/refraction blur kernel-size.
                float roughnessBlurMul;

            } reflectionsRefractions;

            struct HitDistanceSearch
            {
                // Hit-dist-search can and should be performed at lower resolution to improve blur quality and performance.
                // This value decides at what resolution compared to screen the operation will be performed.
                // It was tested at 1/4 resolution of 1024x768 screen - so divider = 4 worked fine.
                int resolutionDivider; 
            } hitDistanceSearch;

            struct Combining
            {
                // Thresholds and multipliers deciding how reflection/refraction samples are weighted.
                // Sample weight depends on how much it differs (in terms of pos/normal) from the central sample (in blur kernel).
                // Increasing a multiplier increases significance of position or normal difference - samples are more strongly "rejected" based on that criteria.
                // Increasing threshold increases "acceptance" level of error between center sample and neighbors.
                float positionDiffMul;
                float normalDiffMul;
                float positionNormalThreshold;
            } combining;
        } rendering;

        struct Textures
        {
            struct Defaults
            {
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > alpha;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        emissive;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        albedo;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        normal;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > metalness;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > roughness;
                std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > refractiveIndex;
            } defaults;
        } textures;

        struct Physics
        {
            float fixedStepDuration; // In seconds.
        } physics;

        private:

        // Use this method to modify settings - it marks settings as changed.
        // Note: A bit wasteful - calling it many times causes recalculation 
        // of settings each time - because when we modify them we can also read them and they need to be up-to-date.
        static Settings& modify();

        static void initializeInternal();
        static void onChanged();

        // These should not be changed from outside of this class - even by friend classes.
        // Instead "modify()" method should be used to change settings.
        static Settings s_settings;
        static bool     s_modified;
    };

    // A global, shortcut method to get settings more easily.
    const Settings& settings();
}

