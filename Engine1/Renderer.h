#pragma once

#include <memory>
#include "TTexture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

namespace Engine1 
{
    class Direct3DDeferredRenderer;
    class RaytraceRenderer;
    class ShadingRenderer;
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
            Albedo,
            Normal,
            RayDirections1,
            RaytracingHitDistance,
            RaytracingHitNormal,
            RaytracingHitAlbedo,
        };

        Renderer( Direct3DDeferredRenderer& deferredRenderer, RaytraceRenderer& raytraceRenderer, ShadingRenderer& shadingRenderer, CombiningRenderer& combiningRenderer );
        ~Renderer();

        void initialize( std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

        std::tuple< 
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > >
        renderScene( const CScene& scene, const Camera& camera );

        void setActiveView( const View view );
        View getActiveView() const;

        private:

        View activeView;

        Direct3DDeferredRenderer& deferredRenderer;
        RaytraceRenderer&         raytraceRenderer;
        ShadingRenderer&          shadingRenderer;
        CombiningRenderer&        combiningRenderer;

        std::shared_ptr<const BlockMesh>  axisMesh;
        std::shared_ptr<const BlockModel> lightModel;

        // Copying is not allowed.
        Renderer( const Renderer& ) = delete;
        Renderer& operator=(const Renderer&) = delete;
    };
};

