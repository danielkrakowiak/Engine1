#include "Renderer.h"

#include <memory>

#include "Direct3DDefferedRenderer.h"
#include "RaytraceRenderer.h"
#include "CScene.h"
#include "Camera.h"
#include "MathUtil.h"
#include "BlockActor.h"
#include "BlockModel.h"
#include "SkeletonActor.h"
#include "SkeletonModel.h"
#include "Light.h"
#include "Texture2D.h"
#include "RenderTargetTexture2D.h"
#include "ComputeTargetTexture2D.h"

using namespace Engine1;

Renderer::Renderer( Direct3DDefferedRenderer& defferedRenderer, RaytraceRenderer& raytraceRenderer ) :
defferedRenderer( defferedRenderer ),
raytraceRenderer( raytraceRenderer )
{}

Renderer::~Renderer()
{}

void Renderer::initialize( std::shared_ptr<const BlockMesh> axisMesh, std::shared_ptr<const BlockModel> lightModel )
{
    this->axisMesh = axisMesh;
    this->lightModel = lightModel;
}

std::shared_ptr<Texture2D> Renderer::renderScene( const CScene& scene, const Camera& camera )
{
    defferedRenderer.clearRenderTargets( float4( 0.2f, 0.2f, 0.2f, 1.0f ), 1.0f );

    float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint(), camera.getPosition(), camera.getUp() );

    // Render 'axises' model.
    if ( axisMesh )
        defferedRenderer.render( *axisMesh, float43::IDENTITY, viewMatrix );

    // Render actors in the scene.
    const std::unordered_set< std::shared_ptr<Actor> >& actors = scene.getActors();
    for ( const std::shared_ptr<Actor> actor : actors ) {
        if ( actor->getType() == Actor::Type::BlockActor ) {
            const std::shared_ptr<BlockActor> blockActor = std::static_pointer_cast<BlockActor>(actor);
            const std::shared_ptr<BlockModel> blockModel = blockActor->getModel();

            if ( blockModel->isInGpuMemory() )
                defferedRenderer.render( *blockModel, blockActor->getPose(), viewMatrix );
            else if ( blockModel->getMesh() && blockModel->getMesh()->isInGpuMemory() )
                defferedRenderer.render( *blockModel->getMesh(), blockActor->getPose(), viewMatrix );

        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const std::shared_ptr<SkeletonActor> skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
            const std::shared_ptr<SkeletonModel> skeletonModel = skeletonActor->getModel();

            if ( skeletonModel->isInGpuMemory() )
                defferedRenderer.render( *skeletonModel, skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose() );
            else if ( skeletonModel->getMesh() && skeletonModel->getMesh()->isInGpuMemory() )
                defferedRenderer.render( *skeletonModel->getMesh(), skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose() );
        }
    }

    // Render light sources in the scene.
    if ( lightModel ) {
        const std::unordered_set< std::shared_ptr<Light> >& lights = scene.getLights();
        float43 lightPose( float43::IDENTITY );
        for ( const std::shared_ptr<Light> light : lights ) {
            lightPose.setTranslation( light->getPosition() );
            defferedRenderer.render( *lightModel, lightPose, viewMatrix );
        }
    }

    raytraceRenderer.generateRays();
    std::shared_ptr<ComputeTargetTexture2D> raytraceFrame = raytraceRenderer.getComputeTarget();

    std::shared_ptr<RenderTargetTexture2D> defferedFrame = defferedRenderer.getRenderTarget( Direct3DDefferedRenderer::RenderTargetType::ALBEDO );

    //std::shared_ptr<Texture2D> frame = std::dynamic_pointer_cast<Texture2D>(defferedFrame);
    std::shared_ptr<Texture2D> frame = std::dynamic_pointer_cast<Texture2D>(raytraceFrame);

    return frame;
}
