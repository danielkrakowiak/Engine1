#pragma once

#include <memory>
#include <wrl.h>
#include "Texture2D.h"

#include "Direct3DDeferredRenderer.h"
#include "ASSAORenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "ReflectionRefractionShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "HitDistanceSearchRenderer.h"
#include "CombiningRenderer.h"
#include "TextureRescaleRenderer.h"
#include "RaytraceShadowRenderer.h"
#include "RasterizeShadowRenderer.h"
#include "ShadowMapRenderer.h"
#include "MipmapRenderer.h"
#include "DistanceToOccluderSearchRenderer.h"
#include "BlurShadowPatternRenderer.h"
#include "BlurShadowsRenderer.h"
#include "CombineShadowLayersRenderer.h"
#include "UtilityRenderer.h"
#include "BokehBlurRenderer.h"
#include "ExtractBrightPixelsRenderer.h"
#include "ToneMappingRenderer.h"
#include "AntialiasingRenderer.h"

#include "RenderingStage.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

namespace Engine1 
{
    class Direct3DRendererCore;
    class Profiler;
    class RenderTargetManager;
    class Scene;
    class Camera;
    class BlockMesh;
    class BlockModel;
    class BlockActor;
    class SkeletonMesh;
    class SkeletonModel;
    class SkeletonActor;
    class Light;
    class Selection;

    class Renderer
    {
        public:

        enum class View : char {
            Final = 0,
            ShadedCombined,
            Shaded,
            Depth,
            Position,
            Emissive,
            Albedo,
            Normal,
            Metalness,
            Roughness,
            IndexOfRefraction,
            AmbientOcclusion,
            RayDirections,
            Contribution,
            CurrentRefractiveIndex,
            Preillumination,
            HardShadow,
            MediumShadow,
            SoftShadow,
            SmoothedPatternHardShadows,
            SmoothedPatternMediumShadows,
            SmoothedPatternSoftShadows,
            BlurredHardShadows,
            BlurredMediumShadows,
            BlurredSoftShadows,
            BlurredShadows,
            SpotlightDepth,
            DistanceToOccluderHardShadow,
            DistanceToOccluderMediumShadow,
            DistanceToOccluderSoftShadow,
            FinalDistanceToOccluderHardShadow,
            FinalDistanceToOccluderMediumShadow,
            FinalDistanceToOccluderSoftShadow,
            MaxIlluminationBlurRadiusInScreenSpace,
            MinIlluminationBlurRadiusInWorldSpace,
            MaxIlluminationBlurRadiusInWorldSpace,
            BloomBrightPixels,
            HitDistance,
            HitDistanceBlurred,
            HitDistanceToCamera,
			Test
        };

        static std::string viewToString( const View view );

        struct Output
        {
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > ucharImage;
            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        uchar4Image;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        float4Image;
            std::shared_ptr< RenderTargetTexture2D< float2 > >        float2Image;
            std::shared_ptr< RenderTargetTexture2D< float  > >        floatImage;
            std::shared_ptr< DepthTexture2D< uchar4 > >               depthUchar4Image;

            bool isEmpty()
            {
                return !ucharImage && !uchar4Image && !float4Image && !float2Image && !floatImage && !depthUchar4Image;
            }

            void reset()
            {
                ucharImage       = nullptr;
                uchar4Image      = nullptr;
                float4Image      = nullptr;
                float2Image      = nullptr;
                floatImage       = nullptr;
                depthUchar4Image = nullptr;
            }
        };

        struct LayerRenderTargets
        {
            LayerRenderTargets()
            {
                reset();
            }

            void reset()
            {
                contributionRoughness  = nullptr;
                rayOrigin              = nullptr;
                rayDirection           = nullptr;
                hitPosition            = nullptr;
                hitEmissive            = nullptr;
                hitAlbedo              = nullptr;
                hitMetalness           = nullptr;
                hitRoughness           = nullptr;
                hitNormal              = nullptr;
                hitRefractiveIndex     = nullptr;
                currentRefractiveIndex = nullptr;
                depth                  = nullptr;
                hitDistance            = nullptr;
                hitDistanceBlurred     = nullptr;
                hitDistanceToCamera    = nullptr;
                hitShaded              = nullptr;
                shadedCombined         = nullptr;
                ambientOcclusion       = nullptr;
            }

            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        contributionRoughness;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        rayOrigin;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        rayDirection;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        hitPosition;
            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        hitEmissive;
            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        hitAlbedo;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > hitMetalness;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > hitRoughness;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        hitNormal;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > hitRefractiveIndex;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > currentRefractiveIndex;
            std::shared_ptr< DepthTexture2D< uchar4 > >               depth;
            std::shared_ptr< RenderTargetTexture2D< float > >         hitDistance;
            std::shared_ptr< RenderTargetTexture2D< float > >         hitDistanceBlurred;
            std::shared_ptr< RenderTargetTexture2D< float > >         hitDistanceToCamera;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        hitShaded;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        shadedCombined;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > ambientOcclusion;
        };

        Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler, RenderTargetManager& renderTargetManager );
        ~Renderer();

