#pragma once

#include <vector>

#include "int2.h"
#include "float3.h"

#include "Texture2DTypes.h"
#include "RenderingStage.h"

struct ID3D11Device3;

namespace Engine1
{
    template< typename > class Texture2D;

    class Settings
    {
        // Only Application and ControlPanel classes can modify the settings.
        friend class Application;
        friend class EngineApplication;
        friend class ControlPanel;
        friend class Benchmark;
        friend class RenderingTester;

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

        struct Paths
        {
            std::string assets;
            std::string testAssets;
            struct RenderingTests
            {
                std::string references;
                std::string testCases;
                std::string results;
            } renderingTests;
        } paths;

        struct Debug
        {
            bool debugRenderAlpha;
            bool debugWireframeMode;
            int  debugDisplayedMipmapLevel;
            bool renderFps;
            bool renderText;
            bool renderLightSources;

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

        struct Animation
        {
            float cameraPlaybackSpeed;
            float lightsPlaybackSpeed;
            float actorsPlaybackSpeed;
        } animation;

        struct Rendering 
        {
            float fieldOfViewDegress;

            // Color used when ray doesn't hit any geometry.
            float3 skyColor;

            struct Optimization
            {
                bool useHalfFloatsForRayDirections;
                bool useHalfFloatsForNormals;
                bool useHalfFloatsForHitDistance;
                bool useHalfFLoatsForDistanceToOccluder;

                int distToOccluderPositionSampleMipmapLevel;
                int distToOccluderNormalSampleMipmapLevel;

                int blurShadowPatternShadowSampleMipmapLevel;
                int blurShadowPatternPositionSampleMipmapLevel;
                int blurShadowPatternNormalSampleMipmapLevel;

                int blurShadowsPositionSampleMipmapLevel;
                int blurShadowsNormalSampleMipmapLevel;

                // #TODO: Add the same settings for combining stage - separate for primary/secondary reflections
            } optimization;

            struct AmbientOcclusion
            {
                struct ASSAO
                {
                    bool  enabled;
                    float radius;                             // [0.0,  ~ ] World (view) space size of the occlusion sphere.
                    float shadowMultiplier;                   // [0.0, 5.0] Effect strength linear multiplier
                    float shadowPower;                        // [0.5, 5.0] Effect strength pow modifier
                    float shadowClamp;                        // [0.0, 1.0] Effect max limit (applied after multiplier but before blur)
                    float horizonAngleThreshold;              // [0.0, 0.2] Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)
                    float fadeOutFrom;                        // [0.0,  ~ ] Distance to start start fading out the effect.
                    float fadeOutTo;                          // [0.0,  ~ ] Distance at which the effect is faded out.
                    int   qualityLevel;                       // [ -1,  3 ] Effect quality; -1 - lowest (low, half res checkerboard), 0 - low, 1 - medium, 2 - high, 3 - very high / adaptive; each quality level is roughly 2x more costly than the previous, except the q3 which is variable but, in general, above q2.
                    float adaptiveQualityLimit;               // [0.0, 1.0] (only for Quality Level 3)
                    int   blurPassCount;                      // [  0,   6] Number of edge-sensitive smart blur passes to apply. Quality 0 is an exception with only one 'dumb' blur pass used.
                    float sharpness;                          // [0.0, 1.0] (How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges)
                    float temporalSupersamplingAngleOffset;   // [0.0,  PI] Used to rotate sampling kernel; If using temporal AA / supersampling, suggested to rotate by ( (frame%3)/3.0*PI ) or similar. Kernel is already symmetrical, which is why we use PI and not 2*PI.
                    float temporalSupersamplingRadiusOffset;  // [0.0, 2.0] Used to scale sampling kernel; If using temporal AA / supersampling, suggested to scale by ( 1.0f + (((frame%3)-1.0)/3.0)*0.1 ) or similar.
                    float detailShadowStrength;               // [0.0, 5.0] Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing).
                } assao;
            } ambientOcclusion;

            struct Shadows
            {
                bool enabled;

