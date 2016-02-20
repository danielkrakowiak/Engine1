#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

// #TODO:
// Add support for mipmaps. Generated on CPU or GPU or both.
// Add support for writing/reading mipmaps CPU -> GPU, GPU -> CPU.

// Rules:
// Padding in texture data is removed during copy. Data stored in the texture is a multiple of texture pixel type.
// Mipmaps are generated on the GPU if possible (binding: Render target + Shader resource)
// Mipmaps are generated on the CPU in other cases.
// Mipmaps are stored on GPU and CPU if they are enabled (and if texture is stored on CPU at all)

// Almost any texture can be created without initial data. And not initially loaded to GPU.
// Loading to GPU can always occur through deleting and recreating the texture and views.

namespace Engine1
{
    enum class TTexture2DUsage : int
    {
        Immutable = 0,
        Dynamic,
        Default,
        StagingRead,
        StagingWrite,
        StagingReadWrite
    };

    enum class TTexture2DBinding : int
    {
        ShaderResource = 0,
        RenderTarget,
        DepthStencil,
        UnorderedAccess,
        RenderTarget_ShaderResource,
        RenderTarget_UnorderedAccess,
        RenderTarget_UnorderedAccess_ShaderResource,
        DepthStencil_ShaderResource,
        DepthStencil_UnorderedAccess,
        DepthStencil_UnorderedAccess_ShaderResource,
        UnorderedAccess_ShaderResource
    };

    D3D11_USAGE getTextureUsage( TTexture2DUsage usage );
    UINT        getTextureCPUAccessFlags( TTexture2DUsage usage );
    UINT        getTextureBindFlags( TTexture2DBinding binding );
    D3D11_MAP   getMapForWriteFlag( TTexture2DUsage usage );
    D3D11_MAP   getMapForReadFlag( TTexture2DUsage usage );

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    class TTexture2DBase
    {
        public:

        void                     setFileInfo( const Texture2DFileInfo& fileInfo );
        const Texture2DFileInfo& getFileInfo() const;
        Texture2DFileInfo&       getFileInfo();

        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        int getMipMapCountOnCpu()  const;
        int getMipMapCountOnGpu() const;
        int getBytesPerPixel() const;
        int getWidth( unsigned int mipMapLevel = 0 ) const;
        int getHeight( unsigned int mipMapLevel = 0 ) const;
        int getSize( unsigned int mipMapLevel = 0 ) const;
        int getLineSize( unsigned int mipMapLevel = 0 ) const;

        protected:

        void initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps );

        void initialize( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format,
                            const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps );

