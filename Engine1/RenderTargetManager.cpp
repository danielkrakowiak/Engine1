#include "RenderTargetManager.h"

using namespace Engine1;

RenderTargetManager::RenderTargetManager() :
    m_initialized( false )
{}

RenderTargetManager::~RenderTargetManager()
{}

void RenderTargetManager::initialize( ID3D11Device& device, int2 imageDimensions )
{
    const int rtFloatCount       = 1;
    const int rtFloat4Count      = 2;
    const int rtUcharCount       = 3;
    const int rtUchar4Count      = 3;
    const int rtDepthUchar4Count = 1;

    const bool storeOnCpu           = false;
    const bool storeOnGpu           = true;
    const bool generateMipmaps      = true;
    const bool generateMipmapsDepth = false;

    // Create float textures.
    for ( int i = 0; i < rtFloatCount; ++i ) {
        auto renderTarget =
            std::make_shared<
                Texture2D<
                    TexUsage::Default,
                    TexBind::RenderTarget_UnorderedAccess_ShaderResource,
                    float
                >
            >(
                device,
                imageDimensions.x,
                imageDimensions.y,
                storeOnCpu,
                storeOnGpu,
                generateMipmaps,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_R32_FLOAT,
                DXGI_FORMAT_R32_FLOAT
                );

        m_renderTargetsFloat.push_back( renderTarget );
    }

    // Create float4 textures.
    for ( int i = 0; i < rtFloat4Count; ++i ) {
        auto renderTarget =
            std::make_shared<
                Texture2D<
                    TexUsage::Default,
                    TexBind::RenderTarget_UnorderedAccess_ShaderResource,
                    float4
                >
            >(
                device,
                imageDimensions.x,
                imageDimensions.y,
                storeOnCpu,
                storeOnGpu,
                generateMipmaps,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                DXGI_FORMAT_R32G32B32A32_FLOAT
                );

        m_renderTargetsFloat4.push_back( renderTarget );
    }

    // Create uchar textures.
    for ( int i = 0; i < rtUcharCount; ++i ) {
        auto renderTarget =
            std::make_shared<
                Texture2D<
                    TexUsage::Default,
                    TexBind::RenderTarget_UnorderedAccess_ShaderResource,
                    unsigned char
                >
            >(
                device,
                imageDimensions.x,
                imageDimensions.y,
                storeOnCpu,
                storeOnGpu,
                generateMipmaps,
                DXGI_FORMAT_R8_UNORM,
                DXGI_FORMAT_R8_UNORM,
                DXGI_FORMAT_R8_UNORM,
                DXGI_FORMAT_R8_UNORM
                );

        m_renderTargetsUchar.push_back( renderTarget );
    }

    // Create uchar4 textures.
    for ( int i = 0; i < rtUchar4Count; ++i ) {
        auto renderTarget =
            std::make_shared<
                Texture2D<
                    TexUsage::Default,
                    TexBind::RenderTarget_UnorderedAccess_ShaderResource,
                    uchar4
                >
            >(
                device,
                imageDimensions.x,
                imageDimensions.y,
                storeOnCpu,
                storeOnGpu,
                generateMipmaps,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM
                );

        m_renderTargetsUchar4.push_back( renderTarget );
    }

    // Create uchar4 textures.
    for ( int i = 0; i < rtDepthUchar4Count; ++i ) {
        auto renderTarget =
            std::make_shared<
                Texture2D<
                    TexUsage::Default,
                    TexBind::DepthStencil_ShaderResource,
                    uchar4
                >
            >(
                device,
                imageDimensions.x,
                imageDimensions.y,
                storeOnCpu,
                storeOnGpu,
                generateMipmapsDepth,
                DXGI_FORMAT_R24G8_TYPELESS,
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                DXGI_FORMAT_R24_UNORM_X8_TYPELESS
                );

        m_renderTargetsDepthUchar4.push_back( renderTarget );
    }

    m_initialized = true;
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > 
RenderTargetManager::getRenderTarget( const int2 imageDimensions )
{
    if ( !m_initialized )
        throw std::exception( "RenderTargetManager::getRenderTarget - manager is not initialzied." );

    for ( auto& renderTarget : m_renderTargetsFloat )
    {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type/dimensions is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
RenderTargetManager::getRenderTarget( const int2 imageDimensions )
{
    if ( !m_initialized )
        throw std::exception( "RenderTargetManager::getRenderTarget - manager is not initialzied." );

    for ( auto& renderTarget : m_renderTargetsFloat4 ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type/dimensions is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > >
RenderTargetManager::getRenderTarget( const int2 imageDimensions )
{
    if ( !m_initialized )
        throw std::exception( "RenderTargetManager::getRenderTarget - manager is not initialzied." );

    for ( auto& renderTarget : m_renderTargetsUchar ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type/dimensions is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
RenderTargetManager::getRenderTarget( const int2 imageDimensions )
{
    if ( !m_initialized )
        throw std::exception( "RenderTargetManager::getRenderTarget - manager is not initialzied." );

    for ( auto& renderTarget : m_renderTargetsUchar4 ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type/dimensions is not available." );
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > 
RenderTargetManager::getRenderTargetDepth( const int2 imageDimensions )
{
    if ( !m_initialized )
        throw std::exception( "RenderTargetManager::getRenderTarget - manager is not initialzied." );

    for ( auto& renderTarget : m_renderTargetsDepthUchar4 ) {
        if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTargetDepth() - render target of reqested type/dimensions is not available." );
}
