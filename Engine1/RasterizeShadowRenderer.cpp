#include "RasterizeShadowRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "RasterizingShadowsComputeShader.h"
#include "uint3.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"

using namespace Engine1;

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
    Microsoft::WRL::ComPtr< ID3D11Device > device,
    Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;

    createComputeTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( *device.Get() );

    m_initialized = true;
}

void RasterizeShadowRenderer::performShadowMapping(
    const std::shared_ptr< Light > light,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > surfaceNormalTexture
    /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,*/
    )
{
    m_rendererCore.disableRenderingPipeline();

    m_rendererCore.enableComputeShader( m_rasterizeShadowsComputeShader );

    // Clear unordered access targets. #TODO: Probably not needed for shadow mapping? Every value will be written.
    m_illuminationTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 255, 255, 255 ) );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsU1.push_back( m_illuminationTexture );

    m_rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    const int imageWidth = m_imageWidth; //rayOriginTexture->getWidth();
    const int imageHeight = m_imageHeight; //rayOriginTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rasterizeShadowsComputeShader->setParameters(
        *m_deviceContext.Get(), *light, *rayOriginTexture, *surfaceNormalTexture, 
        imageWidth, imageHeight
        );

    m_rendererCore.compute( groupCount );

    // Unbind resources to avoid binding the same resource on input and output.
    m_rendererCore.disableUnorderedAccessViews();

    m_rasterizeShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > RasterizeShadowRenderer::getIlluminationTexture()
{
    return m_illuminationTexture;
}

void RasterizeShadowRenderer::createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    // #TODO: Is using mipmaps? Disable them if they are not necessary.
    m_illuminationTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, true, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM );
}

void RasterizeShadowRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    m_rasterizeShadowsComputeShader->compileFromFile( "Shaders/RasterizingShadowsShader/cs.hlsl", device );
}


