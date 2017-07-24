#pragma once

#include <vector>
#include <memory>
#include <wrl.h>
#include <tuple>

#include "Texture2D.h"
#include "float4.h"
#include "int2.h"

namespace Engine1
{
    class RenderTargetManager
    {
        public:

        RenderTargetManager();
        ~RenderTargetManager();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        template < typename T >
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > > 
            getRenderTarget( const int2 imageDimensions );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > 
            getRenderTargetDepth( const int2 imageDimensions );

        // Debug methods to keep track of render target usage.
        template< typename T >
        int getTotalRenderTargetCount();
        int getTotalRenderTargetDepthCount();

        private:

        // Helper method to get a set of all render targets for given pixel type.
        template< typename T >
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > > >& 
            getAllRenderTargets();

        template < typename T >
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > > 
            createRenderTarget( const int2 imageDimensions );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >
            createRenderTargetDepth( const int2 imageDimensions );

        template< typename T >
        std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
            getViewFormatsForPixelType();

        Microsoft::WRL::ComPtr< ID3D11Device3 > m_device;

        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > >         m_renderTargetsFloat;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > >        m_renderTargetsFloat4;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > > m_renderTargetsUchar;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > >        m_renderTargetsUchar4;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > >                        m_renderTargetsDepthUchar4;
    };

    template < typename T >
    std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > >
        RenderTargetManager::getRenderTarget( const int2 imageDimensions )
    {
        auto& renderTargets = getAllRenderTargets< T >();
        for ( auto& renderTarget : renderTargets ) {
            if ( renderTarget.use_count() == 1 && renderTarget->getDimensions() == imageDimensions )
                return renderTarget;
        }

        // Render target not found - create a new one.
        auto renderTarget = createRenderTarget< T >( imageDimensions );

        renderTargets.push_back( renderTarget );

        return renderTarget;
    }

    template < typename T >
    std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > >
        RenderTargetManager::createRenderTarget( const int2 imageDimensions )
    {
        DXGI_FORMAT view1, view2, view3, view4;
        std::tie( view1, view2, view3, view4 ) = getViewFormatsForPixelType< T >();

        const bool storeOnCpu      = false;
        const bool storeOnGpu      = true;
        const bool generateMipmaps = true;

        return std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > >(
            *m_device.Get(),
            imageDimensions.x,
            imageDimensions.y,
            storeOnCpu,
            storeOnGpu,
            generateMipmaps,
            view1,
            view2,
            view3,
            view4
        );
    }

    template< typename T >
    int RenderTargetManager::getTotalRenderTargetCount()
    {
        return (int)getAllRenderTargets< T >().size();
    }
}

