#include "RenderTargetManager.h"

using namespace Engine1;

RenderTargetManager::RenderTargetManager( ID3D11Device& device, int2 imageDimensions )
{
    const int rtFloatCount   = 1;
    const int rtFloat4Count  = 1;
    const int rtUcharCount   = 1;
    const int rtUchar4Count  = 1;

    const bool storeOnCpu      = false;
    const bool storeOnGpu      = true;
    const bool generateMipmaps = true;

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
    for ( int i = 0; i < rtFloat4Count; ++i )
    {
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
}

RenderTargetManager::~RenderTargetManager()
{}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > 
RenderTargetManager::getRenderTarget()
{
    for ( auto& renderTarget : m_renderTargetsFloat )
    {
        if ( renderTarget.use_count() == 1 )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >
RenderTargetManager::getRenderTarget()
{
    for ( auto& renderTarget : m_renderTargetsFloat4 ) {
        if ( renderTarget.use_count() == 1 )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > >
RenderTargetManager::getRenderTarget()
{
    for ( auto& renderTarget : m_renderTargetsUchar ) {
        if ( renderTarget.use_count() == 1 )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type is not available." );
}

template <>
std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >
RenderTargetManager::getRenderTarget()
{
    for ( auto& renderTarget : m_renderTargetsUchar4 ) {
        if ( renderTarget.use_count() == 1 )
            return renderTarget;
    }

    throw std::exception( "RenderTargetManager::getRenderTarget() - render target of reqested type is not available." );
}