                // If enabled, rays at different pixels aim at different parts of area light.
                // Averaging values from neighboring pixels gives correct shadow value.
                // It makes shadow edges smooth, and fixes a lot of artifacts for large area lights.
                // If shadow averaging from neighbor pixels is done well, this gives huge quality improvements.
                bool enableAlteringRayDirection; 

                // When altering shadow ray directions, a pattern of bright and dark pixels appears in shadow texture.
                // This enables a pass which blurs this pattern into a smooth image.
                bool enableBlurShadowPattern;

                bool useSeparableShadowPatternBlur;

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

                struct BlurPattern
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
                } blurPattern;

                struct Blur
                {
                    // Can be used to reduce overall amount of blur applied to shadows.
                    // Added, because shadows generated using altered ray directions probably need less blur 
                    // than hard-edged shadows without altered ray directions.
                    // For hard edges shadows - value of 1 is default.
                    float radiusMultiplier;

                    struct SetupBlur
                    {
                        // Thresholds and multipliers deciding how shadow samples are weighted.
                        // Sample weight depends on how much a sample differs (in terms of pos/normal) from the central sample (in blur kernel).
                        // Increasing a multiplier increases significance of position or normal difference - samples are more strongly "rejected" based on that criteria.
                        // Increasing threshold increases "acceptance" level of error between center sample and neighbors.
                        float positionThreshold;
                        float normalThreshold;
                    };

                    SetupBlur hardShadows;
                    SetupBlur mediumShadows;
                    SetupBlur softShadows;
                } blur;
            } shadows;

            struct ReflectionsRefractions
            {
                int maxLevel; // 0 - no reflections or refractions.
                // true - reflection, false - refraction. Number of elements defines the current view level.
                RenderingStage debugViewStage; 
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

                // Temporarily added just to test how roughness will look if we raise hit-distance to different powers.
                // To test how roughness could impact blur radius depending on hit-distance.
                float debugHitDistPower;

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

            struct PostProcess
            {
                bool  bloom;
                float exposure;
                bool  antialiasing;
                struct DepthOfField
                {
                    bool enabled;

                    // Diameter of the hole through which the light enters the camera/sensors.
                    float apertureDiameter;

                    // Parameter of the lens - distance from lens at which refracted rays cross (towards the scene).
                    // Popular value is 50 mm, so 0.05 (in meters).
                    float focalLength;

                    // Distance from camera at which objects appear focused/sharp.
                    float cameraFocusDist;

                    // Whether focus should automatically be set at the clicked object.
                    bool setFocusAtClickedObject;

                    // Circle-of-confusion is often measured in meters, while we work with pixels.
                    // This multiplier allows calculatin CoC in pixels from CoC in meters.
                    // Could be seen as number of pixels per meter of image/display/monitor etc.
                    // Assuming 72 dpi - default value is 2834 (pixels per meter).
                    float coCMul;

                    // Max circle-of-confusion radius in pixels. In other words, maximal radius of a Bokeh.
                    float maxCoC;

                    // How much Bokeh from further pixels (from the camera) can influence closer pixels.
                    // Input to the Gaussian weighting function - can be interpreted as an acceptable relative difference between
                    // center-depth and sample-depth (sample is the source of Bokeh).
                    float relativeDepthThreshold; 
                } depthOfField;
            } postProcess;
        } rendering;

        struct Textures
        {
            struct Defaults
            {
                std::shared_ptr< Texture2D< unsigned char > > alpha;
                std::shared_ptr< Texture2D< uchar4 > >        emissive;
                std::shared_ptr< Texture2D< uchar4 > >        albedo;
                std::shared_ptr< Texture2D< uchar4 > >        normal;
                std::shared_ptr< Texture2D< unsigned char > > metalness;
                std::shared_ptr< Texture2D< unsigned char > > roughness;
                std::shared_ptr< Texture2D< unsigned char > > refractiveIndex;
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

        struct Profiling
        {
            struct Display
            {
                bool enabled;
                bool coloredByTimeTaken;

                RenderingStage startWithStage;
            } display;
        } profiling;

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

