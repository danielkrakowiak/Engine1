#include "DistanceToOccluderSearchRenderer.h"

#include "DX11RendererCore.h"

#include "DistanceToOccluderSearchComputeShader.h"
#include "Camera.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

DistanceToOccluderSearchRenderer::DistanceToOccluderSearchRenderer( DX11RendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_distanceToOccluderSearchComputeShader( std::make_shared< DistanceToOccluderSearchComputeShader >() )
{}

DistanceToOccluderSearchRenderer::~DistanceToOccluderSearchRenderer()
{}

void DistanceToOccluderSearchRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device,
                 ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void DistanceToOccluderSearchRenderer::performDistanceToOccluderSearch( 
    const Camera& camera,
    const float positionThreshold,
    const float normalThreshold,
    const float searchRadiusInShadow,
    const float searchStepInShadow,
    const float searchRadiusInLight,
    const float searchStepInLight,
    const int searchMipmapLevel,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< float > > distanceToOccluder,
    const std::shared_ptr< RenderTargetTexture2D< float > > finalDistanceToOccluderRenderTarget,
    const Light& light 
)
{
    m_rendererCore.disableRenderingPipeline();

    m_distanceToOccluderSearchComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        positionThreshold,
        normalThreshold,
        searchRadiusInShadow,
        searchStepInShadow,
        searchRadiusInLight,
        searchStepInLight,
        searchMipmapLevel,
        positionTexture,
        normalTexture, 
        distanceToOccluder, 
        finalDistanceToOccluderRenderTarget->getDimensions(),
        light 
    );

    m_rendererCore.enableComputeShader( m_distanceToOccluderSearchComputeShader );

	RenderTargets unorderedAccessTargets;

    //#TODO: Remove that clear - just for debug.
    finalDistanceToOccluderRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4::ZERO, 0 );

	unorderedAccessTargets.typeFloat.push_back( finalDistanceToOccluderRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = finalDistanceToOccluderRenderTarget->getWidth();
    const int imageHeight = finalDistanceToOccluderRenderTarget->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_distanceToOccluderSearchComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void DistanceToOccluderSearchRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_distanceToOccluderSearchComputeShader->loadAndInitialize( "Engine1/Shaders/DistanceToOccluderSearchShader/DistanceToOccluderSearch_cs.cso", device );
}