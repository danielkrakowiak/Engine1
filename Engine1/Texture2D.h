#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DGeneric.h"
#include "Texture2DSpecUsage.h"
#include "Texture2DSpecBind.h"

// #TODO:
// Constructor from raw data could take iterators rather than the whole vector.

namespace Engine1
{
    template< TexUsage usage, TexBind binding, typename PixelType >
    class Texture2D
    {
        protected: 

        //virtual bool isUsageBindingSupported()
        //{
        //    // Any usage/binding configurations other than the below template specializations are not supported.
        //    return false;
        //}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Immutable, PixelType >,
           public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Immutable, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Dynamic, PixelType >,
           public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Dynamic, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::ShaderResource, PixelType >
    {
        public:

        // TEMP: Only for use during refactoring. Should be removed! To be used when texture is not loaded, but stores file information.
        Texture2D() : Texture2DGeneric()
        {}

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::ShaderResource, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::RenderTarget, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::RenderTarget, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget, textureFormat, viewFormat )
        {}

        // Specific for render target - useful to create a texture from the frame back buffer.
        Texture2D( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture ) :
            Texture2DGeneric( device, texture )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::DepthStencil, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::DepthStencil, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::UnorderedAccess, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess, textureFormat, viewFormat )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::DepthStencil_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2 )
        {}
    };

    template< typename PixelType >
    class Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType >
        : public Texture2DSpecUsage< TexUsage::Default, PixelType >,
           public Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType >
    {
        public:

        Texture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu,
                    const bool generateMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}

        Texture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}


        Texture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}

        Texture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) :
            Texture2DGeneric( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, textureFormat, viewFormat1, viewFormat2, viewFormat3 )
        {}
    };
}
