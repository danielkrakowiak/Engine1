#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "TTexture2DSpecialized.h"

// #TODO:
// Constructor from raw data could take iterators rather than the whole vector.

namespace Engine1
{
    template< TexUsage usage, TexBind binding, typename PixelType >
    class TTexture2D 
        : public TTexture2DSpecialized< usage, PixelType >
    {
        public:

        // Constructor 1 - without initial data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu, DXGI_FORMAT textureFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::StagingRead ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::None ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, width, height, storeOnCpu, storeOnGpu, textureFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 1 - without initial data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingRead ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::ShaderResource ||
                          _b == TexBind::RenderTarget ||
                          _b == TexBind::DepthStencil ||
                          _b == TexBind::UnorderedAccess ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, width, height, storeOnCpu, storeOnGpu, textureFormat, viewFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 1 - without initial data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2,
                    typename std::enable_if< 
                        ( _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingRead ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::DepthStencil_ShaderResource ||
                          _b == TexBind::RenderTarget_ShaderResource ||
                          _b == TexBind::RenderTarget_UnorderedAccess ||
                          _b == TexBind::UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, width, height, storeOnCpu, storeOnGpu, textureFormat, viewFormat1, viewFormat2, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 1 - without initial data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3,
                    typename std::enable_if< 
                        ( _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingRead ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, width, height, storeOnCpu, storeOnGpu, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
        }

        // Constructor 2 - from file.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::None ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 2 - from file.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::ShaderResource ||
                          _b == TexBind::RenderTarget ||
                          _b == TexBind::DepthStencil ||
                          _b == TexBind::UnorderedAccess ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 2 - from file.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::DepthStencil_ShaderResource ||
                          _b == TexBind::RenderTarget_ShaderResource ||
                          _b == TexBind::RenderTarget_UnorderedAccess ||
                          _b == TexBind::UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 2 - from file.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
        }

        // Constructor 3 - from file in memory.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, 
                    DXGI_FORMAT textureFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::None ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 3 - from file in memory.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, 
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::ShaderResource ||
                          _b == TexBind::RenderTarget ||
                          _b == TexBind::DepthStencil ||
                          _b == TexBind::UnorderedAccess ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 3 - from file in memory.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, 
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::DepthStencil_ShaderResource ||
                          _b == TexBind::RenderTarget_ShaderResource ||
                          _b == TexBind::RenderTarget_UnorderedAccess ||
                          _b == TexBind::UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 3 - from file in memory.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, 
                    DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
        }

        // Constructor 4 - from raw data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu,
                    const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat, 
                    typename std::enable_if< 
                        ( _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::None ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 4 - from raw data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu,
                    const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::ShaderResource ||
                          _b == TexBind::RenderTarget ||
                          _b == TexBind::DepthStencil ||
                          _b == TexBind::UnorderedAccess ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 4 - from raw data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu,
                    const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::DepthStencil_ShaderResource ||
                          _b == TexBind::RenderTarget_ShaderResource ||
                          _b == TexBind::RenderTarget_UnorderedAccess ||
                          _b == TexBind::UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, DXGI_FORMAT_UNKNOWN );
        }

        // Constructor 4 - from raw data.
        template< TexUsage _u = usage, TexBind _b = binding >
        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu,
                    const bool storeOnGpu, const bool generateMipmaps, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3,
                    typename std::enable_if< 
                        ( _u == TexUsage::Immutable ||
                          _u == TexUsage::Dynamic ||
                          _u == TexUsage::Default ||
                          _u == TexUsage::StagingReadWrite ||
                          _u == TexUsage::StagingWrite ) 
                         &&
                        ( _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
                    >::type* = 0 )
        {
            TTexture2DGeneric::initialize( usage, binding, device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
        }

        template< TexUsage _u = usage, TexBind _b = binding >
        ID3D11ShaderResourceView* getShaderResourceView
            ( typename std::enable_if<
                ( _u == TexUsage::Immutable ||
                  _u == TexUsage::Dynamic ||
                  _u == TexUsage::Default )
                  &&
                ( _b == TexBind::ShaderResource || 
                  _b == TexBind::DepthStencil_ShaderResource ||
                  _b == TexBind::RenderTarget_ShaderResource ||
                  _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ||
                  _b == TexBind::UnorderedAccess_ShaderResource )
            >::type* = 0 )
        {
            return m_shaderResourceView.Get();
        }

        template< TexUsage _u = usage, TexBind _b = binding >
        ID3D11RenderTargetView* getRenderTargetView
            ( typename std::enable_if< 
                ( _u == TexUsage::Immutable ||
                  _u == TexUsage::Dynamic ||
                  _u == TexUsage::Default )
                  &&
                ( _b == TexBind::RenderTarget || 
                  _b == TexBind::RenderTarget_ShaderResource ||
                  _b == TexBind::RenderTarget_UnorderedAccess ||
                  _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource )
            >::type* = 0 )
        {
            return m_renderTargetView.Get();
        }

            
        template< TexUsage _u = usage, TexBind _b = binding >
        ID3D11DepthStencilView* getDepthStencilView
            ( typename std::enable_if< 
                ( _u == TexUsage::Immutable ||
                  _u == TexUsage::Dynamic ||
                  _u == TexUsage::Default )
                  &&
                ( _b == TexBind::DepthStencil ||
                  _b == TexBind::DepthStencil_ShaderResource )
             >::type* = 0 )
        {
            return m_depthStencilView.Get();
        }

        template< TexUsage _u = usage, TexBind _b = binding >
        ID3D11UnorderedAccessView* getUnorderedAccessView
            ( typename std::enable_if< 
                ( _u == TexUsage::Immutable ||
                  _u == TexUsage::Dynamic ||
                  _u == TexUsage::Default )
                    &&
                ( _b == TexBind::UnorderedAccess ||
                  _b == TexBind::RenderTarget_UnorderedAccess ||
                  _b == TexBind::RenderTarget_UnorderedAccess_ShaderResource ||
                  _b == TexBind::UnorderedAccess ||
                  _b == TexBind::UnorderedAccess_ShaderResource )
             >::type* = 0 )
        {
            return m_unorderedAccessView.Get();
        }
    };
}



