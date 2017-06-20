#include "RenderTargetManager.h"

#include <tuple>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RenderTargetManager::RenderTargetManager()
{}

RenderTargetManager::~RenderTargetManager()
{}

void RenderTargetManager::initialize( ComPtr< ID3D11Device >& device )
{
    m_device = device;
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > 
RenderTargetManager::getRenderTargetDepth( const int2 imageDimensions )
{
    for ( auto& renderTarget : m_renderTargetsDepthUchar4 ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
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
std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsFloat4;
}

template<>
std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsFloat;
}

template<>
std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsUchar;
}

template<>
std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > >&
RenderTargetManager::getAllRenderTargets()
{
    return m_renderTargetsUchar4;
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >
RenderTargetManager::createRenderTargetDepth( const int2 imageDimensions )
{
    const bool storeOnCpu = false;
    const bool storeOnGpu = true;
    const bool generateMipmaps = false;

    return std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >(
        *m_device.Get(),
        imageDimensions.x,
        imageDimensions.y,
        storeOnCpu,
        storeOnGpu,
        generateMipmaps,
        DXGI_FORMAT_R24G8_TYPELESS,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS
        );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > RenderTargetManager::getViewFormatsForPixelType< float >()
{
    return std::make_tuple( 
        DXGI_FORMAT_R32_FLOAT, 
        DXGI_FORMAT_R32_FLOAT, 
        DXGI_FORMAT_R32_FLOAT, 
        DXGI_FORMAT_R32_FLOAT 
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > RenderTargetManager::getViewFormatsForPixelType< float4 >()
{
    return std::make_tuple( 
        DXGI_FORMAT_R32G32B32A32_FLOAT, 
        DXGI_FORMAT_R32G32B32A32_FLOAT, 
        DXGI_FORMAT_R32G32B32A32_FLOAT, 
        DXGI_FORMAT_R32G32B32A32_FLOAT 
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > RenderTargetManager::getViewFormatsForPixelType< unsigned char >()
{
    return std::make_tuple( 
        DXGI_FORMAT_R8_TYPELESS,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_R8_UINT,
        DXGI_FORMAT_R8_UNORM
    );
}

template<>
std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > RenderTargetManager::getViewFormatsForPixelType< uchar4 >()
{
    return std::make_tuple(
        DXGI_FORMAT_R8G8B8A8_TYPELESS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
}
