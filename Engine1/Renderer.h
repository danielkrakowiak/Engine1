#pragma once

#include <memory>
#include <wrl.h>
#include "Texture2D.h"

#include "Direct3DDeferredRenderer.h"
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
#include "BlurShadowsRenderer.h"
#include "UtilityRenderer.h"
#include "ExtractBrightPixelsRenderer.h"
#include "ToneMappingRenderer.h"
#include "AntialiasingRenderer.h"

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
            RayDirections,
            Contribution,
            CurrentRefractiveIndex,
            Preillumination,
            HardIllumination,
            SoftIllumination,
            BlurredShadows,
            SpotlightDepth,
            DistanceToOccluder,
            FinalDistanceToOccluder,
            MaxIlluminationBlurRadiusInScreenSpace,
            MinIlluminationBlurRadiusInWorldSpace,
            MaxIlluminationBlurRadiusInWorldSpace,
            BloomBrightPixels,
            HitDistance,
            HitDistanceBlurred,
            HitDistanceToCamera,
			Test
        };

        enum class LayerType : char {
            Reflection = 0,
            Refraction
        };

        static std::string viewToString( const View view );

        struct Output
        {
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > ucharImage;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >        uchar4Image;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >        float4Image;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >        float2Image;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >        floatImage;

            bool isEmpty()
            {
                return !ucharImage && !uchar4Image && !float4Image && !float2Image && !floatImage;
            }

            void reset()
            {
                ucharImage  = nullptr;
                uchar4Image = nullptr;
                float4Image = nullptr;
                float2Image = nullptr;
                floatImage  = nullptr;
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
            }

            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        contributionRoughness;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        rayOrigin;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        rayDirection;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        hitPosition;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        hitEmissive;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        hitAlbedo;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitMetalness;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitRoughness;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        hitNormal;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > currentRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, uchar4 > >                        depth;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         hitDistance;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         hitDistanceBlurred;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         hitDistanceToCamera;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        hitShaded;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        shadedCombined;
        };

        Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler, RenderTargetManager& renderTargetManager );
        ~Renderer();

        void initialize( 
            const int2 imageDimensions, 
            Microsoft::WRL::ComPtr< ID3D11Device > device, 
            Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext,
            std::shared_ptr<const BlockModel> lightModel 
        );

        // Should be called at the beginning of each frame, before calling renderScene(). 
        void clear();

        void clear2();

		void renderShadowMaps( const Scene& scene );

        Output renderScene( 
            const Scene& scene, const Camera& camera,
            const bool wireframeMode,
            const Selection& selection,
            const std::shared_ptr< BlockMesh > selectionVolumeMesh 
        );

        Renderer::Output renderText( 
            const std::string& text, 
            Font& font, 
            float2 position, 
            float4 color
        );

        void setActiveViewType( const View view );
        View getActiveViewType() const;
        
        float getExposure();
        void  setExposure( const float exposure );

        // Temporary - for debug.
        const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > > debugGetCurrentRefractiveIndexTextures();

        private:

        Output renderPrimaryLayer( 
            const Scene& scene, const Camera& camera, 
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const std::vector< bool >& activeViewLevel, const View activeViewType,
            const bool wireframeMode,
            const Selection& selection,
            const std::shared_ptr< BlockMesh > selectionVolumeMesh 
        );

        Output renderSecondaryLayers( 
            const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            std::vector< bool >& renderedViewLevel,
            const std::vector< bool >& activeViewLevel,
            const View activeViewType
        );

        void renderSecondaryLayer(
            const LayerType layerType, const int level, const int refractionLevel, const Camera& camera,
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows
        );

        void combineLayers( const int currentLevel, const Camera& camera );

        void performBloom( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture, const float minBrightness );

        void performToneMapping( 
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
            std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture,
            const float exposure 
        );

        void performAntialiasing( 
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > srcTexture,
            std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > dstTexture
        );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        View m_activeViewType;

        int2 m_imageDimensions;

        // Tone mapping configuration.
        float m_exposure;

        // Bloom configuration.
        float m_minBrightness;

        Direct3DRendererCore&     m_rendererCore;
        Profiler&                 m_profiler;
        RenderTargetManager&      m_renderTargetManager;

        Direct3DDeferredRenderer            m_deferredRenderer;
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
        BlurShadowsRenderer                 m_blurShadowsRenderer;
        UtilityRenderer                     m_utilityRenderer;
        ExtractBrightPixelsRenderer         m_extractBrightPixelsRenderer;
        ToneMappingRenderer                 m_toneMappingRenderer;
        AntialiasingRenderer                m_antialiasingRenderer;

        // Render target.
        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > m_finalRenderTargetLDR;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > m_temporaryRenderTargetLDR;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_finalRenderTargetHDR;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_temporaryRenderTarget1;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_temporaryRenderTarget2;

        std::vector< LayerRenderTargets > m_layersRenderTargets;

        std::shared_ptr<const BlockModel> m_lightModel;

        Output getLayerRenderTarget( View view, int level );

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

