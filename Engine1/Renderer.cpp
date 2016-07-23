#include "Renderer.h"

#include <memory>

#include "Direct3DRendererCore.h"
#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "ReflectionShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "CombiningRenderer.h"
#include "TextureRescaleRenderer.h"
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

Renderer::Renderer( Direct3DRendererCore& rendererCore, Direct3DDeferredRenderer& deferredRenderer, RaytraceRenderer& raytraceRenderer, RaytraceRenderer& raytraceRenderer2,
                    ShadingRenderer& shadingRenderer, ReflectionShadingRenderer& reflectionShadingRenderer, EdgeDetectionRenderer& edgeDetectionRenderer, CombiningRenderer& combiningRenderer,
                    TextureRescaleRenderer& textureRescaleRenderer ) :
rendererCore( rendererCore ),
deferredRenderer( deferredRenderer ),
raytraceRenderer( raytraceRenderer ),
raytraceRenderer2( raytraceRenderer2 ),
shadingRenderer( shadingRenderer ),
reflectionShadingRenderer( reflectionShadingRenderer ),
edgeDetectionRenderer( edgeDetectionRenderer ),
combiningRenderer( combiningRenderer ),
textureRescaleRenderer( textureRescaleRenderer ),
activeView( View::Final ),
activeViewLevel( 0 ),
maxLevelCount( 3 )
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

    createRenderTargets( imageWidth, imageHeight, *device.Get() );
}

// Should be called at the beginning of each frame, before calling renderScene(). 
void Renderer::prepare()
{
    // Note: this color is important. It's used to check which pixels haven't been changed when spawning secondary rays. 
    // Be careful when changing!
    deferredRenderer.clearRenderTargets( float4( 0.0f, 0.0f, 0.0f, 1.0f ), 1.0f ); 
}

