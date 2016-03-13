#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <d3d11.h>
#include <wrl.h>

#include "ImageLibrary.h"
#include "TTexture2DBase.h"
#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

// #TODO: Add support for mipmaps. Generated on CPU or GPU or both.
// #TODO: Add support for writing/reading mipmaps CPU -> GPU, GPU -> CPU.
// #TODO: PixelType should be compared with textureFormat passed in constructor. If they are different, wrong amounts of memory will be allocated on CPU vs GPU and future copying will fail.

// Rules:
// Padding in texture data is removed during copy. Data stored in the texture is a multiple of texture pixel type.
// Mipmaps are generated on the GPU if possible (binding: Render target + Shader resource)
// Mipmaps are generated on the CPU in other cases.
// Mipmaps are stored on GPU and CPU if they are enabled (and if texture is stored on CPU at all)

// Almost any texture can be created without initial data. And not initially loaded to GPU.
// Loading to GPU can always occur through deleting and recreating the texture and views.

namespace Engine1
{
    enum class TexUsage : int
    {
        Immutable = 0,
        Dynamic,
        Default,
        StagingRead,
        StagingWrite,
        StagingReadWrite
    };

    enum class TexBind : int
    {
        None = 0,
        ShaderResource,
        RenderTarget,
        DepthStencil,
        UnorderedAccess,
        RenderTarget_ShaderResource,
        RenderTarget_UnorderedAccess,
        RenderTarget_UnorderedAccess_ShaderResource,
        DepthStencil_ShaderResource,
        UnorderedAccess_ShaderResource
    };

    

    template< TexUsage usage, TexBind binding, typename PixelType >
    class TTexture2DInternal : public TTexture2DBase
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

        TTexture2DInternal() { checkIfSupported(); };
        ~TTexture2DInternal() {};

        void reset();

