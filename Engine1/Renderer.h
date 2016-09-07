#pragma once

#include <memory>
#include <wrl.h>
#include "Texture2D.h"

#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "ReflectionRefractionShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "CombiningRenderer.h"
#include "TextureRescaleRenderer.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

namespace Engine1 
{
    class Direct3DRendererCore;
    class CScene;
    class Camera;
    class BlockMesh;
    class BlockModel;
    class BlockActor;
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
            CurrentRefractiveIndex
        };

        Renderer( Direct3DRendererCore& rendererCore );
        ~Renderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext,
                         std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

        // Should be called at the beginning of each frame, before calling renderScene(). 
        void prepare();

        std::tuple< 
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > >
        renderScene( const CScene& scene, const Camera& camera );

        std::tuple< 
        bool,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
        renderMainImage( const CScene& scene, const Camera& camera, const std::vector< std::shared_ptr< Light > >& lightsVector,
                         const std::vector< bool >& activeViewLevel, const View activeViewType );

        std::tuple<
        bool,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
        renderReflectionsRefractions( const bool reflectionFirst, const int level, const int refractionLevel, const int maxLevelCount, const Camera& camera,
                                      const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                      const std::vector< std::shared_ptr< Light > >& lightsVector,
                                      std::vector< bool >& renderedViewLevel,
                                      const std::vector< bool >& activeViewLevel,
                                      const View activeViewType );

        void renderFirstReflections( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector );
        void renderFirstRefractions( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector );

        void renderReflections( const int level, const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector );
        void renderRefractions( const int level, const int refractionLevel, const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector );

        void                       setActiveViewType( const View view );
        View                       getActiveViewType() const;
        void                       activateNextViewLevel( const bool reflection );
        void                       activatePrevViewLevel();
        const std::vector< bool >& getActiveViewLevel() const;
        void                       setMaxLevelCount( const int levelCount );
        int                        getMaxLevelCount() const;

        // Temporary - for debug.
        const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& debugGetCurrentRefractiveIndexTextures();

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        View              m_activeViewType;
        std::vector<bool> m_activeViewLevel; // Empty - main image. true - reflection, false - refraction.
        int               m_maxLevelCount;

        Direct3DRendererCore&     m_rendererCore;
        Direct3DDeferredRenderer  m_deferredRenderer;
        RaytraceRenderer          m_raytraceRenderer;
        ShadingRenderer           m_shadingRenderer;
        ReflectionRefractionShadingRenderer m_reflectionRefractionShadingRenderer;
        EdgeDetectionRenderer     m_edgeDetectionRenderer;
        CombiningRenderer         m_combiningRenderer;
        TextureRescaleRenderer    m_textureRescaleRenderer;

        // Render target.
        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > m_finalRenderTarget;

        std::shared_ptr<const BlockMesh>  m_axisMesh;
        std::shared_ptr<const BlockModel> m_lightModel;

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