std::tuple< 
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderScene( const CScene& scene, const Camera& camera )
{
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

    // Gather block actors.
    std::vector< std::shared_ptr< const BlockActor > > blockActors;
    blockActors.reserve( actors.size() );
    for ( const std::shared_ptr<Actor> actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            blockActors.push_back( std::static_pointer_cast<BlockActor>( actor ) );
    }

    if (activeViewLevel == 0)
    {
        switch (activeView)
        {
            case View::Depth: 
                return std::make_tuple( nullptr, deferredRenderer.getDepthRenderTarget(), nullptr, nullptr, nullptr );
            case View::Position: 
                return std::make_tuple( nullptr, nullptr, deferredRenderer.getPositionRenderTarget(), nullptr, nullptr );
            case View::Emissive: 
                return std::make_tuple( nullptr, deferredRenderer.getEmissiveRenderTarget(), nullptr, nullptr, nullptr );
            case View::Albedo: 
                return std::make_tuple( nullptr, deferredRenderer.getAlbedoRenderTarget(), nullptr, nullptr, nullptr );
            case View::Normal:
                return std::make_tuple( nullptr, nullptr, deferredRenderer.getNormalRenderTarget(), nullptr, nullptr );
            case View::Metalness:
                return std::make_tuple( deferredRenderer.getMetalnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::Roughness:
                return std::make_tuple( deferredRenderer.getRoughnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::IndexOfRefraction:
                return std::make_tuple( deferredRenderer.getIndexOfRefractionRenderTarget(), nullptr, nullptr, nullptr, nullptr );
        }
    }

    // Generate mipmaps for normal and position g-buffers.
    deferredRenderer.getNormalRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );
    deferredRenderer.getPositionRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    // Perform shading on the main image.
    const std::unordered_set< std::shared_ptr< Light > >& lights = scene.getLights();
    const std::vector< std::shared_ptr< Light > > lightsVec( lights.begin(), lights.end() );
    shadingRenderer.performShading( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getEmissiveRenderTarget(), 
                                    deferredRenderer.getAlbedoRenderTarget(), deferredRenderer.getMetalnessRenderTarget(), 
                                    deferredRenderer.getRoughnessRenderTarget(), deferredRenderer.getNormalRenderTarget(), 
                                    deferredRenderer.getIndexOfRefractionRenderTarget(), lightsVec );

    // Copy main shaded image to final render target.
    rendererCore.copyTexture( finalRenderTarget, shadingRenderer.getColorRenderTarget() );

    if ( activeView == Renderer::View::Shaded && activeViewLevel == 0 )
        return std::make_tuple( nullptr, nullptr, shadingRenderer.getColorRenderTarget(), nullptr, nullptr );

    if ( maxLevelCount >= 2 )
    {
        // Perform reflection shading.
        reflectionShadingRenderer.performShading( 0, camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget(), deferredRenderer.getAlbedoRenderTarget(),
                                                  deferredRenderer.getMetalnessRenderTarget(), deferredRenderer.getRoughnessRenderTarget() );

        if ( activeView == Renderer::View::Test && activeViewLevel == 1 )
            return std::make_tuple( nullptr, reflectionShadingRenderer.getReflectionTermTarget( 0 ), nullptr, nullptr, nullptr );

        // Perform ray tracing.
        raytraceRenderer.generateAndTraceFirstRefractedRays( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget(),
                                                             deferredRenderer.getRoughnessRenderTarget(), reflectionShadingRenderer.getReflectionTermTarget( 0 ), blockActors );

        if (activeViewLevel == 1)
        {
            switch (activeView)
            {
                /*case View::Depth: 
                    return std::make_tuple( nullptr, raytraceRenderer.getRayHitDistanceTexture(), nullptr, nullptr, nullptr );*/
                case View::Position: 
                    return std::make_tuple( nullptr, nullptr, raytraceRenderer.getRayHitPositionTexture(), nullptr, nullptr );
                case View::Emissive: 
                    return std::make_tuple( nullptr, raytraceRenderer.getRayHitEmissiveTexture(), nullptr, nullptr, nullptr );
                case View::Albedo: 
                    return std::make_tuple( nullptr, raytraceRenderer.getRayHitAlbedoTexture(), nullptr, nullptr, nullptr );
                case View::Normal:
                    return std::make_tuple( nullptr, nullptr, raytraceRenderer.getRayHitNormalTexture(), nullptr, nullptr );
                case View::Metalness:
                    return std::make_tuple( raytraceRenderer.getRayHitMetalnessTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::Roughness:
                    return std::make_tuple( raytraceRenderer.getRayHitRoughnessTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::IndexOfRefraction:
                    return std::make_tuple( raytraceRenderer.getRayHitIndexOfRefractionTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::RayDirections: 
                    return std::make_tuple( nullptr, nullptr, raytraceRenderer.getRayDirectionsTexture(), nullptr, nullptr );
            }
        }

        // Perform shading on the reflected image.
        shadingRenderer.performShading( camera, raytraceRenderer.getRayHitPositionTexture(), raytraceRenderer.getRayHitEmissiveTexture(), 
                                        raytraceRenderer.getRayHitAlbedoTexture(), raytraceRenderer.getRayHitMetalnessTexture(),
                                        raytraceRenderer.getRayHitRoughnessTexture(), raytraceRenderer.getRayHitNormalTexture(),
                                        raytraceRenderer.getRayHitIndexOfRefractionTexture(), lightsVec );

        if ( activeView == Renderer::View::Shaded && activeViewLevel == 1 )
            return std::make_tuple( nullptr, nullptr, shadingRenderer.getColorRenderTarget(), nullptr, nullptr );

        // Generate mipmaps for the shaded, reflected image.
        shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

        // Upscale reflected image low-resolution mipmaps to half of the screen size to avoid visible aliasing.
        //const int srcMipmapCount = shadingRenderer.getColorRenderTarget()->getMipMapCountOnGpu();
        //for ( int srcMipmapLevel = 3; srcMipmapLevel > 2; --srcMipmapLevel )
        //    textureRescaleRenderer.rescaleTexture( shadingRenderer.getColorRenderTarget(), (unsigned char)srcMipmapLevel, shadingRenderer.getColorRenderTarget(), (unsigned char)( srcMipmapLevel - 1 ) );

        // Perform edge detection on the main image (position + normal) - to detect discontinuities in reflections.
        //edgeDetectionRenderer.performEdgeDetection( deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget() );

        // Combine main image with reflections and refractions.
        combiningRenderer.combine( finalRenderTarget, shadingRenderer.getColorRenderTarget(), reflectionShadingRenderer.getReflectionTermTarget( 0 ), deferredRenderer.getNormalRenderTarget(), 
                                   deferredRenderer.getPositionRenderTarget(), deferredRenderer.getDepthRenderTarget(), raytraceRenderer.getRayHitDistanceTexture(),
                                   deferredRenderer.getAlbedoRenderTarget(), deferredRenderer.getMetalnessRenderTarget(), deferredRenderer.getRoughnessRenderTarget(), camera.getPosition() );
    }

    for (int level = 2; level < maxLevelCount; level++)
    { 
        RaytraceRenderer& prevRaytraceRenderer = level % 2 == 0 ? raytraceRenderer : raytraceRenderer2;
        RaytraceRenderer& currRaytraceRenderer = level % 2 == 0 ? raytraceRenderer2 : raytraceRenderer;

        reflectionShadingRenderer.performShading( level - 1, camera, prevRaytraceRenderer.getRayHitPositionTexture(), prevRaytraceRenderer.getRayHitNormalTexture(), prevRaytraceRenderer.getRayHitAlbedoTexture(),
                                              prevRaytraceRenderer.getRayHitMetalnessTexture(), prevRaytraceRenderer.getRayHitRoughnessTexture() );

        if ( activeView == Renderer::View::Test && activeViewLevel == level )
            return std::make_tuple( nullptr, reflectionShadingRenderer.getReflectionTermTarget( level - 1 ), nullptr, nullptr, nullptr );

        currRaytraceRenderer.generateAndTraceRefractedRays( prevRaytraceRenderer.getRayDirectionsTexture(), prevRaytraceRenderer.getRayHitPositionTexture(), 
                                                            prevRaytraceRenderer.getRayHitNormalTexture(), prevRaytraceRenderer.getRayHitRoughnessTexture(), reflectionShadingRenderer.getReflectionTermTarget( level - 1 ), blockActors );

        if ( activeView == Renderer::View::RayDirections && activeViewLevel == level )
            return std::make_tuple( nullptr, nullptr, currRaytraceRenderer.getRayDirectionsTexture(), nullptr, nullptr );

        // Perform shading on the reflected image.
        shadingRenderer.performShading( currRaytraceRenderer.getRayOriginsTexture(), currRaytraceRenderer.getRayHitPositionTexture(), currRaytraceRenderer.getRayHitEmissiveTexture(), 
                                        currRaytraceRenderer.getRayHitAlbedoTexture(), currRaytraceRenderer.getRayHitMetalnessTexture(),
                                        currRaytraceRenderer.getRayHitRoughnessTexture(), currRaytraceRenderer.getRayHitNormalTexture(),
                                        currRaytraceRenderer.getRayHitIndexOfRefractionTexture(), lightsVec );

        if ( activeView == Renderer::View::Shaded && activeViewLevel == level )
            return std::make_tuple( nullptr, nullptr, shadingRenderer.getColorRenderTarget(), nullptr, nullptr );

        if (activeViewLevel == 2)
        {
            switch (activeView)
            {
                /*case View::Depth: 
                    return std::make_tuple( nullptr, raytraceRenderer2.getRayHitDistanceTexture(), nullptr, nullptr, nullptr );*/
                case View::Position: 
                    return std::make_tuple( nullptr, nullptr, currRaytraceRenderer.getRayHitPositionTexture(), nullptr, nullptr );
                case View::Emissive: 
                    return std::make_tuple( nullptr, currRaytraceRenderer.getRayHitEmissiveTexture(), nullptr, nullptr, nullptr );
                case View::Albedo: 
                    return std::make_tuple( nullptr, currRaytraceRenderer.getRayHitAlbedoTexture(), nullptr, nullptr, nullptr );
                case View::Normal:
                    return std::make_tuple( nullptr, nullptr, currRaytraceRenderer.getRayHitNormalTexture(), nullptr, nullptr );
                case View::Metalness:
                    return std::make_tuple( currRaytraceRenderer.getRayHitMetalnessTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::Roughness:
                    return std::make_tuple( currRaytraceRenderer.getRayHitRoughnessTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::IndexOfRefraction:
                    return std::make_tuple( currRaytraceRenderer.getRayHitIndexOfRefractionTexture(), nullptr, nullptr, nullptr, nullptr );
                case View::RayDirections: 
                    return std::make_tuple( nullptr, nullptr, currRaytraceRenderer.getRayDirectionsTexture(), nullptr, nullptr );
            }
        }

        // Generate mipmaps for the shaded, reflected image.
        shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

        combiningRenderer.combine( finalRenderTarget, shadingRenderer.getColorRenderTarget(), reflectionShadingRenderer.getReflectionTermTarget( level - 1 ), prevRaytraceRenderer.getRayHitNormalTexture(), 
                                   prevRaytraceRenderer.getRayHitPositionTexture(), deferredRenderer.getDepthRenderTarget(), currRaytraceRenderer.getRayHitDistanceTexture(),
                                   prevRaytraceRenderer.getRayHitAlbedoTexture(), prevRaytraceRenderer.getRayHitMetalnessTexture(), prevRaytraceRenderer.getRayHitRoughnessTexture(), camera.getPosition() );
    }

    return std::make_tuple( nullptr, nullptr, finalRenderTarget, nullptr, nullptr );
}

void Renderer::setActiveView( const View view )
{
    activeView = view;
}

Renderer::View Renderer::getActiveView() const
{
    return activeView;
}

void Renderer::setActiveViewLevel( const int viewLevel )
{
    activeViewLevel = std::max( 0, std::min( maxLevelCount - 1, viewLevel ) );
}

int Renderer::getActiveViewLevel() const
{
    return activeViewLevel;
}

void Renderer::setMaxLevelCount( const int levelCount )
{
    maxLevelCount = std::max( 1, levelCount );
    activeViewLevel = std::min( maxLevelCount - 1, activeViewLevel );
}

int Renderer::getMaxLevelCount() const
{
    return maxLevelCount;
}

void Renderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    finalRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    // Create one temp render target for each screen image mipmap smaller than half of the screen size.
    const int tempRenderTargetCount = (int)floor( log2( std::max( imageWidth / 2, imageHeight / 2 ) ) ); 

    //#TODO: Could probably use only RGB instead of RGBA. Could also ignore 1x1, 2x2 mipmaps to save memory.
    for ( int i = 0; i < tempRenderTargetCount; ++i )
    {
        halfSizeTempRenderTargets.push_back(
            std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >
            ( device, imageWidth / 2, imageHeight / 2, false, true, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT )
        );
    }

}