        void initialize( 
            const int2 imageDimensions, 
            Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
            Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext,
            std::shared_ptr<const BlockModel> lightModel 
        );

		void renderShadowMaps( const Scene& scene );

        Output renderScene( 
            const Scene& scene, const Camera& camera,
            const bool wireframeMode,
            const Selection& selection,
            const std::shared_ptr< BlockMesh > selectionVolumeMesh 
        );

        void renderText( 
            const std::string& text, 
            Font& font, 
            float2 position, 
            float4 color,
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > colorRenderTarget
        );

        void setActiveViewType( const View view );
        View getActiveViewType() const;
        
        float getExposure();
        void  setExposure( const float exposure );

        // Temporary - for debug.
        const std::vector< std::shared_ptr< Texture2D< unsigned char > > > debugGetCurrentRefractiveIndexTextures();

        Microsoft::WRL::ComPtr< ID3D11Device3 >        getDevice() const        { return m_device; }
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > getDeviceContext() const { return m_deviceContext; };

        private:

        Output renderPrimaryLayer( 
            const Scene& scene, const Camera& camera, 
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const RenderingStage debugViewStage, const View debugViewType,
            const bool wireframeMode,
            const Selection& selection,
            const std::shared_ptr< BlockMesh > selectionVolumeMesh 
        );

        Output renderSecondaryLayers( 
            const int maxLevelCount, const Camera& camera,
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const RenderingStage renderingStage,
            const RenderingStage debugViewStage,
            const View debugViewType
        );

        void renderSecondaryLayer(
            const RenderingStage renderingStage, const Camera& camera,
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows
        );

        void combineLayers( const RenderingStage renderingStage, const Camera& camera );

        void performBloom( 
            std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture, 
            std::shared_ptr< Texture2D< float4 > > colorTexture, 
            const float minBrightness );

        void performToneMapping( 
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > dstTexture,
            std::shared_ptr< Texture2D< float4 > > srcTexture,
            const float exposure 
        );

        void performAntialiasing( 
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > dstTexture,
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > srcTexture
        );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        View m_debugViewType;

        int2 m_imageDimensions;

        // Tone mapping configuration.
        float m_exposure;

        // Bloom configuration.
        float m_minBrightness;

        Direct3DRendererCore&     m_rendererCore;
        Profiler&                 m_profiler;
        RenderTargetManager&      m_renderTargetManager;

        Direct3DDeferredRenderer            m_deferredRenderer;
        ASSAORenderer                       m_ASSAORenderer;
        RaytraceRenderer                    m_raytraceRenderer;
        ShadingRenderer                     m_shadingRenderer;
        ReflectionRefractionShadingRenderer m_reflectionRefractionShadingRenderer;
        EdgeDetectionRenderer               m_edgeDetectionRenderer;
        HitDistanceSearchRenderer           m_hitDistanceSearchRenderer;
        CombiningRenderer                   m_combiningRenderer;
        TextureRescaleRenderer              m_textureRescaleRenderer;
        RasterizeShadowRenderer             m_rasterizeShadowRenderer;
		RaytraceShadowRenderer              m_raytraceShadowRenderer;
		ShadowMapRenderer                   m_shadowMapRenderer;
        MipmapRenderer                      m_mipmapRenderer;
        DistanceToOccluderSearchRenderer    m_distanceToOccluderSearchRenderer;
        BlurShadowPatternRenderer           m_blurShadowPatternRenderer;
        BlurShadowsRenderer                 m_blurShadowsRenderer;
        CombineShadowLayersRenderer         m_combineShadowLayersRenderer;
        UtilityRenderer                     m_utilityRenderer;
        ExtractBrightPixelsRenderer         m_extractBrightPixelsRenderer;
        ToneMappingRenderer                 m_toneMappingRenderer;
        AntialiasingRenderer                m_antialiasingRenderer;
        BokehBlurRenderer                   m_bokehBlurRenderer;

        std::vector< LayerRenderTargets > m_layersRenderTargets;

        std::shared_ptr<const BlockModel> m_lightModel;

        Output getLayerRenderTarget( View view, int level );

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

