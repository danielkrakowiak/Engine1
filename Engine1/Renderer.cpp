#include "Renderer.h"

#include <memory>

#include "Direct3DRendererCore.h"
#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "ReflectionRefractionShadingRenderer.h"
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

#include "StringUtil.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Renderer::Renderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
deferredRenderer( rendererCore ),
raytraceRenderer( rendererCore ),
shadingRenderer( rendererCore ),
reflectionRefractionShadingRenderer( rendererCore ),
edgeDetectionRenderer( rendererCore ),
combiningRenderer( rendererCore ),
textureRescaleRenderer( rendererCore ),
activeViewType( View::Final ),
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

	deferredRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    raytraceRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    shadingRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    reflectionRefractionShadingRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    edgeDetectionRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    combiningRenderer.initialize( imageWidth, imageHeight, device, deviceContext );
    textureRescaleRenderer.initialize( device, deviceContext );

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
    bool frameReceived = false;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    const std::unordered_set< std::shared_ptr< Actor > >& actors = scene.getActors();

    const std::unordered_set< std::shared_ptr<Light> >& lights = scene.getLights();
    const std::vector< std::shared_ptr< Light > > lightsVector( lights.begin(), lights.end() );

    // Gather block actors.
    std::vector< std::shared_ptr< const BlockActor > > blockActors;
    blockActors.reserve( actors.size() );
    for ( const std::shared_ptr< Actor > actor : actors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor )
            blockActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
    }

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat ) 
        = renderMainImage( scene, camera, lightsVector, activeViewLevel, activeViewType );
    
    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    OutputDebugStringW( StringUtil::widen( "------------------- \n\n\n" ).c_str() );

    std::vector< bool > renderedViewType;

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( true, 1, maxLevelCount, camera, blockActors, lightsVector, renderedViewType, activeViewLevel, activeViewType );
    
    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( false, 1, maxLevelCount, camera, blockActors, lightsVector, renderedViewType, activeViewLevel, activeViewType );

    if ( frameReceived )
        return std::make_tuple( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    return std::make_tuple( nullptr, nullptr, finalRenderTarget, nullptr, nullptr );
}

std::tuple< 
bool,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderMainImage( const CScene& scene, const Camera& camera, const std::vector< std::shared_ptr< Light > >& lightsVector,
                           const std::vector< bool >& activeViewLevel, const View activeViewType )
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
        float43 lightPose( float43::IDENTITY );
        for ( const std::shared_ptr<Light> light : lightsVector ) {
            lightPose.setTranslation( light->getPosition() );
            deferredRenderer.render( *lightModel, lightPose, viewMatrix );
        }
    }

    // Generate mipmaps for normal and position g-buffers.
    deferredRenderer.getNormalRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );
    deferredRenderer.getPositionRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    // Perform shading on the main image.
    shadingRenderer.performShading( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getEmissiveRenderTarget(), 
                                    deferredRenderer.getAlbedoRenderTarget(), deferredRenderer.getMetalnessRenderTarget(), 
                                    deferredRenderer.getRoughnessRenderTarget(), deferredRenderer.getNormalRenderTarget(), 
                                    deferredRenderer.getIndexOfRefractionRenderTarget(), lightsVector );

    // Copy main shaded image to final render target.
    rendererCore.copyTexture( finalRenderTarget, shadingRenderer.getColorRenderTarget() );

    if ( activeViewLevel.empty() )
    {
        switch ( activeViewType )
        {
            case View::Shaded: 
                return std::make_tuple( true, nullptr, nullptr, shadingRenderer.getColorRenderTarget(), nullptr, nullptr );
            case View::Depth: 
                return std::make_tuple( true, nullptr, deferredRenderer.getDepthRenderTarget(), nullptr, nullptr, nullptr );
            case View::Position: 
                return std::make_tuple( true, nullptr, nullptr, deferredRenderer.getPositionRenderTarget(), nullptr, nullptr );
            case View::Emissive: 
                return std::make_tuple( true, nullptr, deferredRenderer.getEmissiveRenderTarget(), nullptr, nullptr, nullptr );
            case View::Albedo: 
                return std::make_tuple( true, nullptr, deferredRenderer.getAlbedoRenderTarget(), nullptr, nullptr, nullptr );
            case View::Normal:
                return std::make_tuple( true, nullptr, nullptr, deferredRenderer.getNormalRenderTarget(), nullptr, nullptr );
            case View::Metalness:
                return std::make_tuple( true, deferredRenderer.getMetalnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::Roughness:
                return std::make_tuple( true, deferredRenderer.getRoughnessRenderTarget(), nullptr, nullptr, nullptr, nullptr );
            case View::IndexOfRefraction:
                return std::make_tuple( true, deferredRenderer.getIndexOfRefractionRenderTarget(), nullptr, nullptr, nullptr, nullptr );
        }
    }

    return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );
}