        void initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                            const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps );

        void initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu );

        void initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                            const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps );

        void createTextureOnCpu( const int width, const int height );
        void createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height );
        // Re-interprets source data as the desired pixel type, removes padding at the end of each line and sets the data for the texture.
        void createTextureOnCpu( const char* data, const int width, const int height, const int lineSize );

        void createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps );
        void createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool generateMipmaps );

        void createTextureViewsOnGpu( ID3D11Device& device );

        bool supportsInitialLoadCpuToGpu();
        bool supportsLoadCpuToGpu();
        bool supportsLoadGpuToCpu();

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();

        Texture2DFileInfo fileInfo;

        int  width;
        int  height;
        bool hasMipmapsOnGpu;

        std::vector< std::vector< PixelType > >   dataMipmaps;
        Microsoft::WRL::ComPtr< ID3D11Texture2D > texture;

        Microsoft::WRL::ComPtr< ID3D11ShaderResourceView >  shaderResourceView;
        Microsoft::WRL::ComPtr< ID3D11RenderTargetView >    renderTargetView;
        Microsoft::WRL::ComPtr< ID3D11DepthStencilView >    depthStencilView;
        Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > unorderedAccessView;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
    {
        initialize( device, fileInfo.getPath(), fileInfo.getFormat(), storeOnCpu, storeOnGpu, generateMipmaps );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::initialize( ID3D11Device& device, const std::string& path, const Texture2DFileInfo::Format format,
                      const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
    {
        std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

        initialize( device, fileData->cbegin(), fileData->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );

        fileInfo.setPath( path );
        fileInfo.setFormat( format );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                      const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
    {
        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DBase::initialize - texture is set to be stored neither on CPU or GPU." );

        format; // Unused.

        // #TODO: FIT_BITMAP is not always the good image type (see "To do.txt").
        fipImage image( FIT_BITMAP );
        // #TODO: WARNING: casting away const qualifier on data - need to make sure that FreeImage doesn't modify the input data!
        const size_t dataSize = (dataEndIt - dataIt) / sizeof(char);
        fipMemoryIO data( const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&(*dataIt))), (DWORD)dataSize );

        // Parse image data.
        if ( !image.loadFromMemory( data ) )
            throw std::exception( "TTexture2DBase::initialize - parsing texture from file in memory failed." );

        // Convert image to 32 bpp if needed.
        if ( image.getBitsPerPixel() != 32 ) {
            if ( image.getBitsPerPixel() == 24 ) {
                if ( !image.convertTo32Bits() )
                    throw std::exception( "TTexture2DBase::initialize - loaded texture is 24 bits per pixel and it's conversion to 32 bits per pixel failed." );
            } else if ( image.getBitsPerPixel() != 8 ) {
                throw std::exception( "TTexture2DBase::initialize - loaded texture is neither 32, 24 or 8 bits per pixel. This byte per pixel fromat is not supported." );
            }
        }

        createTextureOnCpu( (char*)image.accessPixels(), image.getWidth(), image.getHeight(), image.getLine() );

        if ( storeOnGpu ) {
            if ( supportsInitialLoadCpuToGpu() )
                createTextureOnGpu( device, dataMipmaps.front(), width, height, generateMipmaps );
            else
                createTextureOnGpu( device, width, height, generateMipmaps );

            createTextureViewsOnGpu( device );
        }

        if ( !storeOnCpu )
            dataMipmaps.clear();

        this->width           = image.getWidth();
        this->height          = image.getHeight();
        this->hasMipmapsOnGpu = generateMipmaps;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
    {
        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DBase::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu )
            createTextureOnCpu( width, height );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, width, height, false );
            createTextureViewsOnGpu( device );
        }

        this->width           = width;
        this->height          = height;
        this->hasMipmapsOnGpu = false;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                      const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
    {
        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DBase::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu ) {
            createTextureOnCpu( data, width, height );

            // #TODO: generate mipmaps on CPU.
        }

        if ( storeOnGpu ) {
            createTextureOnGpu( device, data, width, height, generateMipmaps );
            createTextureViewsOnGpu( device );
        }

        this->width           = width;
        this->height          = height;
        this->hasMipmapsOnGpu = generateMipmaps;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::setFileInfo( const Texture2DFileInfo& fileInfo )
    {
        this->fileInfo = fileInfo;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    const Texture2DFileInfo& TTexture2DBase< usage, binding, PixelType, format >
        ::getFileInfo() const
    {
        return fileInfo;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    Texture2DFileInfo& TTexture2DBase< usage, binding, PixelType, format >
        ::getFileInfo()
    {
        return fileInfo;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureOnCpu( const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DBase::createTextureOnCpu - given width or height has zero or negative value." );

        dataMipmaps.clear();
        dataMipmaps.push_back( std::vector< PixelType >() );
        dataMipmaps.back().resize( width * height );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DBase::createTextureOnCpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "TTexture2DBase::createTextureOnCpu - given data size doesn't match given width and height." );

        dataMipmaps.clear();
        dataMipmaps.push_back( std::vector< PixelType >() );
        dataMipmaps.back().insert( dataMipmaps.back().begin(), data.cbegin(), data.cend() );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureOnCpu( const char* data, const int width, const int height, const int lineSize )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DBase::createTextureOnCpu - given width or height has zero or negative value." );

        dataMipmaps.clear();
        dataMipmaps.push_back( std::vector< PixelType >() );

        std::vector< PixelType >& dstData = dataMipmaps.back();
        dstData.resize( width * height );

        if ( (int)sizeof(PixelType) * width < lineSize ) {
            // There is some padding at the end of each line.
            for ( int y = 0; y < height; ++y ) {
                std::memcpy( dstData.data() + (width * y), data + (lineSize * y), sizeof(PixelType) * width );
            }
        } else if ( (int)sizeof(PixelType) * width == lineSize ) {
            // There is no padding.
            std::memcpy( dstData.data(), data, (int)sizeof(PixelType) * width * height );
        } else {
            throw std::exception( "TTexture2DBase::createTextureOnCpu - given line size is too small to store the pixels for the given width and pixel type." );
        }
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DBase::createTextureOnGpu - given width or height has zero or negative value." );

        const D3D11_USAGE textureUsage          = getTextureUsage( usage );
        const UINT        textureBindFlags      = getTextureBindFlags( binding );
        const UINT        textureCPUAccessFlags = getTextureCPUAccessFlags( usage );
        const UINT        mipmapCount           = generateMipmaps ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        mipmapCount; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1; //mipmapCount; #TODO: add support for mipmaps.
        desc.ArraySize          = 1; //mipmapCount;
        desc.Format             = format;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = textureUsage;
        desc.BindFlags          = textureBindFlags;
        desc.CPUAccessFlags     = textureCPUAccessFlags;
        desc.MiscFlags          = 0;

        HRESULT result = device.CreateTexture2D( &desc, nullptr, texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TTexture2DBase::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, const bool generateMipmaps )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DBase::createTextureOnGpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "TTexture2DBase::createTextureOnGpu - given data size doesn't match given width and height." );

        const D3D11_USAGE textureUsage          = getTextureUsage( usage );
        const UINT        textureBindFlags      = getTextureBindFlags( binding );
        const UINT        textureCPUAccessFlags = getTextureCPUAccessFlags( usage );
        const UINT        mipmapCount           = generateMipmaps ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        mipmapCount; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1; //mipmapCount; // #TODO: add support for mipmaps. Generated on CPU or GPU or both.
        desc.ArraySize          = 1; //mipmapCount;
        desc.Format             = format;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = textureUsage;
        desc.BindFlags          = textureBindFlags;
        desc.CPUAccessFlags     = textureCPUAccessFlags;
        desc.MiscFlags          = 0;

        D3D11_SUBRESOURCE_DATA dataDesc;
        ZeroMemory( &dataDesc, sizeof(dataDesc) );
        dataDesc.pSysMem          = data.data();
        dataDesc.SysMemPitch      = sizeof(PixelType) * width;
        dataDesc.SysMemSlicePitch = sizeof(PixelType) * width * height;

        HRESULT result = device.CreateTexture2D( &desc, &dataDesc, texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TTexture2DBase::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::createTextureViewsOnGpu( ID3D11Device& device )
    {
        UINT textureBindFlags = getTextureBindFlags( binding );

        if ( (textureBindFlags & D3D11_BIND_SHADER_RESOURCE) != 0 ) {
            HRESULT result = device.CreateShaderResourceView( texture.Get(), nullptr, shaderResourceView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );
        }

        if ( (textureBindFlags & D3D11_BIND_RENDER_TARGET) != 0 ) {
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format = format;
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateRenderTargetView( texture.Get(), &desc, renderTargetView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );
        }

        if ( (textureBindFlags & D3D11_BIND_DEPTH_STENCIL) != 0 ) {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format = format; //DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_D32_FLOAT; // Note: Could be also: DXGI_FORMAT_D24_UNORM_S8_UINT (to allow stencil sampling).
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateDepthStencilView( texture.Get(), &desc, depthStencilView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating depth stencil view on GPU failed." );
        }

        if ( (textureBindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0 ) {
            D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format = format;
            desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateUnorderedAccessView( texture.Get(), &desc, unorderedAccessView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating unordered access view on GPU failed." );
        }
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    bool TTexture2DBase< usage, binding, PixelType, format >
        ::supportsInitialLoadCpuToGpu()
    {
        #pragma warning(suppress: 4127)
        if ( usage == TTexture2DUsage::StagingRead )
            return false;
        else
            return true;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    bool TTexture2DBase< usage, binding, PixelType, format >
        ::supportsLoadCpuToGpu()
    {
        if ( usage == TTexture2DUsage::Dynamic || usage == TTexture2DUsage::StagingWrite || usage == TTexture2DUsage::StagingReadWrite )
            return true;
        else
            return false;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    bool TTexture2DBase< usage, binding, PixelType, format >
        ::supportsLoadGpuToCpu()
    {
        if ( usage == TTexture2DUsage::StagingRead || usage == TTexture2DUsage::StagingReadWrite )
            return true;
        else
            return false;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "TTexture2DBase::loadCpuToGpu - texture is not in CPU memory." );

        if ( !isInGpuMemory() ) {
            if ( supportsInitialLoadCpuToGpu() )
                createTextureOnGpu( device, dataMipmaps.front(), width, height, getMipMapCountOnCpu() > 1 );
            else
                throw std::exception( "TTexture2DBase::loadCpuToGpu - operation not available for this type (usage) of texture." );
        } else {
            if ( supportsLoadCpuToGpu() ) {
                D3D11_MAPPED_SUBRESOURCE mappedResource;

                HRESULT result = deviceContext.Map( texture.Get(), 0, getMapForWriteFlag( usage ), 0, &mappedResource );
                if ( result < 0 ) throw std::exception( "TTexture2DBase::loadCpuToGpu - mapping texture for write failed." );

                std::memcpy( mappedResource.pData, dataMipmaps, getSize() );
                mappedResource.RowPitch   = getLineSize();
                mappedResource.DepthPitch = getSize();

                deviceContext.Unmap( texture.Get(), 0 );
            } else if ( supportsInitialLoadCpuToGpu() ) {
                unloadFromGpu();
                createTextureOnGpu( device, dataMipmaps.front(), width, height, getMipMapCountOnCpu() > 1 );
            } else {
                throw std::exception( "TTexture2DBase::loadCpuToGpu - operation not available for this type (usage) of texture." );
            }
        }
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::loadGpuToCpu()
    {
        if ( !isInGpuMemory() )
            throw std::exception( "TTexture2DBase::loadGpuToCpu - texture is not in GPU memory." );

        if ( !supportsLoadGpuToCpu() )
            throw std::exception( "TTexture2DBase::loadGpuToCpu - operation not available for this type (usage) of texture." );
        
        if ( !isInCpuMemory() )
            createTextureOnCpu( width, height );

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = deviceContext.Map( texture.Get(), 0, getMapForReadFlag( usage ), 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "TTexture2DBase::loadGpuToCpu - mapping texture for read failed." );

        std::memcpy( dataMipmaps.front().data(), mappedResource.pData, getSize() );
        mappedResource.RowPitch   = getLineSize();
        mappedResource.DepthPitch = getSize();

        deviceContext.Unmap( texture.Get(), 0 );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::unloadFromCpu()
    {
        dataMipmaps.clear();
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    void TTexture2DBase< usage, binding, PixelType, format >
        ::unloadFromGpu()
    {
	    texture.Reset();

        shaderResourceView.Reset();
        renderTargetView.Reset();
        depthStencilView.Reset();
        unorderedAccessView.Reset();

	    mipmapsOnGpu = false;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    bool TTexture2DBase< usage, binding, PixelType, format >
        ::isInCpuMemory() const
    {
	    return !dataMipmaps.empty() && !dataMipmaps.front().empty();
    }
    
    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    bool TTexture2DBase< usage, binding, PixelType, format >
        ::isInGpuMemory() const
    {
	    return texture != nullptr;
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getMipMapCountOnCpu()  const
    {
        return (int)dataMipmaps.size();
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getMipMapCountOnGpu() const
    {
         if ( isInGpuMemory() ) {
	        if ( hasMipmapsOnGpu )
		        return 1 + (int)( floor( log2( std::max( width, height ) ) ) );
	        else
		        return 1;
        } else {
	        return 0;
        }
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getBytesPerPixel() const
    {
        return sizeof( PixelType );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getWidth( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DBase::getWidth - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, width / ( 1 + (int)mipMapLevel ) );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getHeight( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DBase::getHeight - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, height / ( 1 + (int)mipMapLevel ) );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DBase::getSize - Incorrect level requested. There is no mipmap with such level." );

	    return getLineSize( mipMapLevel ) * getHeight( mipMapLevel );
    }

    template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
    int TTexture2DBase< usage, binding, PixelType, format >
        ::getLineSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DBase::getLineSize - Incorrect level requested. There is no mipmap with such level." );

	    return getWidth( mipMapLevel ) * getBytesPerPixel();
    }
}

