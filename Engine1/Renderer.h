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

        Renderer( Direct3DRendererCore& rendererCore, Profiler& profiler );
        ~Renderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext,
                         std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

        // Should be called at the beginning of each frame, before calling renderScene(). 
        void clear();

        void clear2();

		void renderShadowMaps( const Scene& scene );

        std::tuple< 
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > >
        renderScene( const Scene& scene, const Camera& camera,
                     const bool wireframeMode,
                     const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
                     const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
                     const std::vector< std::shared_ptr< Light > >& selectedLights,
                     const std::shared_ptr< BlockMesh > selectionVolumeMesh );

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >
        renderText( const std::string& text, Font& font, float2 position, float4 color );

        void                       setActiveViewType( const View view );
        View                       getActiveViewType() const;
        void                       activateNextViewLevel( const bool reflection );
        void                       activatePrevViewLevel();
        const std::vector< bool >& getActiveViewLevel() const;
        void                       setMaxLevelCount( const int levelCount );
        int                        getMaxLevelCount() const;
        
        void debugSetUseSeparableShadowsBlur( const bool useSeparableBlur );
        bool debugIsUsingSeparableShadowsBlur();

        float getExposure();
        void  setExposure( const float exposure );

        // Temporary - for debug.
        const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& debugGetCurrentRefractiveIndexTextures();

        private:

        std::tuple< 
        bool,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
        renderMainImage( const Scene& scene, const Camera& camera, 
                         const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                         const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
                         const std::vector< bool >& activeViewLevel, const View activeViewType,
                         const bool wireframeMode,
                         const std::vector< std::shared_ptr< BlockActor > >& selectedBlockActors,
                         const std::vector< std::shared_ptr< SkeletonActor > >& selectedSkeletonActors,
                         const std::vector< std::shared_ptr< Light > >& selectedLights,
                         const std::shared_ptr< BlockMesh > selectionVolumeMesh );

        std::tuple<
        bool,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
        renderReflectionsRefractions( const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
                                      const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                      const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                      const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows,
                                      std::vector< bool >& renderedViewLevel,
                                      const std::vector< bool >& activeViewLevel,
                                      const View activeViewType );

        void renderFirstReflections( const Camera& camera, 
                                     const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                     const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                     const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows );

        void renderFirstRefractions( const Camera& camera, 
                                     const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                     const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                     const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows );

        void renderReflections( const int level, const Camera& camera, 
                                const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows );
        
        void renderRefractions( const int level, const int refractionLevel, const Camera& camera, 
                                const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                const std::vector< std::shared_ptr< Light > >& lightsCastingShadows,
                                const std::vector< std::shared_ptr< Light > >& lightsNotCastingShadows );

        void performBloom( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > colorTexture, const float minBrightness );

        void performToneMapping( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > texture, const float exposure );

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        View              m_activeViewType;
        std::vector<bool> m_activeViewLevel; // Empty - main image. true - reflection, false - refraction.
        int               m_maxLevelCount;

        // Debug option to enable/disable blurring shadows in two passes - horizontal and vertical.
        // It reduces blurring complexity from n^2 to 2n, where n is blurring kernel size.
        // But it's not mathematically correct (because of variable levels of blur per pixel) so may lead to some artifacts.
        bool m_debugUseSeparableShadowsBlur;

        // Tone mapping configuration.
        float m_exposure;

        // Bloom configuration.
        float m_minBrightness;

        Direct3DRendererCore&     m_rendererCore;
        Profiler&                 m_profiler;

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

