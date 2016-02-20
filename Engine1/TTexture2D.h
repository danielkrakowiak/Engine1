#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "TTexture2DBase2.h"

// - Can't use static createFromFile method, because it has to return TTexture2D class object - so it has to be in TTexture2D class (to know TTexture2D type). 
//   But TTexture2D class is specialized, so that requires having multiple versions of the same method. It would need to use base class method and cast the result maybe. It's simpler to use constructors.

// CreateFromFile can create the appropriate texture type and load content from files - no need to change that when initial data is required.
// It looks like Immutable texture can still be modified through uploading data to it with DISCARD_OVERWRITE flag or something like that.

// How to deal with padding at the end of each data row? - remove padding when copying data. Only when loading from file (inside the class). Otherwise require unpadded data.

// What with generating and storing mipmaps on CPU and GPU?

namespace Engine1
{
    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    class TTexture2D 
        : public TTexture2DBase2< usage, binding, PixelType, format >
    {};

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::Immutable, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::Immutable, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, path, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
        {
            TTexture2DBase::loadCpuToGpu( device, deviceContext );
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::Dynamic, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::Dynamic, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, path, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool generateMipmaps )
        {
            initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, width, height, storeOnCpu, storeOnGpu );
        }
        
        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
        {
            TTexture2DBase::loadCpuToGpu( device, deviceContext );
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::Default, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::Default, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, path, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, width, height, storeOnCpu, storeOnGpu );
        }

        TTexture2D( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            initialize( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
        {
            TTexture2DBase::loadCpuToGpu( device, deviceContext );
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::StagingRead, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::StagingRead, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, width, height, storeOnCpu, storeOnGpu );
        }

        void loadGpuToCpu()
        {
            TTexture2DBase::loadGpuToCpu();
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::StagingWrite, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::StagingWrite, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, fileInfo, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, path, format, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, width, height, storeOnCpu, storeOnGpu );
        }

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
        {
            TTexture2DBase::loadCpuToGpu( device, deviceContext );
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };

    template< TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format>
    class TTexture2D< TTexture2DUsage::StagingReadWrite, binding, PixelType, format >
        : public TTexture2DBase2< TTexture2DUsage::StagingReadWrite, binding, PixelType, format >
    {
        public:

        TTexture2D( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, fileInfo, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, path, format, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, false );
        }

        TTexture2D( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            initialize( device, width, height, storeOnCpu, storeOnGpu );
        }

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
        {
            TTexture2DBase::loadCpuToGpu( device, deviceContext );
        }

        void loadGpuToCpu()
        {
            TTexture2DBase::loadGpuToCpu();
        }

        void unloadFromCpu()
        {
            TTexture2DBase::unloadFromCpu();
        }

        void unloadFromGpu()
        {
            TTexture2DBase::unloadFromCpu()
        }
    };
}



