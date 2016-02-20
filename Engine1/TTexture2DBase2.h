#pragma once

#include "TTexture2DBase.h"

namespace Engine1
{
    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2
        : public TTexture2DBase< usage, binding, PixelType, format >
    {};

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::ShaderResource, PixelType, format >
    {
        public:

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::RenderTarget, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::RenderTarget, PixelType, format >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView()
        {
            return renderTargetView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::DepthStencil, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::DepthStencil, PixelType, format >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView()
        {
            return depthStencilView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::UnorderedAccess, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::UnorderedAccess, PixelType, format >
    {
        public:

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::RenderTarget_ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::RenderTarget_ShaderResource, PixelType, format >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView()
        {
            return renderTargetView.Get();
        }

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::RenderTarget_UnorderedAccess, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::RenderTarget_UnorderedAccess, PixelType, format >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView()
        {
            return renderTargetView.Get();
        }

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::RenderTarget_UnorderedAccess_ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::RenderTarget_UnorderedAccess_ShaderResource, PixelType, format >
    {
        public:

        ID3D11RenderTargetView* getRenderTargetView()
        {
            return renderTargetView.Get();
        }

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::DepthStencil_ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::DepthStencil_ShaderResource, PixelType, format >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView()
        {
            return depthStencilView.Get();
        }

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::DepthStencil_UnorderedAccess, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::DepthStencil_UnorderedAccess, PixelType, format >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView()
        {
            return depthStencilView.Get();
        }

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::DepthStencil_UnorderedAccess_ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::DepthStencil_UnorderedAccess_ShaderResource, PixelType, format >
    {
        public:

        ID3D11DepthStencilView* getDepthStencilView()
        {
            return depthStencilView.Get();
        }

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }
    };

    template< TTexture2DUsage usage, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase2< usage, TTexture2DBinding::UnorderedAccess_ShaderResource, PixelType, format >
        : public TTexture2DBase< usage, TTexture2DBinding::UnorderedAccess_ShaderResource, PixelType, format >
    {
        public:

        ID3D11UnorderedAccessView* getUnorderedAccessView()
        {
            return unorderedAccessView.Get();
        }

        ID3D11ShaderResourceView* getShaderResourceView()
        {
            return shaderResourceView.Get();
        }
    };
}

