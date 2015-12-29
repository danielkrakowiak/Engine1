#pragma once

#include <memory>

namespace Engine1 
{
    class Direct3DDefferedRenderer;
    class RaytraceRenderer;
    class CScene;
    class Camera;
    class BlockMesh;
    class BlockModel;
    class Texture2D;

    class Renderer
    {
        public:

        Renderer( Direct3DDefferedRenderer& defferedRenderer, RaytraceRenderer& raytraceRenderer );
        ~Renderer();

        void initialize( std::shared_ptr<const BlockMesh> axisModel, std::shared_ptr<const BlockModel> lightModel );

        std::shared_ptr<Texture2D> renderScene( const CScene& scene, const Camera& camera );

        private:

        Direct3DDefferedRenderer& defferedRenderer;
        RaytraceRenderer&         raytraceRenderer;

        std::shared_ptr<const BlockMesh>  axisMesh;
        std::shared_ptr<const BlockModel> lightModel;
    };
};

