#include "RenderTargetManager.h"

#include <tuple>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RenderTargetManager::RenderTargetManager()
{}

RenderTargetManager::~RenderTargetManager()
{}

void RenderTargetManager::initialize( ComPtr< ID3D11Device3 >& device )
{
    m_device = device;
}

std::shared_ptr< DepthTexture2D< uchar4 > > 
RenderTargetManager::getRenderTargetDepth( const int2 imageDimensions, const std::string debugName )
{
    for ( auto& renderTarget : m_renderTargetsDepthUchar4 ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
        {
            Direct3DUtil::setResourceName( *renderTarget->getTextureResource().Get(), debugName );

            return renderTarget;
        }
    }

    // Render target not found - create a new one.
    auto renderTarget = createRenderTargetDepth( imageDimensions );

    m_renderTargetsDepthUchar4.push_back( renderTarget );

    return renderTarget;
}

int RenderTargetManager::getTotalRenderTargetDepthCount()
{
    return (int)m_renderTargetsDepthUchar4.size();
}

template<>
std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsFloat3;
}

template<>
std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsFloat4;
}

template<>
std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsFloat;
}

template<>
std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsUchar;
}

template<>
std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsUchar4;
}

std::shared_ptr< DepthTexture2D< uchar4 > >
RenderTargetManager::createRenderTargetDepth( const int2 imageDimensions )
{
    const bool storeOnCpu = false;
    const bool storeOnGpu = true;
    const bool generateMipmaps = false;

    return std::make_shared< DepthTexture2D< uchar4 > >(
        *m_device.Get(),
        imageDimensions.x,
        imageDimensions.y,
        storeOnCpu,
        storeOnGpu,
        generateMipmaps,
        DXGI_FORMAT_R24G8_TYPELESS,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_D24_UNORM_S8_UINT
        );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
RenderTargetManager::getViewFormatsForPixelType< float >( const bool reducedPrecision )
{
    DXGI_FORMAT format = (reducedPrecision ? DXGI_FORMAT_R16_FLOAT : DXGI_FORMAT_R32_FLOAT);

    return std::make_tuple( 
        format, 
        format, 
        format, 
        format 
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
RenderTargetManager::getViewFormatsForPixelType< float4 >( const bool reducedPrecision )
{
    DXGI_FORMAT format = (reducedPrecision ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R32G32B32A32_FLOAT);

    return std::make_tuple( 
        format, 
        format, 
        format, 
        format 
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
RenderTargetManager::getViewFormatsForPixelType< float3 >( const bool reducedPrecision )
{
    // Note: DXGI_FORMAT_R32G32B32_FLOAT is not supported on any GPU yet.
    DXGI_FORMAT format = (reducedPrecision ? DXGI_FORMAT_R11G11B10_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT);

    return std::make_tuple( 
        format,  
        format, 
        format, 
        format 
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
RenderTargetManager::getViewFormatsForPixelType< unsigned char >( const bool reducedPrecision )
{
    reducedPrecision; // Unused.

    return std::make_tuple( 
        DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_R8_UINT,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_R8_UNORM
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
RenderTargetManager::getViewFormatsForPixelType< uchar4 >( const bool reducedPrecision )
{
    reducedPrecision; // Unused.

    return std::make_tuple(
        DXGI_FORMAT_R8G8B8A8_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
}