#include "RasterizeShadowRenderer.h"

#include <d3d11_3.h>

#include "Direct3DRendererCore.h"
#include "RasterizingShadowsComputeShader.h"
#include "uint3.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"
#include "Texture2Dtypes.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RasterizeShadowRenderer::RasterizeShadowRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_rasterizeShadowsComputeShader( std::make_shared< RasterizingShadowsComputeShader >() )
{}

RasterizeShadowRenderer::~RasterizeShadowRenderer()
{}

void RasterizeShadowRenderer::initialize(
    int imageWidth,
    int imageHeight,
    Microsoft::WRL::ComPtr< ID3D11Device3 > device,
    Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;

    createComputeTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( device );

    m_initialized = true;
}

void RasterizeShadowRenderer::performShadowMapping(
    const float3& cameraPos,
    const std::shared_ptr< Light > light,
    const std::shared_ptr< Texture2D< float4 > > surfacePositionTexture,
    const std::shared_ptr< Texture2D< float4 > > surfaceNormalTexture
    )
{
    m_rendererCore.disableRenderingPipeline();

    m_rendererCore.enableComputeShader( m_rasterizeShadowsComputeShader );

    // Clear unordered access targets. #TODO: Probably not needed for shadow mapping? Every value will be written.
    m_shadowTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 0, 0, 0, 255 ) );
    m_distanceToOccluderTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 1000, 1000, 1000, 1000 ) );

	RenderTargets unorderedAccessTargets;

    unorderedAccessTargets.typeFloat.push_back( m_distanceToOccluderTexture );
	unorderedAccessTargets.typeUchar.push_back( m_shadowTexture );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = m_imageWidth; //rayOriginTexture->getWidth();
    const int imageHeight = m_imageHeight; //rayOriginTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rasterizeShadowsComputeShader->setParameters(
        *m_deviceContext.Get(), cameraPos, *light, *surfacePositionTexture, *surfaceNormalTexture, 
        imageWidth, imageHeight
        );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableRenderTargets();

    m_rasterizeShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< RenderTargetTexture2D< unsigned char > > RasterizeShadowRenderer::getShadowTexture()
{
    return m_shadowTexture;
}

std::shared_ptr< RenderTargetTexture2D< float > > RasterizeShadowRenderer::getDistanceToOccluder()
{
    return m_distanceToOccluderTexture;
}

void RasterizeShadowRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device3& device )
{
    // #TODO: Is using mipmaps? Disable them if they are not necessary.
    m_shadowTexture = std::make_shared< RenderTargetTexture2D< unsigned char > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM );

    // #TODO: Is using mipmaps? Disable them if they are not necessary.
    m_distanceToOccluderTexture = std::make_shared< RenderTargetTexture2D< float > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT );
}

void RasterizeShadowRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_rasterizeShadowsComputeShader->loadAndInitialize( "Engine1/Shaders/RasterizingShadowsShader/RasterizingShadows_cs.cso", device );
}


