#pragma once

#include <memory>
#include <wrl.h>
#include "TTexture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

namespace Engine1 
{
    class Direct3DRendererCore;
    class Direct3DDeferredRenderer;
    class RaytraceRenderer;
    class ShadingRenderer;
    class ReflectionShadingRenderer;
    class EdgeDetectionRenderer;
    class CombiningRenderer;
    class TextureRescaleRenderer;
    class CScene;
    class Camera;
    class BlockMesh;
    class BlockModel;

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
            Test
        };

        Renderer( Direct3DRendererCore& rendererCore, Direct3DDeferredRenderer& deferredRenderer, RaytraceRenderer& raytraceRenderer, RaytraceRenderer& raytraceRenderer2,
                  ShadingRenderer& shadingRenderer, ReflectionShadingRenderer& reflectionShadingRenderer, EdgeDetectionRenderer& edgeDetectionRenderer, CombiningRenderer& combiningRenderer,
                  TextureRescaleRenderer& textureRescaleRenderer );
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

        void setActiveView( const View view );
        View getActiveView() const;

        void setActiveViewLevel( const int viewLevel );
        int  getActiveViewLevel() const;

        void setMaxLevelCount( const int levelCount );
        int  getMaxLevelCount() const;

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        View activeView;
        int  activeViewLevel;
        int maxLevelCount;

        Direct3DRendererCore&      rendererCore;
        Direct3DDeferredRenderer&  deferredRenderer;
        RaytraceRenderer&          raytraceRenderer;
        RaytraceRenderer&          raytraceRenderer2;
        ShadingRenderer&           shadingRenderer;
        ReflectionShadingRenderer& reflectionShadingRenderer;
        EdgeDetectionRenderer&     edgeDetectionRenderer;
        CombiningRenderer&         combiningRenderer;
        TextureRescaleRenderer&    textureRescaleRenderer;

        // Render target.
        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > finalRenderTarget;

        // Half size, temporary render targets (to store upscaled reflection/refraction mipmaps).
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > > halfSizeTempRenderTargets;

        std::shared_ptr<const BlockMesh>  axisMesh;
        std::shared_ptr<const BlockModel> lightModel;

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

