#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DEnums.h"
#include "Texture2DGeneric.h"

namespace Engine1
{
    template< TexBind binding, typename PixelType >
    class Texture2DSpecializedBinding
    {};

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11ShaderResourceView* getShaderResourceView() const
        {
            return m_shaderResourceView.Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::RenderTarget, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView() const
        {
            return m_renderTargetView.Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::DepthStencil, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView() const
        {
            return m_depthStencilView.Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::UnorderedAccess, PixelType > 
        : public virtual Texture2DGeneric< PixelType >
    {
        public:

        ID3D11UnorderedAccessView* getUnorderedAccessView() const
        {
            return m_unorderedAccessView.Get();
        }
    };

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::RenderTarget_ShaderResource, PixelType > 
        : public Texture2DSpecializedBinding< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType > 
    {};

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::RenderTarget_UnorderedAccess, PixelType > 
        : public Texture2DSpecializedBinding< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecializedBinding< TexBind::UnorderedAccess, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::DepthStencil_ShaderResource, PixelType > 
        : public Texture2DSpecializedBinding< TexBind::DepthStencil, PixelType >, 
          public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::UnorderedAccess_ShaderResource, PixelType > 
        : public Texture2DSpecializedBinding< TexBind::UnorderedAccess, PixelType >,
          public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {};

    template< typename PixelType >
    class Texture2DSpecializedBinding< TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > 
        : public Texture2DSpecializedBinding< TexBind::RenderTarget, PixelType >, 
          public Texture2DSpecializedBinding< TexBind::UnorderedAccess, PixelType >, 
          public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {};
}