std::tuple<
bool,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2 > >,
std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > > > 
Renderer::renderReflectionsRefractions( const bool reflectionFirst, const int level, const int maxLevelCount, const Camera& camera, 
                                        const std::vector< std::shared_ptr< const BlockActor > >& blockActors, 
                                        const std::vector< std::shared_ptr< Light > >& lightsVector,
                                        std::vector< bool >& renderedViewLevel,
                                        const std::vector< bool >& activeViewLevel,
                                        const View activeViewType )
{
    if ( level > maxLevelCount )
        return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );

    bool frameReceived = false;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

    renderedViewLevel.push_back( reflectionFirst );

    if ( reflectionFirst ) 
    {
        OutputDebugStringW( StringUtil::widen( "Reflection - " + std::to_string( level ) + "\n" ).c_str() );

        if ( level == 1 )
            renderFirstReflections( camera, blockActors, lightsVector );
        else
            renderReflections( level, camera, blockActors, lightsVector );
    }
    else 
    {
        OutputDebugStringW( StringUtil::widen( "Refraction - " + std::to_string( level ) + "\n" ).c_str() );

        if ( level == 1 )
            renderFirstRefractions( camera, blockActors, lightsVector );
        else
            renderRefractions( level, camera, blockActors, lightsVector );
    }

    if ( renderedViewLevel == activeViewLevel )
    {
        switch ( activeViewType )
        {
            case View::Shaded: 
                return std::make_tuple( true, nullptr, nullptr, shadingRenderer.getColorRenderTarget(), nullptr, nullptr );
            case View::Position: 
                return std::make_tuple( true, nullptr, nullptr, raytraceRenderer.getRayHitPositionTexture( level - 1 ), nullptr, nullptr );
            case View::Emissive: 
                return std::make_tuple( true, nullptr, raytraceRenderer.getRayHitEmissiveTexture( level - 1 ), nullptr, nullptr, nullptr );
            case View::Albedo: 
                return std::make_tuple( true, nullptr, raytraceRenderer.getRayHitAlbedoTexture( level - 1 ), nullptr, nullptr, nullptr );
            case View::Normal:
                return std::make_tuple( true, nullptr, nullptr, raytraceRenderer.getRayHitNormalTexture( level - 1 ), nullptr, nullptr );
            case View::Metalness:
                return std::make_tuple( true, raytraceRenderer.getRayHitMetalnessTexture( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::Roughness:
                return std::make_tuple( true, raytraceRenderer.getRayHitRoughnessTexture( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::IndexOfRefraction:
                return std::make_tuple( true, raytraceRenderer.getRayHitIndexOfRefractionTexture( level - 1 ), nullptr, nullptr, nullptr, nullptr );
            case View::RayDirections: 
                return std::make_tuple( true, nullptr, nullptr, raytraceRenderer.getRayDirectionsTexture( level - 1 ), nullptr, nullptr );
            case View::Test: 
                return std::make_tuple( true, nullptr, reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), nullptr, nullptr, nullptr );
        }
    }

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( true, level + 1, maxLevelCount, camera, blockActors, lightsVector, renderedViewLevel, activeViewLevel, activeViewType );

    if ( frameReceived )
        return std::make_tuple( true, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    std::tie( frameReceived, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
        = renderReflectionsRefractions( false, level + 1, maxLevelCount, camera, blockActors, lightsVector, renderedViewLevel, activeViewLevel, activeViewType );

    if ( frameReceived )
        return std::make_tuple( true, frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat );

    OutputDebugStringW( StringUtil::widen( "\n" ).c_str() );

    renderedViewLevel.pop_back();

    return std::make_tuple( false, nullptr, nullptr, nullptr, nullptr, nullptr );
}

void Renderer::renderFirstReflections( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector )
{
    // Perform reflection shading.
    reflectionRefractionShadingRenderer.performFirstReflectionShading( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget(), deferredRenderer.getAlbedoRenderTarget(),
                                                                  deferredRenderer.getMetalnessRenderTarget(), deferredRenderer.getRoughnessRenderTarget() );

    // Perform ray tracing.
    raytraceRenderer.generateAndTraceFirstReflectedRays( camera, deferredRenderer.getPositionRenderTarget(), deferredRenderer.getNormalRenderTarget(),
                                                            deferredRenderer.getRoughnessRenderTarget(), reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), blockActors );

    // Perform shading on the reflected image.
    /*shadingRenderer.performShading( camera, raytraceRenderer.getRayHitPositionTexture(), raytraceRenderer.getRayHitEmissiveTexture(), 
                                    raytraceRenderer.getRayHitAlbedoTexture(), raytraceRenderer.getRayHitMetalnessTexture(),
                                    raytraceRenderer.getRayHitRoughnessTexture(), raytraceRenderer.getRayHitNormalTexture(),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture(), lightsVector );*/

    shadingRenderer.performShading( raytraceRenderer.getRayOriginsTexture( 0 ), 
                                    raytraceRenderer.getRayHitPositionTexture( 0 ), 
                                    raytraceRenderer.getRayHitEmissiveTexture( 0 ), 
                                    raytraceRenderer.getRayHitAlbedoTexture( 0 ), 
                                    raytraceRenderer.getRayHitMetalnessTexture( 0 ),
                                    raytraceRenderer.getRayHitRoughnessTexture( 0 ), 
                                    raytraceRenderer.getRayHitNormalTexture( 0 ),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture( 0 ), 
                                    lightsVector );

    // Generate mipmaps for the shaded, reflected image.
    shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    // Combine main image with reflections.
    combiningRenderer.combine( finalRenderTarget, 
                               shadingRenderer.getColorRenderTarget(), 
                               reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                               deferredRenderer.getNormalRenderTarget(), 
                                deferredRenderer.getPositionRenderTarget(), 
                                deferredRenderer.getDepthRenderTarget(), 
                                raytraceRenderer.getRayHitDistanceTexture( 0 ),
                                camera.getPosition() );
}

void Renderer::renderFirstRefractions( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector )
{
    // Perform refraction shading.
    reflectionRefractionShadingRenderer.performFirstRefractionShading( camera, 
                                                                       deferredRenderer.getPositionRenderTarget(), 
                                                                       deferredRenderer.getNormalRenderTarget(), 
                                                                       deferredRenderer.getAlbedoRenderTarget(),
                                                                       deferredRenderer.getMetalnessRenderTarget(), 
                                                                       deferredRenderer.getRoughnessRenderTarget() );

    // Perform ray tracing.
    raytraceRenderer.generateAndTraceFirstRefractedRays( camera, 
                                                         deferredRenderer.getPositionRenderTarget(), 
                                                         deferredRenderer.getNormalRenderTarget(),
                                                         deferredRenderer.getRoughnessRenderTarget(), 
                                                         reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                                                         blockActors );

    // Perform shading on the refraction image.
    /*shadingRenderer.performShading( camera, raytraceRenderer.getRayHitPositionTexture(), raytraceRenderer.getRayHitEmissiveTexture(), 
                                    raytraceRenderer.getRayHitAlbedoTexture(), raytraceRenderer.getRayHitMetalnessTexture(),
                                    raytraceRenderer.getRayHitRoughnessTexture(), raytraceRenderer.getRayHitNormalTexture(),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture(), lightsVector );*/

    shadingRenderer.performShading( raytraceRenderer.getRayOriginsTexture( 0 ), 
                                    raytraceRenderer.getRayHitPositionTexture( 0 ), 
                                    raytraceRenderer.getRayHitEmissiveTexture( 0 ), 
                                    raytraceRenderer.getRayHitAlbedoTexture( 0 ), 
                                    raytraceRenderer.getRayHitMetalnessTexture( 0 ),
                                    raytraceRenderer.getRayHitRoughnessTexture( 0 ), 
                                    raytraceRenderer.getRayHitNormalTexture( 0 ),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture( 0 ), 
                                    lightsVector );

    // Generate mipmaps for the shaded, reflected image.
    shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    // Combine main image with refractions.
    combiningRenderer.combine( finalRenderTarget, 
                               shadingRenderer.getColorRenderTarget(), 
                               reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( 0 ), 
                               deferredRenderer.getNormalRenderTarget(), 
                               deferredRenderer.getPositionRenderTarget(), 
                               deferredRenderer.getDepthRenderTarget(), 
                               raytraceRenderer.getRayHitDistanceTexture( 0 ),
                               camera.getPosition() );
}

void Renderer::renderReflections( const int level, const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector )
{

    reflectionRefractionShadingRenderer.performReflectionShading( level - 1, 
                                                                  raytraceRenderer.getRayOriginsTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitAlbedoTexture( level - 2 ),
                                                                  raytraceRenderer.getRayHitMetalnessTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitRoughnessTexture( level - 2 ) );

    raytraceRenderer.generateAndTraceReflectedRays( level - 1,
                                                    /*raytraceRenderer.getRayDirectionsTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitRoughnessTexture( level - 2 ), */
                                                    reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                                                    blockActors );

    // Perform shading on the reflected image.
    shadingRenderer.performShading( raytraceRenderer.getRayOriginsTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitPositionTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitEmissiveTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitAlbedoTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
                                    raytraceRenderer.getRayHitRoughnessTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitNormalTexture( level - 1 ),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture( level - 1 ), 
                                    lightsVector );

    // Generate mipmaps for the shaded, reflected image.
    shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    combiningRenderer.combine( finalRenderTarget, 
                               shadingRenderer.getColorRenderTarget(), 
                               reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                               raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                               raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                               deferredRenderer.getDepthRenderTarget(), 
                               raytraceRenderer.getRayHitDistanceTexture( level - 1 ), // ARRRRREEEEEEEEE YOU SUUUUREEEEEEEEEEEE???????
                               camera.getPosition() );
}

void Renderer::renderRefractions( const int level, const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& blockActors, const std::vector< std::shared_ptr< Light > >& lightsVector )
{
    reflectionRefractionShadingRenderer.performRefractionShading( level - 1, 
                                                                  raytraceRenderer.getRayOriginsTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitAlbedoTexture( level - 2 ),
                                                                  raytraceRenderer.getRayHitMetalnessTexture( level - 2 ), 
                                                                  raytraceRenderer.getRayHitRoughnessTexture( level - 2 ) );

    raytraceRenderer.generateAndTraceRefractedRays( level - 1,
                                                    /*raytraceRenderer.getRayDirectionsTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                                                    raytraceRenderer.getRayHitRoughnessTexture( level - 2 ), */
                                                    reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ), 
                                                    blockActors );

    // Perform shading on the reflected image.
    shadingRenderer.performShading( raytraceRenderer.getRayOriginsTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitPositionTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitEmissiveTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitAlbedoTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitMetalnessTexture( level - 1 ),
                                    raytraceRenderer.getRayHitRoughnessTexture( level - 1 ), 
                                    raytraceRenderer.getRayHitNormalTexture( level - 1 ),
                                    raytraceRenderer.getRayHitIndexOfRefractionTexture( level - 1 ), 
                                    lightsVector );

    // Generate mipmaps for the shaded, reflected image.
    shadingRenderer.getColorRenderTarget()->generateMipMapsOnGpu( *deviceContext.Get() );

    combiningRenderer.combine( finalRenderTarget, 
                               shadingRenderer.getColorRenderTarget(), 
                               reflectionRefractionShadingRenderer.getContributionTermRoughnessTarget( level - 1 ),
                               raytraceRenderer.getRayHitNormalTexture( level - 2 ), 
                               raytraceRenderer.getRayHitPositionTexture( level - 2 ), 
                               deferredRenderer.getDepthRenderTarget(), 
                               raytraceRenderer.getRayHitDistanceTexture( level - 1 ),
                               camera.getPosition() );
}

void Renderer::setActiveViewType( const View view )
{
    activeViewType = view;
}

Renderer::View Renderer::getActiveViewType() const
{
    return activeViewType;
}

void Renderer::activateNextViewLevel( const bool reflection )
{
    if ( activeViewLevel.size() < maxLevelCount )
        activeViewLevel.push_back( reflection );
}

void Renderer::activatePrevViewLevel()
{
    if ( !activeViewLevel.empty() )
        activeViewLevel.pop_back();
}

const std::vector< bool >& Renderer::getActiveViewLevel() const
{
    return activeViewLevel;
}

void Renderer::setMaxLevelCount( const int levelCount )
{
    maxLevelCount = std::max( 0, levelCount );

    if ( activeViewLevel.size() >= maxLevelCount )
        activeViewLevel.resize( maxLevelCount );
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
