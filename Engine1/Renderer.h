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
#include "RenderTargetManager.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

namespace Engine1 
{
    class Direct3DRendererCore;
    class Profiler;
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
            BlurredIllumination,
            SpotlightDepth,
            DistanceToOccluder,
            FinalDistanceToOccluder,
            MaxIlluminationBlurRadiusInScreenSpace,
            MinIlluminationBlurRadiusInWorldSpace,
            MaxIlluminationBlurRadiusInWorldSpace,
            BloomBrightPixels,
            HitDistance,
            FinalHitDistance,
			Test
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

        Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler );
        ~Renderer();

        void initialize( const int2 imageDimensions, Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext,
                         std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

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
        const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& debugGetCurrentRefractiveIndexTextures();

        private:

        Output renderSceneImage( 
            const Scene& scene, const Camera& camera, 
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const std::vector< bool >& activeViewLevel, const View activeViewType,
            const bool wireframeMode,
            const Selection& selection,
            const std::shared_ptr< BlockMesh > selectionVolumeMesh 
        );

        Output renderReflectionsRefractions( 
            const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            std::vector< bool >& renderedViewLevel,
            const std::vector< bool >& activeViewLevel,
            const View activeViewType,
            const Direct3DDeferredRenderer::RenderTargets& deferredRenderTargets
        );

        void renderFirstReflections( 
            const Camera& camera, 
            const std::vector< std::shared_ptr< BlockActor > >& blockActors, 
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const Direct3DDeferredRenderer::RenderTargets& deferredRenderTargets
        );

        void renderFirstRefractions( 
            const Camera& camera, 
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const Direct3DDeferredRenderer::RenderTargets& deferredRenderTargets
        );

        void renderReflections( 
            const int level, const Camera& camera, 
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >& deferredDepthRenderTarget
        );
        
        void renderRefractions( 
            const int level, const int refractionLevel, const Camera& camera, 
            const std::vector< std::shared_ptr< BlockActor > >& blockActors,
            const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
            const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
            const std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >& deferredDepthRenderTarget
        );

        void performBloom( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture, const float minBrightness );

        void performToneMapping( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > texture, const float exposure );

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
        RenderTargetManager       m_renderTargetManager;

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

        // Render target.
        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_finalRenderTarget;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_temporaryRenderTarget1;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_temporaryRenderTarget2;

        std::shared_ptr<const BlockMesh>  m_axisMesh;
        std::shared_ptr<const BlockModel> m_lightModel;

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

