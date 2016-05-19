#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
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

        ID3D11ShaderResourceView* getShaderResourceView() const
        {
            return m_shaderResourceView.Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView() const
        {
            return m_renderTargetView.Get();
        }

        void clearRenderTargetView( ID3D11DeviceContext& deviceContext, float4 colorRGBA )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget >::clearRenderTargetView - Texture not in GPU memory." );

	        deviceContext.ClearRenderTargetView( m_renderTargetView.Get(), colorRGBA.getData() );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::DepthStencil, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView() const
        {
            return m_depthStencilView.Get();
        }

        void clearDepthStencilView( ID3D11DeviceContext& deviceContext, bool clearDepth, float depth, bool clearStencil, unsigned char stencil )
        {
            if ( !clearDepth && !clearStencil )
		        return;

	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::DepthStencil >::clearDepthStencilView - Texture not in GPU memory." );

	        UINT flags = 0;
	        if ( clearDepth )   flags |= D3D11_CLEAR_DEPTH;
	        if ( clearStencil ) flags |= D3D11_CLEAR_STENCIL;

	        deviceContext.ClearDepthStencilView( m_depthStencilView.Get(), flags, depth, stencil );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::UnorderedAccess, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11UnorderedAccessView* getUnorderedAccessView() const
        {
            return m_unorderedAccessView.Get();
        }

        // TODO: maybe the input type should be PixelType and based on it the uint/float function gets called.
        void clearUnorderedAccessViewUint( ID3D11DeviceContext& deviceContext, uint4 value )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewUint - Texture not in GPU memory." );
            
	        deviceContext.ClearUnorderedAccessViewUint( m_unorderedAccessView.Get(), value.getData() );
        }

        // TODO: maybe the input type should be PixelType and based on it the uint/float function gets called.
        void clearUnorderedAccessViewFloat( ID3D11DeviceContext& deviceContext, float4 value )
        {
	        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DSpecBind< TexBind::UnorderedAccess >::clearUnorderedAccessViewFloat - Texture not in GPU memory." );
            
            float val[4] = { value.x, value.y, value.z, value.w };
	        deviceContext.ClearUnorderedAccessViewFloat( m_unorderedAccessView.Get(), val );
        }
    };

    template< typename PixelType >
    class Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, PixelType > 
        : public Texture2DSpecBind< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecBind< TexBind::ShaderResource, PixelType > 
    {
        public:

        void generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext )
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

        void generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext )
        {
            if ( !isInGpuMemory() )
                throw std::exception( "Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource >::generateMipMapsOnGpu - Can't generate because texture is not in GPU memory." );

            deviceContext.GenerateMips( m_shaderResourceView.Get() );

            m_hasMipmapsOnGpu = true;
        }
    };
}