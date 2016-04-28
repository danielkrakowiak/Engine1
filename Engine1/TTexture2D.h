#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DGeneric.h"
#include "Texture2DSpecializedUsage.h"
#include "Texture2DSpecializedBinding.h"

// #TODO:
// Constructor from raw data could take iterators rather than the whole vector.

namespace Engine1
{
    template< TexUsage usage, TexBind binding, typename PixelType >
    class TTexture2D
    {
        protected: 

        //virtual bool isUsageBindingSupported()
        //{
        //    // Any usage/binding configurations other than the below template specializations are not supported.
        //    return false;
        //}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Immutable, PixelType >,
           public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Dynamic, PixelType >,
           public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::RenderTarget, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::RenderTarget, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::DepthStencil, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::DepthStencil, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::UnorderedAccess, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::RenderTarget_ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::RenderTarget_UnorderedAccess, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::DepthStencil_ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::UnorderedAccess_ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType >
        : public Texture2DSpecializedUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecializedBinding< TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}


        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}
    };
}
