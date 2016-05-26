#include "Renderer.h"

#include <memory>

#include "Direct3DRendererCore.h"
#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "CombiningRenderer.h"
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

using Microsoft::WRL::ComPtr;

Renderer::Renderer( Direct3DRendererCore& rendererCore, Direct3DDeferredRenderer& deferredRenderer, RaytraceRenderer& raytraceRenderer, 
                    ShadingRenderer& shadingRenderer, EdgeDetectionRenderer& edgeDetectionRenderer, CombiningRenderer& combiningRenderer ) :
rendererCore( rendererCore ),
deferredRenderer( deferredRenderer ),
raytraceRenderer( raytraceRenderer ),
shadingRenderer( shadingRenderer ),
edgeDetectionRenderer( edgeDetectionRenderer ),
combiningRenderer( combiningRenderer ),
activeView( View::Final )
{}

Renderer::~Renderer()
{}

void Renderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext, 
                         std::shared_ptr<const BlockMesh> axisMesh, std::shared_ptr<const BlockModel> lightModel )
{
    this->device        = device;
    this->deviceContext = deviceContext;

    this->axisMesh = axisMesh;
    this->lightModel = lightModel;

    createRenderTarget( imageWidth, imageHeight, *device.Get() );
}

std::tuple< 
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderScene( const CScene& scene, const Camera& camera )
{
    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    deferredRenderer.clearRenderTargets( float4( 0.0f, 0.0f, 0.0f, 1.0f ), 1.0f ); 

    float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint(), camera.getPosition(), camera.getUp() );

    // Render 'axises' model.
    if ( axisMesh )
        deferredRenderer.render( *axisMesh, float43::IDENTITY, viewMatrix );

    // Render actors in the scene.
    const std::unordered_set< std::shared_ptr<Actor> >& actors = scene.getActors();
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) {
            const std::shared_ptr<BlockActor> blockActor = std::static_pointer_cast<BlockActor>(actor);
            const std::shared_ptr<BlockModel> blockModel = blockActor->getModel();

            if ( blockModel->isInGpuMemory() )
                deferredRenderer.render( *blockModel, blockActor->getPose(), viewMatrix );
            else if ( blockModel->getMesh() && blockModel->getMesh()->isInGpuMemory() )
                deferredRenderer.render( *blockModel->getMesh(), blockActor->getPose(), viewMatrix );

        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
            const std::shared_ptr<SkeletonActor> skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
            const std::shared_ptr<SkeletonModel> skeletonModel = skeletonActor->getModel();

            if ( skeletonModel->isInGpuMemory() )
                deferredRenderer.render( *skeletonModel, skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose() );
            else if ( skeletonModel->getMesh() && skeletonModel->getMesh()->isInGpuMemory() )
                deferredRenderer.render( *skeletonModel->getMesh(), skeletonActor->getPose(), viewMatrix, skeletonActor->getSkeletonPose() );
        }
    }

    // Render light sources in the scene.
    if ( lightModel ) {
        const std::unordered_set< std::shared_ptr<Light> >& lights = scene.getLights();
        float43 lightPose( float43::IDENTITY );
        for ( const std::shared_ptr<Light> light : lights ) {
            lightPose.setTranslation( light->getPosition() );
            deferredRenderer.render( *lightModel, lightPose, viewMatrix );
        }
    }

    // Perform ray tracing.
    std::vector< std::shared_ptr< const BlockActor > > blockActors;
    blockActors.reserve( actors.size() );
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            blockActors.push_back( std::static_pointer_cast<BlockActor>( actor ) );
    }
    //raytraceRenderer.generateAndTraceRays( camera, blockActors );
    raytraceRenderer.generateAndTraceSecondaryRays( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget(), blockActors );

    // Perform shading on the main image.
    std::vector< std::shared_ptr< Light > > lights;
    shadingRenderer.performShading( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getAlbedoRenderTarget(), deferredRenderer.getNormalRenderTarget(), lights );

    // Copy main shaded image to final render target.
    rendererCore.copyTexture( finalRenderTarget, shadingRenderer.getColorRenderTarget() );

    // Perform shading on the reflected image.
    shadingRenderer.performShading( camera, raytraceRenderer.getRayHitPositionTexture(), raytraceRenderer.getRayHitAlbedoTexture(), raytraceRenderer.getRayHitNormalTexture(), lights );

    // Generate mipmaps for the shaded, reflected image.
    shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    // Perform edge detection on the main image (position + normal) - to detect discontinuities in reflections.
    edgeDetectionRenderer.performEdgeDetection( deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget() );

    // Combine main image with reflections and refractions.
    combiningRenderer.combine( finalRenderTarget, shadingRenderer.getColorRenderTarget(), 0.5f );

    switch (activeView)
    {
        case View::Final: 
            return std::make_tuple( 
                nullptr,
                nullptr,
                finalRenderTarget,
                nullptr,
                nullptr
             );
        case View::Depth: 
            return std::make_tuple( 
                nullptr,
                deferredRenderer.getDepthRenderTarget(),
                nullptr,
                nullptr,
                nullptr
             );
        case View::Position: 
            return std::make_tuple( 
                nullptr,
                nullptr,
                deferredRenderer.getPositionRenderTarget(),
                nullptr,
                nullptr
             );
        case View::Albedo: 
            return std::make_tuple( 
                nullptr,
                deferredRenderer.getAlbedoRenderTarget(),
                nullptr,
                nullptr,
                nullptr
             );
        case View::Normal:
            return std::make_tuple(
                nullptr,
                nullptr,
                deferredRenderer.getNormalRenderTarget(),
                nullptr,
                nullptr
             );
        case View::Metalness:
            return std::make_tuple(
                deferredRenderer.getMetalnessRenderTarget(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::Roughness:
            return std::make_tuple(
                deferredRenderer.getRoughnessRenderTarget(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::IndexOfRefraction:
            return std::make_tuple(
                deferredRenderer.getIndexOfRefractionRenderTarget(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::DistanceToEdge:
            return std::make_tuple(
                edgeDetectionRenderer.getValueRenderTarget(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::RayDirections1:
            return std::make_tuple(
                nullptr,
                nullptr,
                raytraceRenderer.getRayDirectionsTexture(),
                nullptr,
                nullptr
             );
        case View::RaytracingHitPosition:
            return std::make_tuple(
                nullptr,
                nullptr,
                raytraceRenderer.getRayHitPositionTexture(),
                nullptr,
                nullptr
            );
        case View::RaytracingHitDistance:
            return std::make_tuple(
                nullptr,
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
                nullptr,
                nullptr
            );
        case View::RaytracingHitAlbedo:
            return std::make_tuple(
                nullptr,
                raytraceRenderer.getRayHitAlbedoTexture(),
                nullptr,
                nullptr,
                nullptr
            );
        case View::RaytracingHitMetalness:
            return std::make_tuple(
                raytraceRenderer.getRayHitMetalnessTexture(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::RaytracingHitRoughness:
            return std::make_tuple(
                raytraceRenderer.getRayHitRoughnessTexture(),
                nullptr,
                nullptr,
                nullptr,
                nullptr
             );
        case View::RaytracingHitIndexOfRefraction:
        default:
            return std::make_tuple(
                raytraceRenderer.getRayHitIndexOfRefractionTexture(),
                nullptr,
                nullptr,
                nullptr,
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

void Renderer::createRenderTarget( int imageWidth, int imageHeight, ID3D11Device& device )
{
    finalRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );
}
