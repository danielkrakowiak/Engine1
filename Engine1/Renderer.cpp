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
//#include "RenderTargetTexture2D.h"
//#include "ComputeTargetTexture2D.h"

using namespace Engine1;

Renderer::Renderer( Direct3DDefferedRenderer& defferedRenderer, RaytraceRenderer& raytraceRenderer ) :
defferedRenderer( defferedRenderer ),
raytraceRenderer( raytraceRenderer ),
activeView( View::Albedo )
{}

Renderer::~Renderer()
{}

void Renderer::initialize( std::shared_ptr<const BlockMesh> axisMesh, std::shared_ptr<const BlockModel> lightModel )
{
    this->axisMesh = axisMesh;
    this->lightModel = lightModel;
}

std::tuple< 
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderScene( const CScene& scene, const Camera& camera )
{
    defferedRenderer.clearRenderTargets( float4( 0.2f, 0.2f, 0.2f, 1.0f ), 1.0f );

    float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint(), camera.getPosition(), camera.getUp() );

    // Render 'axises' model.
    if ( axisMesh )
        defferedRenderer.render( *axisMesh, float43::IDENTITY, viewMatrix );

    // Render actors in the scene.
    const std::unordered_set< std::shared_ptr<Actor> >& actors = scene.getActors();
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
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

    // Perform raytracing on the first block actor.
    std::vector< std::shared_ptr< const BlockActor > > blockActors;
    blockActors.reserve( actors.size() );
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            blockActors.push_back( std::static_pointer_cast<BlockActor>( actor ) );
    }
    raytraceRenderer.generateAndTraceRays( camera, blockActors );

    switch (activeView)
    {
        case View::Albedo: 
            return std::make_tuple( 
                defferedRenderer.getRenderTarget( Direct3DDefferedRenderer::RenderTargetType::ALBEDO ),
                nullptr,
                nullptr,
                nullptr
             );
        case View::Normal:
            return std::make_tuple(
                defferedRenderer.getRenderTarget( Direct3DDefferedRenderer::RenderTargetType::NORMAL ),
                nullptr,
                nullptr,
                nullptr
             );
        case View::RayDirections1:
            return std::make_tuple(
                nullptr,
                raytraceRenderer.getRayDirectionsTexture(),
                nullptr,
                nullptr
             );
        case View::RaytracingHitDistance:
            return std::make_tuple(
                nullptr,
                nullptr,
                nullptr,
                raytraceRenderer.getRayHitDistanceTexture() 
            );
        case View::RaytracingHitNormal:
            return std::make_tuple(
                nullptr,
                nullptr,
                raytraceRenderer.getRayHitNormalTexture(),
                nullptr
            );
        case View::RaytracingHitTexCoords:
        default:
            return std::make_tuple(
                nullptr,
                nullptr,
                raytraceRenderer.getRayHitBarycentricTexture(),
                nullptr
            );
    }
}

void Renderer::setActiveView( const View view )
{
    activeView = view;
}

Renderer::View Renderer::getActiveView() const
{
    return activeView;
}
