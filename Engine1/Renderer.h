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
    class EdgeDetectionRenderer;
    class CombiningRenderer;
    class CScene;
    class Camera;
    class BlockMesh;
    class BlockModel;

    class Renderer
    {
        public:

        enum class View : char {
            Final = 0,
            Depth,
            Position,
            Albedo,
            Normal,
            Metalness,
            Roughness,
            IndexOfRefraction,
            DistanceToEdge,
            RayDirections1,
            RaytracingHitPosition,
            RaytracingHitDistance,
            RaytracingHitNormal,
            RaytracingHitAlbedo,
            RaytracingHitMetalness,
            RaytracingHitRoughness,
            RaytracingHitIndexOfRefraction
        };

        Renderer( Direct3DRendererCore& rendererCore, Direct3DDeferredRenderer& deferredRenderer, RaytraceRenderer& raytraceRenderer, 
                  ShadingRenderer& shadingRenderer, EdgeDetectionRenderer& edgeDetectionRenderer, CombiningRenderer& combiningRenderer );
        ~Renderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext,
                         std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

        std::tuple< 
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > >
        renderScene( const CScene& scene, const Camera& camera );

        void setActiveView( const View view );
        View getActiveView() const;

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext;

        View activeView;

        Direct3DRendererCore&     rendererCore;
        Direct3DDeferredRenderer& deferredRenderer;
        RaytraceRenderer&         raytraceRenderer;
        ShadingRenderer&          shadingRenderer;
        EdgeDetectionRenderer&    edgeDetectionRenderer;
        CombiningRenderer&        combiningRenderer;

        // Render target.
        void createRenderTarget( int imageWidth, int imageHeight, ID3D11Device& device );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > finalRenderTarget;

        std::shared_ptr<const BlockMesh>  axisMesh;
        std::shared_ptr<const BlockModel> lightModel;

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