        void initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, 
                         const bool generateMipmaps, DXGI_FORMAT textureFormat, 
                         DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                         const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                         DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                         DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                         const bool storeOnCpu, const bool storeOnGpu,const bool generateMipmaps, DXGI_FORMAT textureFormat,
                         DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void createTextureOnCpu( const int width, const int height );
        void createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height );
        // Re-interprets source data as the desired pixel type, removes padding at the end of each line and sets the data for the texture.
        void createTextureOnCpu( const char* data, const int width, const int height, const int lineSize );

        void createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                                 const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureViewsOnGpu( ID3D11Device& device, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                      DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat );

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void loadGpuToCpu( ID3D11DeviceContext& deviceContext );
        void unloadFromCpu();
        void unloadFromGpu();

        void checkIfSupported();
        bool supportsInitialLoadCpuToGpu();
        bool supportsLoadCpuToGpu();
        bool supportsLoadGpuToCpu();

        DXGI_FORMAT readShaderResourceViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readRenderTargetViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readDepthStencilViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readUnorderedAccessViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        D3D11_USAGE getTextureUsageFlag();
        UINT        getTextureCPUAccessFlags();
        UINT        getTextureBindFlags();
        D3D11_MAP   getMapForWriteFlag();
        D3D11_MAP   getMapForReadFlag();

        Texture2DFileInfo fileInfo;

        int  width;
        int  height;
        bool hasMipmapsOnGpu;

        std::vector< std::vector< PixelType > >   dataMipmaps;
        Microsoft::WRL::ComPtr< ID3D11Texture2D > texture;
        
        DXGI_FORMAT textureFormat;
        DXGI_FORMAT shaderResourceViewFormat;
        DXGI_FORMAT renderTargetViewFormat;
        DXGI_FORMAT depthStencilViewFormat;
        DXGI_FORMAT unorderedAccessViewFormat;

        Microsoft::WRL::ComPtr< ID3D11ShaderResourceView >  shaderResourceView;
        Microsoft::WRL::ComPtr< ID3D11RenderTargetView >    renderTargetView;
        Microsoft::WRL::ComPtr< ID3D11DepthStencilView >    depthStencilView;
        Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > unorderedAccessView;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::reset()
    {
        dataMipmaps.clear();

        textureFormat             = DXGI_FORMAT_UNKNOWN;
        shaderResourceViewFormat  = DXGI_FORMAT_UNKNOWN;
        renderTargetViewFormat    = DXGI_FORMAT_UNKNOWN;
        depthStencilViewFormat    = DXGI_FORMAT_UNKNOWN;
        unorderedAccessViewFormat = DXGI_FORMAT_UNKNOWN;

        width           = 0;
        height          = 0;
        hasMipmapsOnGpu = false;

        texture.Reset();
        shaderResourceView.Reset();
        renderTargetView.Reset();
        depthStencilView.Reset();
        unorderedAccessView.Reset();
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( fileInfo.getPath() );

        initialize( device, fileData->cbegin(), fileData->cend(), fileInfo.getFormat(), storeOnCpu, storeOnGpu, generateMipmaps,
                    textureFormat, viewFormat1, viewFormat2, viewFormat3 );

        this->fileInfo = fileInfo;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                      const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        reset();

        this->textureFormat       = textureFormat;
        shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DInternal::initialize - texture is set to be stored neither on CPU or GPU." );

        format; // Unused.

        // #TODO: FIT_BITMAP is not always the good image type (see "To do.txt").
        fipImage image( FIT_BITMAP );
        // #TODO: WARNING: casting away const qualifier on data - need to make sure that FreeImage doesn't modify the input data!
        const size_t dataSize = (dataEndIt - dataIt) / sizeof(char);
        fipMemoryIO data( const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&(*dataIt))), (DWORD)dataSize );

        // Parse image data.
        if ( !image.loadFromMemory( data ) )
            throw std::exception( "TTexture2DInternal::initialize - parsing texture from file in memory failed." );

        // Convert image to 32 bpp if needed.
        if ( image.getBitsPerPixel() != 32 ) {
            if ( image.getBitsPerPixel() == 24 ) {
                if ( !image.convertTo32Bits() )
                    throw std::exception( "TTexture2DInternal::initialize - loaded texture is 24 bits per pixel and it's conversion to 32 bits per pixel failed." );
            } else if ( image.getBitsPerPixel() != 8 ) {
                throw std::exception( "TTexture2DInternal::initialize - loaded texture is neither 32, 24 or 8 bits per pixel. This byte per pixel fromat is not supported." );
            }
        }

        createTextureOnCpu( (char*)image.accessPixels(), image.getWidth(), image.getHeight(), image.getLine() );

        if ( storeOnGpu ) {
            if ( supportsInitialLoadCpuToGpu() )
                createTextureOnGpu( device, dataMipmaps.front(), image.getWidth(), image.getHeight(), generateMipmaps, textureFormat );
            else
                createTextureOnGpu( device, image.getWidth(), image.getHeight(), generateMipmaps, textureFormat );

            createTextureViewsOnGpu( device, shaderResourceViewFormat, renderTargetViewFormat, depthStencilViewFormat, unorderedAccessViewFormat );
        }

        if ( !storeOnCpu )
            dataMipmaps.clear();

        this->width           = image.getWidth();
        this->height          = image.getHeight();
        this->hasMipmapsOnGpu = generateMipmaps;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        reset();

        this->textureFormat       = textureFormat;
        shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DInternal::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu )
            createTextureOnCpu( width, height );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, width, height, false, textureFormat );
            createTextureViewsOnGpu( device, this->shaderResourceViewFormat, this->renderTargetViewFormat, this->depthStencilViewFormat, this->unorderedAccessViewFormat );
        }

        this->width           = width;
        this->height          = height;
        this->hasMipmapsOnGpu = false;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                      const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        reset();

        this->textureFormat       = textureFormat;
        shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "TTexture2DInternal::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu ) {
            createTextureOnCpu( data, width, height );

            // #TODO: generate mipmaps on CPU.
        }

        if ( storeOnGpu ) {
            createTextureOnGpu( device, data, width, height, generateMipmaps, textureFormat );
            createTextureViewsOnGpu( device, shaderResourceViewFormat, renderTargetViewFormat, depthStencilViewFormat, unorderedAccessViewFormat );
        }

        this->width           = width;
        this->height          = height;
        this->hasMipmapsOnGpu = generateMipmaps;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::setFileInfo( const Texture2DFileInfo& fileInfo )
    {
        this->fileInfo = fileInfo;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    const Texture2DFileInfo& TTexture2DInternal< usage, binding, PixelType >
        ::getFileInfo() const
    {
        return fileInfo;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    Texture2DFileInfo& TTexture2DInternal< usage, binding, PixelType >
        ::getFileInfo()
    {
        return fileInfo;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureOnCpu( const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DInternal::createTextureOnCpu - given width or height has zero or negative value." );

        dataMipmaps.clear();
        dataMipmaps.push_back( std::vector< PixelType >() );
        dataMipmaps.back().resize( width * height );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DInternal::createTextureOnCpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "TTexture2DInternal::createTextureOnCpu - given data size doesn't match given width and height." );

        dataMipmaps.clear();
        dataMipmaps.push_back( std::vector< PixelType >() );
        dataMipmaps.back().insert( dataMipmaps.back().begin(), data.cbegin(), data.cend() );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureOnCpu( const char* data, const int width, const int height, const int lineSize )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DInternal::createTextureOnCpu - given width or height has zero or negative value." );

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
            throw std::exception( "TTexture2DInternal::createTextureOnCpu - given line size is too small to store the pixels for the given width and pixel type." );
        }
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DInternal::createTextureOnGpu - given width or height has zero or negative value." );

        const UINT mipmapCount = generateMipmaps ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        mipmapCount; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1; //mipmapCount; #TODO: add support for mipmaps.
        desc.ArraySize          = 1; //mipmapCount;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = 0;

        HRESULT result = device.CreateTexture2D( &desc, nullptr, texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TTexture2DInternal::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                              const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "TTexture2DInternal::createTextureOnGpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "TTexture2DInternal::createTextureOnGpu - given data size doesn't match given width and height." );

        const UINT mipmapCount = generateMipmaps ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        mipmapCount; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1; //mipmapCount; // #TODO: add support for mipmaps. Generated on CPU or GPU or both.
        desc.ArraySize          = 1; //mipmapCount;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = 0;

        D3D11_SUBRESOURCE_DATA dataDesc;
        ZeroMemory( &dataDesc, sizeof(dataDesc) );
        dataDesc.pSysMem          = data.data();
        dataDesc.SysMemPitch      = sizeof(PixelType) * width;
        dataDesc.SysMemSlicePitch = sizeof(PixelType) * width * height;

        HRESULT result = device.CreateTexture2D( &desc, &dataDesc, texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TTexture2DInternal::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::createTextureViewsOnGpu( ID3D11Device& device, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                   DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat )
    {
        const UINT textureBindFlags = getTextureBindFlags();

        if ( (textureBindFlags & D3D11_BIND_SHADER_RESOURCE) != 0 ) {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	        ZeroMemory( &desc, sizeof( desc ) );
	        desc.Format                    = shaderResourceViewFormat;
	        desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	        desc.Texture2D.MostDetailedMip = 0;
	        desc.Texture2D.MipLevels       = 1; // #TODO: add support for mipmaps.

            HRESULT result = device.CreateShaderResourceView( texture.Get(), &desc, shaderResourceView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );

            this->shaderResourceViewFormat = shaderResourceViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_RENDER_TARGET) != 0 ) {
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format             = renderTargetViewFormat;
            desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateRenderTargetView( texture.Get(), &desc, renderTargetView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );

            this->renderTargetViewFormat = renderTargetViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_DEPTH_STENCIL) != 0 ) {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format             = depthStencilViewFormat;
            desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateDepthStencilView( texture.Get(), &desc, depthStencilView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating depth stencil view on GPU failed." );

            this->depthStencilViewFormat = depthStencilViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0 ) {
            D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Format             = unorderedAccessViewFormat;
            desc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            HRESULT result = device.CreateUnorderedAccessView( texture.Get(), &desc, unorderedAccessView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating unordered access view on GPU failed." );

            this->unorderedAccessViewFormat = unorderedAccessViewFormat;
        }
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::checkIfSupported()
    {
        static_assert( 
            ( usage == TexUsage::Immutable && binding == TexBind::ShaderResource ) ||
            ( usage == TexUsage::Dynamic && binding == TexBind::ShaderResource ) ||
            ( usage == TexUsage::Default && binding != TexBind::None ) ||
            ( 
                ( 
                    usage == TexUsage::StagingRead ||
                    usage == TexUsage::StagingReadWrite ||
                    usage == TexUsage::StagingWrite 
                ) && 
                binding == TexBind::None 
            ), 
            "Unsupported texture configuration." 
        );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    bool TTexture2DInternal< usage, binding, PixelType >
        ::supportsInitialLoadCpuToGpu()
    {
        #pragma warning(suppress: 4127)
        if ( usage == TexUsage::StagingRead )
            return false;
        else
            return true;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    bool TTexture2DInternal< usage, binding, PixelType >
        ::supportsLoadCpuToGpu()
    {
        if ( usage == TexUsage::Dynamic || usage == TexUsage::StagingWrite || usage == TexUsage::StagingReadWrite )
            return true;
        else
            return false;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    bool TTexture2DInternal< usage, binding, PixelType >
        ::supportsLoadGpuToCpu()
    {
        if ( usage == TexUsage::StagingRead || usage == TexUsage::StagingReadWrite )
            return true;
        else
            return false;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    DXGI_FORMAT  TTexture2DInternal< usage, binding, PixelType >
        ::readShaderResourceViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) 
    {
        #pragma warning(suppress: 4127)
        if ( binding == TexBind::ShaderResource )
            return viewFormat1;
        else if ( binding == TexBind::DepthStencil_ShaderResource || binding == TexBind::RenderTarget_ShaderResource || binding == TexBind::UnorderedAccess_ShaderResource ) 
            return viewFormat2;
        else if ( binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
            return viewFormat3;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    DXGI_FORMAT  TTexture2DInternal< usage, binding, PixelType >
        ::readRenderTargetViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat2; viewFormat3; // Unused.

        #pragma warning(suppress: 4127)
        if ( binding == TexBind::RenderTarget || binding == TexBind::RenderTarget_ShaderResource || 
             binding == TexBind::RenderTarget_UnorderedAccess || binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource )

             return viewFormat1;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    DXGI_FORMAT  TTexture2DInternal< usage, binding, PixelType >
        ::readDepthStencilViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat2; viewFormat3; // Unused.

        #pragma warning(suppress: 4127)
        if ( binding == TexBind::DepthStencil || binding == TexBind::DepthStencil_ShaderResource )

             return viewFormat1;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    DXGI_FORMAT  TTexture2DInternal< usage, binding, PixelType >
        ::readUnorderedAccessViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat3; // Unused.

        #pragma warning(suppress: 4127)
        if ( binding == TexBind::UnorderedAccess || binding == TexBind::UnorderedAccess_ShaderResource ) 
            return viewFormat1;
        else if ( binding == TexBind::RenderTarget_UnorderedAccess || binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 

            return viewFormat2;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "TTexture2DInternal::loadCpuToGpu - texture is not in CPU memory." );

        if ( !isInGpuMemory() ) {
            if ( supportsInitialLoadCpuToGpu() ) {
                createTextureOnGpu( device, dataMipmaps.front(), width, height, getMipMapCountOnCpu() > 1, textureFormat );
                createTextureViewsOnGpu( device, this->shaderResourceViewFormat, this->renderTargetViewFormat, this->depthStencilViewFormat, this->unorderedAccessViewFormat );
            } else {
                throw std::exception( "TTexture2DInternal::loadCpuToGpu - operation not available for this type (usage) of texture." );
            }
        } else {
            if ( supportsLoadCpuToGpu() ) {
                D3D11_MAPPED_SUBRESOURCE mappedResource;

                HRESULT result = deviceContext.Map( texture.Get(), 0, getMapForWriteFlag(), 0, &mappedResource );
                if ( result < 0 ) throw std::exception( "TTexture2DInternal::loadCpuToGpu - mapping texture for write failed." );

                std::memcpy( mappedResource.pData, dataMipmaps.front().data(), getSize() );
                mappedResource.RowPitch   = getLineSize();
                mappedResource.DepthPitch = getSize();

                deviceContext.Unmap( texture.Get(), 0 );
            } else if ( supportsInitialLoadCpuToGpu() ) {
                unloadFromGpu();
                createTextureOnGpu( device, dataMipmaps.front(), width, height, getMipMapCountOnCpu() > 1, textureFormat );
                createTextureViewsOnGpu( device, this->shaderResourceViewFormat, this->renderTargetViewFormat, this->depthStencilViewFormat, this->unorderedAccessViewFormat );
            } else {
                throw std::exception( "TTexture2DInternal::loadCpuToGpu - operation not available for this type (usage) of texture." );
            }
        }
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::loadGpuToCpu( ID3D11DeviceContext& deviceContext )
    {
        if ( !isInGpuMemory() )
            throw std::exception( "TTexture2DInternal::loadGpuToCpu - texture is not in GPU memory." );

        if ( !supportsLoadGpuToCpu() )
            throw std::exception( "TTexture2DInternal::loadGpuToCpu - operation not available for this type (usage) of texture." );
        
        if ( !isInCpuMemory() )
            createTextureOnCpu( width, height );

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = deviceContext.Map( texture.Get(), 0, getMapForReadFlag(), 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "TTexture2DInternal::loadGpuToCpu - mapping texture for read failed." );

        std::memcpy( dataMipmaps.front().data(), mappedResource.pData, getSize() );
        mappedResource.RowPitch   = getLineSize();
        mappedResource.DepthPitch = getSize();

        deviceContext.Unmap( texture.Get(), 0 );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::unloadFromCpu()
    {
        dataMipmaps.clear();
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    void TTexture2DInternal< usage, binding, PixelType >
        ::unloadFromGpu()
    {
	    texture.Reset();

        shaderResourceView.Reset();
        renderTargetView.Reset();
        depthStencilView.Reset();
        unorderedAccessView.Reset();

	    hasMipmapsOnGpu = false;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    bool TTexture2DInternal< usage, binding, PixelType >
        ::isInCpuMemory() const
    {
	    return !dataMipmaps.empty() && !dataMipmaps.front().empty();
    }
    
    template< TexUsage usage, TexBind binding, typename PixelType >
    bool TTexture2DInternal< usage, binding, PixelType >
        ::isInGpuMemory() const
    {
	    return texture != nullptr;
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getMipMapCountOnCpu()  const
    {
        return (int)dataMipmaps.size();
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
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

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getBytesPerPixel() const
    {
        return sizeof( PixelType );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getWidth( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DInternal::getWidth - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, width / ( 1 + (int)mipMapLevel ) );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getHeight( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DInternal::getHeight - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, height / ( 1 + (int)mipMapLevel ) );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DInternal::getSize - Incorrect level requested. There is no mipmap with such level." );

	    return getLineSize( mipMapLevel ) * getHeight( mipMapLevel );
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    int TTexture2DInternal< usage, binding, PixelType >
        ::getLineSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "TTexture2DInternal::getLineSize - Incorrect level requested. There is no mipmap with such level." );

	    return getWidth( mipMapLevel ) * getBytesPerPixel();
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    D3D11_USAGE TTexture2DInternal< usage, binding, PixelType >
        ::getTextureUsageFlag()
    {
        switch ( usage ) {
            case TexUsage::Immutable:
                return D3D11_USAGE_IMMUTABLE;
            case TexUsage::Dynamic:
                return D3D11_USAGE_DYNAMIC;
            case TexUsage::Default:
                return D3D11_USAGE_DEFAULT;
            case TexUsage::StagingRead:
            case TexUsage::StagingWrite:
            case TexUsage::StagingReadWrite:
                return D3D11_USAGE_STAGING;
        }

        assert( false );
        return D3D11_USAGE_DEFAULT; // To avoid warning.
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    UINT TTexture2DInternal< usage, binding, PixelType >
        ::getTextureCPUAccessFlags()
    {
        switch ( usage ) {
            case TexUsage::Immutable:
                return 0;
            case TexUsage::Dynamic:
                return D3D11_CPU_ACCESS_WRITE;
            case TexUsage::Default:
                return 0;
            case TexUsage::StagingRead:
                return D3D11_CPU_ACCESS_READ;
            case TexUsage::StagingWrite:
                return D3D11_CPU_ACCESS_WRITE;
            case TexUsage::StagingReadWrite:
                return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        }

        assert( false );
        return 0; // To avoid warning.
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    UINT TTexture2DInternal< usage, binding, PixelType >
        ::getTextureBindFlags()
    {
        switch ( binding ) {
            case TexBind::None:
                return 0;
            case TexBind::ShaderResource:
                return D3D11_BIND_SHADER_RESOURCE;
            case TexBind::RenderTarget:
                return D3D11_BIND_RENDER_TARGET;
            case TexBind::DepthStencil:
                return D3D11_BIND_DEPTH_STENCIL;
            case TexBind::UnorderedAccess:
                return D3D11_BIND_UNORDERED_ACCESS;
            case TexBind::RenderTarget_ShaderResource:
                return D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            case TexBind::RenderTarget_UnorderedAccess:
                return D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
            case TexBind::RenderTarget_UnorderedAccess_ShaderResource:
                return D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
            case TexBind::DepthStencil_ShaderResource:
                return D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            case TexBind::UnorderedAccess_ShaderResource:
                return D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        }

        assert( false );
        return 0; // To avoid warning.
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    D3D11_MAP TTexture2DInternal< usage, binding, PixelType >
        ::getMapForWriteFlag()
    {
        switch ( usage ) {
            case TexUsage::Immutable:
            case TexUsage::Default:
            case TexUsage::StagingRead:
                throw std::exception( "Engine1::getMapForWriteFlag - given usage doesn't support writing." );
            case TexUsage::Dynamic:
                return D3D11_MAP_WRITE_DISCARD;
            case TexUsage::StagingWrite:
                return D3D11_MAP_WRITE;
            case TexUsage::StagingReadWrite:
                return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
        }

        assert( false );
        return D3D11_MAP_READ; // To avoid warning.
    }

    template< TexUsage usage, TexBind binding, typename PixelType >
    D3D11_MAP TTexture2DInternal< usage, binding, PixelType >
        ::getMapForReadFlag()
    {
        switch ( usage ) {
            case TexUsage::Immutable:
            case TexUsage::Default:
            case TexUsage::Dynamic:
            case TexUsage::StagingWrite:
                throw std::exception( "Engine1::getMapForReadFlag - given usage doesn't support reading." );
            case TexUsage::StagingRead:
                return D3D11_MAP_READ;
            case TexUsage::StagingReadWrite:
                return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
        }

        assert( false );
        return D3D11_MAP_READ; // To avoid warning.
    }
}

