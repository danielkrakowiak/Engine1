#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "TTexture2DBase2.h"

namespace Engine1
{
    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType >
    class TTexture2D 
        : public TTexture2DBase2< usage, binding, PixelType >
    {
        public:

        //TTexture2D( ID3D11Device& device, int width, int height, const bool storeOnCpu, const bool storeOnGpu )
        //{
        //    initialize( device, width, height, storeOnCpu, storeOnGpu );
        //}

        
    };

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::Immutable, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::Immutable, binding, PixelType >
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

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::Dynamic, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::Dynamic, binding, PixelType >
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

        ////////////////////////////////////////

        template< TTexture2DUsage _usage = usage, TTexture2DBinding _binding = binding, typename std::enable_if<_usage == TTexture2DUsage::Dynamic>::type* empty = 0 >
        TTexture2D( ID3D11Device& device )
        {
            //initialize( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
        }

        //template< TTexture2DBinding _binding = binding >
        ////typename std::enable_if<_binding == TTexture2DBinding::DepthStencil, int>::type
        //void tralala(typename std::enable_if<_binding == TTexture2DBinding::DepthStencil>::type* empty = 0)
        //{
        //    int g = 5;
        //}
    };

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::Default, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::Default, binding, PixelType >
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

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::StagingRead, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::StagingRead, binding, PixelType >
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

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::StagingWrite, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::StagingWrite, binding, PixelType >
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

    template< TTexture2DBinding binding, typename PixelType>
    class TTexture2D< TTexture2DUsage::StagingReadWrite, binding, PixelType >
        : public TTexture2DBase2< TTexture2DUsage::StagingReadWrite, binding, PixelType >
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



