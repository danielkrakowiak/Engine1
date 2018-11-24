#pragma once

#include <vector>
#include <memory>
#include <wrl.h>
#include <tuple>

#include "Texture2DTypes.h"
#include "float4.h"
#include "int2.h"
#include "Direct3DUtil.h"

namespace Engine1
{
    class RenderTargetManager
    {
        public:

        RenderTargetManager();
        ~RenderTargetManager();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        template < typename T >
        std::shared_ptr< RenderTargetTexture2D< T > > 
            getRenderTarget( const int2 imageDimensions, const bool reducedPrecision, const std::string debugName = "" );

        std::shared_ptr< DepthTexture2D< uchar4 > > 
            getRenderTargetDepth( const int2 imageDimensions, const std::string debugName = "" );

        // Debug methods to keep track of render target usage.
        template< typename T >
        int getTotalRenderTargetCount();
        int getTotalRenderTargetDepthCount();

        private:

        // Helper method to get a set of all render targets for given pixel type.
        template< typename T >
        std::vector< std::shared_ptr< RenderTargetTexture2D< T > > >& 
            getAllRenderTargets();

        template < typename T >
        std::shared_ptr< RenderTargetTexture2D< T > > 
            createRenderTarget( const int2 imageDimensions, const bool reducedPrecision );

        std::shared_ptr< DepthTexture2D< uchar4 > >
            createRenderTargetDepth( const int2 imageDimensions );

        template< typename T >
        std::tuple< DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT, DXGI_FORMAT > 
            getViewFormatsForPixelType( const bool reducedPrecision );

        Microsoft::WRL::ComPtr< ID3D11Device3 > m_device;

        std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >         m_renderTargetsFloat;
        std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >        m_renderTargetsFloat3;
        std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >        m_renderTargetsFloat4;
        std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > > m_renderTargetsUchar;
        std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >        m_renderTargetsUchar4;
        std::vector< std::shared_ptr< DepthTexture2D< uchar4 > > >               m_renderTargetsDepthUchar4;
    };

    template < typename T >
    std::shared_ptr< RenderTargetTexture2D< T > >
        RenderTargetManager::getRenderTarget( const int2 imageDimensions, const bool reducedPrecision, const std::string debugName )
    {
        auto& renderTargets = getAllRenderTargets< T >();
        for ( auto& renderTarget : renderTargets ) {
            if ( renderTarget.use_count() == 1 
                && renderTarget->getDimensions() == imageDimensions 
                && (reducedPrecision == isReducedPrecisionFormat( renderTarget->getTextureFormat() ) ))
            {
                Direct3DUtil::setResourceName( *renderTarget->getTextureResource().Get(), debugName );

                return renderTarget;
            }
        }

        // Render target not found - create a new one.
        auto renderTarget = createRenderTarget< T >( imageDimensions, reducedPrecision );

        renderTargets.push_back( renderTarget );

        return renderTarget;
    }

    template < typename T >
    std::shared_ptr< RenderTargetTexture2D< T > >
        RenderTargetManager::createRenderTarget( const int2 imageDimensions, const bool reducedPrecision )
    {
        DXGI_FORMAT view1, view2, view3, view4;
        std::tie( view1, view2, view3, view4 ) = getViewFormatsForPixelType< T >( reducedPrecision );

        const bool storeOnCpu      = false;
        const bool storeOnGpu      = true;
        const bool generateMipmaps = true;

        return std::make_shared< RenderTargetTexture2D< T > >(
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

    static bool isReducedPrecisionFormat(DXGI_FORMAT format)
    {
        return (format == DXGI_FORMAT_R11G11B10_FLOAT 
            || format == DXGI_FORMAT_R16G16B16A16_FLOAT
            || format == DXGI_FORMAT_R16_FLOAT);
    }
}

