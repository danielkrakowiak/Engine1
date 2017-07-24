#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11_3.h>
#include <wrl.h>

#include "Texture2DEnums.h"
#include "Texture2DGeneric.h"

#include "uint4.h"

namespace Engine1
{
    template< TexBind binding, typename PixelType >
    class Texture2DSpecBind
    {};

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::ShaderResource, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        // mipmapLevel = -1 - returns view to all mipmaps.
        // mipmapLevel >= 0 - returns view to a specific mipmap.
        ID3D11ShaderResourceView* getShaderResourceView( int mipmapLevel = -1 ) const
        {
            if ( mipmapLevel < -1 || mipmapLevel >= (int)m_shaderResourceViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::ShaderResource >::getShaderResourceView - Tried to access shader resource view for non-existing mipmap level." );

            if ( mipmapLevel == -1 )
                return m_shaderResourceView.Get();
            else
                return m_shaderResourceViews[ mipmapLevel ].Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView( int mipmapLevel = 0 ) const
        {
            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_renderTargetViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget >::getRenderTargetView - Tried to access render target view for non-existing mipmap level." );

            return m_renderTargetViews[ mipmapLevel ].Get();
        }

        void clearRenderTargetView( ID3D11DeviceContext3& deviceContext, float4 colorRGBA, int mipmapLevel = 0 )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget >::clearRenderTargetView - Texture not in GPU memory." );

            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_renderTargetViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget >::clearRenderTargetView - Tried to clear render target view for non-existing mipmap level." );

	        deviceContext.ClearRenderTargetView( m_renderTargetViews[ mipmapLevel ].Get(), colorRGBA.getData() );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::DepthStencil, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView( int mipmapLevel = 0 ) const
        {
            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_depthStencilViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::DepthStencil >::getDepthStencilView - Tried to access depth stencil view for non-existing mipmap level." );

            return m_depthStencilViews[ mipmapLevel ].Get();
        }

        void clearDepthStencilView( ID3D11DeviceContext3& deviceContext, bool clearDepth, float depth, bool clearStencil, unsigned char stencil, int mipmapLevel = 0 )
        {
            if ( !clearDepth && !clearStencil )
		        return;

	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::DepthStencil >::clearDepthStencilView - Texture not in GPU memory." );

            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_depthStencilViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::DepthStencil >::clearDepthStencilView - Tried to clear depth stencil view for non-existing mipmap level." );

	        UINT flags = 0;
	        if ( clearDepth )   flags |= D3D11_CLEAR_DEPTH;
	        if ( clearStencil ) flags |= D3D11_CLEAR_STENCIL;

	        deviceContext.ClearDepthStencilView( m_depthStencilViews[ mipmapLevel ].Get(), flags, depth, stencil );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::UnorderedAccess, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11UnorderedAccessView* getUnorderedAccessView( int mipmapLevel = 0 ) const
        {
            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_unorderedAccessViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::getUnorderedAccessView - Tried to access unordered access view for non-existing mipmap level." );

            return m_unorderedAccessViews[ mipmapLevel ].Get();
        }

        // TODO: maybe the input type should be PixelType and based on it the uint/float function gets called.
        void clearUnorderedAccessViewUint( ID3D11DeviceContext3& deviceContext, uint4 value, int mipmapLevel = 0 )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewUint - Texture not in GPU memory." );

            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_unorderedAccessViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewUint - Tried to clear unordered access view for non-existing mipmap level." );
            
	        deviceContext.ClearUnorderedAccessViewUint( m_unorderedAccessViews[ mipmapLevel ].Get(), value.getData() );
        }

        // TODO: maybe the input type should be PixelType and based on it the uint/float function gets called.
        void clearUnorderedAccessViewFloat( ID3D11DeviceContext3& deviceContext, float4 value, int mipmapLevel = 0 )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewFloat - Texture not in GPU memory." );

            if ( mipmapLevel < 0 || mipmapLevel >= (int)m_unorderedAccessViews.size() )
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewFloat - Tried to clear unordered access view for non-existing mipmap level." );
            
            float val[4] = { value.x, value.y, value.z, value.w };
	        deviceContext.ClearUnorderedAccessViewFloat( m_unorderedAccessViews[ mipmapLevel ].Get(), val );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, PixelType > 
        : public Texture2DSpecBind< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecBind< TexBind::ShaderResource, PixelType > 
    {
        public:

        void generateMipMapsOnGpu( ID3D11DeviceContext3& deviceContext )
        {
            if ( !isInGpuMemory() )
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget_ShaderResource >::generateMipMapsOnGpu - Can't generate because texture is not in GPU memory." );

            deviceContext.GenerateMips( m_shaderResourceView.Get() );

            m_hasMipmapsOnGpu = true;
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess, PixelType > 
        : public Texture2DSpecBind< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecBind< TexBind::UnorderedAccess, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, PixelType > 
        : public Texture2DSpecBind< TexBind::DepthStencil, PixelType >, 
          public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, PixelType > 
        : public Texture2DSpecBind< TexBind::UnorderedAccess, PixelType >,
          public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > 
        : public Texture2DSpecBind< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecBind< TexBind::UnorderedAccess, PixelType >, 
          public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {
        public:

        void generateMipMapsOnGpu( ID3D11DeviceContext3& deviceContext )
        {
            if ( !isInGpuMemory() )
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource >::generateMipMapsOnGpu - Can't generate because texture is not in GPU memory." );

            deviceContext.GenerateMips( m_shaderResourceView.Get() );

            m_hasMipmapsOnGpu = true;
        }
    };
}