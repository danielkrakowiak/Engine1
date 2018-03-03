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
            float  emissiveMul;
            float3 emissiveBaseMul;
            float3 albedoMul;
            float  metalnessMul;
            float  roughnessMul;
            float  refractiveIndexMul;

            bool   lightEnabled;
            bool   lightCastShadows;
            float3 lightColor;
            float  lightIntensity;
            float  lightEmitterRadius;
            float  lightLinearAttenuationFactor;
            float  lightQuadraticAttenuationFactor;

            bool lightEnabledChanged;
            bool lightCastShadowsChanged;
            bool lightColorChanged;
            bool lightIntensityChanged;
            bool lightEmitterRadiusChanged;
            bool lightLinearAttenuationFactorChanged;
            bool lightQuadraticAttenuationFactorChanged;
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

                struct Raytracing
                {
                    struct Layers
                    {
                        // Decide on how to split shadow into layers based on screen-space blur radius (in pixels).
                        // Values below hard-threshold go to hard layer.
                        // Values between hard, soft thresholds go to medium layer.
                        // Values above soft threshold go to soft layer.
                        // Threshold width allows for splitting shadow smoothly between two layer in the transition area.
                        float hardLayerBlurRadiusThreshold;
                        float softLayerBlurRadiusThreshold;
                        float hardLayerBlurRadiusTransitionWidth;
                        float softLayerBlurRadiusTransitionWidth;
                        float distToOccluderHardLayerBlurRadiusTransitionWidth;
                        float distToOccluderSoftLayerBlurRadiusTransitionWidth;
                    } layers;
                } raytracing;

                struct DistanceToOccluderSearch
                {
                    // Dist-to-occluder values below that number are valid. 
                    // Values above that number should be interpreted as lack of value (no information about distance-to-occluder).
                    float maxDistToOccluder;

                    // Shadow processing is split into 3 layers - hard shadows, medium shadows, soft shadows -
                    // depending on the screen-space blur radius. For each layer shadow distance-to-occluder 
                    // is first blurred and spread from shadow areas to lit areas. 
                    // Below params define what is the blur/spread radius and what is the distance between samples.
                    // Increasing search-radius increases blur, spread distance from shadow to lit areas and decreases performance.
                    // Search-step is used to control performance - for larger search-radius larger search step can be used to save performance.
                    // Small search-radius small search-step is required to avoid under sampling artifacts.
                    // Instead of increasing search-step it can be a better idea to increase input-mipmap-level - to avoid skipping over some important pixels.
                    // Search-radius and search-step are given as pixel count at input-mipmap-level.
                    // Search-radius-in-shadow is used when original pixel has some reasonable value in dist-to-occluder texture (is in shadow),
                    // while search-radius-in-light is used when original pixel has huge value in dist-to-occluder texture (meaning no value, is in light). 
                    // Same applies to search-step-in-shadow/light).
                    // It's also best to match output texture dimensions to the input-mipmap dimensions. 
                    // Input-mipmap-level and output-dimensions-divider should probably be always matching. #TODO: For future refactor?
                    // Unit: pixels.

                    struct Setup
                    {
                        float positionThreshold;
                        float normalThreshold;
                        float searchRadiusInShadow;
                        float searchStepInShadow;
                        float searchRadiusInLight;
                        float searchStepInLight;
                        int   inputMipmapLevel;
                        int   outputDimensionsDivider;
                    };

                    Setup hardShadows;
                    Setup mediumShadows;
                    Setup softShadows;
                    
                } distanceToOccluderSearch;

                struct Blur
                {
                    struct Setup
                    {
                        // Thresholds and multipliers deciding how shadow samples are weighted.
                        // Sample weight depends on how much a sample differs (in terms of pos/normal) from the central sample (in blur kernel).
                        // Increasing a multiplier increases significance of position or normal difference - samples are more strongly "rejected" based on that criteria.
                        // Increasing threshold increases "acceptance" level of error between center sample and neighbors.
                        float positionThreshold;
                        float normalThreshold;
                    };

                    Setup hardShadows;
                    Setup mediumShadows;
                    Setup softShadows;
                } blur;
            } shadows;

            struct ReflectionsRefractions
            {
                int maxLevel; // 0 - no reflections or refractions.
                // true - reflection, false - refraction. Number of elements defines the current view level.
                std::vector< bool > activeView; 
                bool reflectionsEnabled;
                bool refractionsEnabled;

                // 1 - maximal quality, every sample is taken from zero mipmap, ultra high number of samples (can kill performance).
                // 0 - worst quality - only one sample taken from the highest possible mipmap for desired blur level.
                // 0.666 - usual default.
                float samplingQuality;

                // Multiplier deciding how surface roughness translates to reflection/refraction blur kernel-size.
                float roughnessBlurMul;

                // How much reflections/refractions should be elongated longitudinally. 1 means no elongation.
                // The more elongated they are, the thinner they get laterally.
                float elongationMul;

                // Whether samples should be weighted in radial manner (outer samples have lower weight, Gaussian like).
                // It can improve quality, but reduce the amount of perceivable blur.
                bool radialBlurEnabled;

            } reflectionsRefractions;

            struct HitDistanceSearch
            {
                // Hit-dist-search can and should be performed at lower resolution to improve blur quality and performance.
                // This value decides at what resolution compared to screen the operation will be performed.
                // It was tested at 1/4 resolution of 1024x768 screen - so divider = 4 worked fine.
                int resolutionDivider; 

                // When blurring hit-distance, near-zero values usually get over blurred,
                // because of how easily they can be dominated by larger values. This option helps reducing that effect
                // and maintaining sharp reflections where object touches reflective surface.
                bool decreaseBlurForSmallValues;

                // At what hit-distance, decreasing blur should stop. 
                // The larger the value, the sharper the reflections.
                float maxHitDistForDecreasedBlur;
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

        struct Importer
        {
            // Used when an imported model has color multipliers, but no texture.
            // We then use default white texture (can be any small resolution white texture).
            std::string defaultWhiteUchar4TextureFileName;
        } importer;

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

