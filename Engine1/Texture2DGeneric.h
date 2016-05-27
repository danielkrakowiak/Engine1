#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DEnums.h"
#include "ImageLibrary.h"
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
    template< typename PixelType >
    class Texture2DGeneric : public Asset
    {
        public:

        virtual TexUsage getUsage() const;
        virtual TexBind  getBinding() const;

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                     setFileInfo( const Texture2DFileInfo& fileInfo );
        const Texture2DFileInfo& getFileInfo() const;
        Texture2DFileInfo&       getFileInfo();

        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        Microsoft::WRL::ComPtr< ID3D11Texture2D > getTextureResource();

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void unloadFromCpu();
        void unloadFromGpu();

        int getMipMapCountOnCpu()  const;
        int getMipMapCountOnGpu() const;
        int getBytesPerPixel() const;
        int getWidth( unsigned int mipMapLevel = 0 ) const;
        int getHeight( unsigned int mipMapLevel = 0 ) const;
        int getSize( unsigned int mipMapLevel = 0 ) const;
        int getLineSize( unsigned int mipMapLevel = 0 ) const;

        protected:

        Texture2DGeneric();

        Texture2DGeneric( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, 
                    const bool generateMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT viewFormat3 = DXGI_FORMAT_UNKNOWN );

        Texture2DGeneric( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT viewFormat3 = DXGI_FORMAT_UNKNOWN );

        Texture2DGeneric( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT viewFormat3 = DXGI_FORMAT_UNKNOWN );

        Texture2DGeneric( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu,const bool generateMipmaps, 
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2 = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT viewFormat3 = DXGI_FORMAT_UNKNOWN );

        Texture2DGeneric( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture );

        ~Texture2DGeneric() {};

        void initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, 
                    const bool generateMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                    const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu,const bool generateMipmaps, 
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture );

        void createTextureOnCpu( const int width, const int height );
        void createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height );
        // Re-interprets source data as the desired pixel type, removes padding at the end of each line and sets the data for the texture.
        void createTextureOnCpu( const char* data, const int width, const int height, const int lineSize );

        void createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                                 const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureViewsOnGpu( ID3D11Device& device, const int width, const int height, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                      DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat );

        virtual bool isUsageBindingSupported( TexUsage usage, TexBind binding );
        bool         supportsLoadCpuToGpu();
        bool         supportsMipmapGenerationOnGpu();

        DXGI_FORMAT readShaderResourceViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readRenderTargetViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readDepthStencilViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );
        DXGI_FORMAT readUnorderedAccessViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        D3D11_USAGE getTextureUsageFlag();
        UINT        getTextureCPUAccessFlags();
        UINT        getTextureBindFlags();
        D3D11_MAP   getMapForWriteFlag();
        UINT        getTextureMiscFlags();

        Texture2DFileInfo m_fileInfo;

        int  m_width;
        int  m_height;
        bool m_hasMipmapsOnGpu;

        std::vector< std::vector< PixelType > >   m_dataMipmaps;
        Microsoft::WRL::ComPtr< ID3D11Texture2D > m_texture;

        TexUsage m_usage;
        TexBind  m_binding;
        
        DXGI_FORMAT m_textureFormat;
        DXGI_FORMAT m_shaderResourceViewFormat;
        DXGI_FORMAT m_renderTargetViewFormat;
        DXGI_FORMAT m_depthStencilViewFormat;
        DXGI_FORMAT m_unorderedAccessViewFormat;

        
        Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_shaderResourceView;

        // Resource views for each mipmap level.
        std::vector< Microsoft::WRL::ComPtr< ID3D11DepthStencilView > >    m_depthStencilViews;
        std::vector< Microsoft::WRL::ComPtr< ID3D11RenderTargetView > >    m_renderTargetViews;
        std::vector< Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > > m_unorderedAccessViews;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric() :
        m_width( 0 ),
        m_height( 0 ),
        m_hasMipmapsOnGpu( false ),
        m_usage( TexUsage::Immutable ),
        m_binding( TexBind::ShaderResource ),
        m_textureFormat( DXGI_FORMAT_UNKNOWN ),
        m_shaderResourceViewFormat( DXGI_FORMAT_UNKNOWN ),
        m_renderTargetViewFormat( DXGI_FORMAT_UNKNOWN ),
        m_depthStencilViewFormat( DXGI_FORMAT_UNKNOWN ),
        m_unorderedAccessViewFormat( DXGI_FORMAT_UNKNOWN )
    {};

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, 
                             const bool generateMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                             DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) : Texture2DGeneric()
    {
        initialize( device, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
    }

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
                             const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, 
                             const bool generateMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                             DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) : Texture2DGeneric()
    {
        initialize( device, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
    }

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                            TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                            DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) : Texture2DGeneric()
    {
        initialize( device, width, height, storeOnCpu, storeOnGpu, usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
    }

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                             const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, 
                             TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                             DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) : Texture2DGeneric()
    {
        initialize( device, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
    }

    template< typename PixelType >
    Texture2DGeneric< PixelType >
        ::Texture2DGeneric( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture )
    {
        initialize( device, texture );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::initialize( ID3D11Device& device, const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, 
                      const bool generateMipmaps, TexUsage usage, TexBind binding,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( fileInfo.getPath() );

        initialize( device, fileData->cbegin(), fileData->cend(), fileInfo.getFormat(), storeOnCpu, storeOnGpu, generateMipmaps,
                    usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );

        m_fileInfo = fileInfo;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::initialize( ID3D11Device& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
                      const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
                      TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
                      DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        if ( !isUsageBindingSupported( usage, binding ) )
            throw std::exception( "Texture2DGeneric::initialize - configuration not supported (usage, binding)." );

        m_usage                     = usage;
        m_binding                   = binding;
        m_textureFormat             = textureFormat;
        m_shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "Texture2DGeneric::initialize - texture is set to be stored neither on CPU or GPU." );

        format; // Unused.

        // #TODO: FIT_BITMAP is not always the good image type (see "To do.txt").
        fipImage image( FIT_BITMAP );
        // #TODO: WARNING: casting away const qualifier on data - need to make sure that FreeImage doesn't modify the input data!
        const size_t dataSize = (dataEndIt - dataIt) / sizeof(char);
        fipMemoryIO data( const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&(*dataIt))), (DWORD)dataSize );

        // Parse image data.
        if ( !image.loadFromMemory( data ) )
            throw std::exception( "Texture2DGeneric::initialize - parsing texture from file in memory failed." );

        // Convert image to 32 bpp if needed.
        if ( image.getBitsPerPixel() != 32 ) {
            if ( image.getBitsPerPixel() == 24 ) {
                if ( !image.convertTo32Bits() )
                    throw std::exception( "Texture2DGeneric::initialize - loaded texture is 24 bits per pixel and it's conversion to 32 bits per pixel failed." );
            } else if ( image.getBitsPerPixel() != 8 ) {
                throw std::exception( "Texture2DGeneric::initialize - loaded texture is neither 32, 24 or 8 bits per pixel. This byte per pixel fromat is not supported." );
            }
        }

        createTextureOnCpu( (char*)image.accessPixels(), image.getWidth(), image.getHeight(), image.getLine() );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, m_dataMipmaps.front(), image.getWidth(), image.getHeight(), generateMipmaps, textureFormat );
            createTextureViewsOnGpu( device, image.getWidth(), image.getHeight(), m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        }

        if ( !storeOnCpu )
            m_dataMipmaps.clear();

        m_width           = image.getWidth();
        m_height          = image.getHeight();
        m_hasMipmapsOnGpu = generateMipmaps;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::initialize( ID3D11Device& device, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                      TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                      DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        if ( !isUsageBindingSupported( usage, binding ) )
            throw std::exception( "Texture2DGeneric::initialize - configuration not supported (usage, binding)." );
        
        m_usage                     = usage;
        m_binding                   = binding;
        m_textureFormat             = textureFormat;
        m_shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "Texture2DGeneric::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu )
            createTextureOnCpu( width, height );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, width, height, false, textureFormat );
            createTextureViewsOnGpu( device, width, height, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        }

        m_width           = width;
        m_height          = height;
        m_hasMipmapsOnGpu = false;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height,
                      const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps, TexUsage usage, TexBind binding,
                      DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        if ( !isUsageBindingSupported( usage, binding ) )
            throw std::exception( "Texture2DGeneric::initialize - configuration not supported (usage, binding)." );

        m_usage                     = usage;
        m_binding                   = binding;
        m_textureFormat             = textureFormat;
        m_shaderResourceViewFormat  = readShaderResourceViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_renderTargetViewFormat    = readRenderTargetViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_depthStencilViewFormat    = readDepthStencilViewFormat( viewFormat1, viewFormat2, viewFormat3 );
        m_unorderedAccessViewFormat = readUnorderedAccessViewFormat( viewFormat1, viewFormat2, viewFormat3 );

        if (!storeOnCpu && !storeOnGpu)
            throw std::exception( "Texture2DGeneric::initialize - texture is set to be stored neither on CPU or GPU." );

        if ( storeOnCpu ) {
            createTextureOnCpu( data, width, height );

            // #TODO: generate mipmaps on CPU.
        }

        if ( storeOnGpu ) {
            createTextureOnGpu( device, data, width, height, generateMipmaps, textureFormat );
            createTextureViewsOnGpu( device, width, height, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        }

        m_width           = width;
        m_height          = height;
        m_hasMipmapsOnGpu = generateMipmaps;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::initialize( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture )
    {
        //TODO: read texture details from GPU - such as dimensions, texture formats etc.

        m_usage            = TexUsage::Default;
        m_binding          = TexBind::RenderTarget;
        m_texture          = texture;
        m_width            = 0;
        m_height           = 0;
        m_hasMipmapsOnGpu  = false;

        createTextureViewsOnGpu( device, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
    }

    template< typename PixelType >
    TexUsage Texture2DGeneric< PixelType >
        ::getUsage() const
    {
        return m_usage;
    }

    template< typename PixelType >
    TexBind Texture2DGeneric< PixelType >
        ::getBinding() const
    {
        return m_binding;
    }

    template< typename PixelType >
    Asset::Type Texture2DGeneric< PixelType >
        ::getType() const
    {
	    return Asset::Type::Texture2D;
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<const Asset> > Texture2DGeneric< PixelType >
        ::getSubAssets( ) const
    {
	    return std::vector< std::shared_ptr<const Asset> >();
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<Asset> > Texture2DGeneric< PixelType >
        ::getSubAssets()
    {
	    return std::vector< std::shared_ptr<Asset> >( );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
    {
	    throw std::exception( "Texture2D::swapSubAsset - there are no sub-assets to be swapped." );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::setFileInfo( const Texture2DFileInfo& fileInfo )
    {
        m_fileInfo = fileInfo;
    }

    template< typename PixelType >
    const Texture2DFileInfo& Texture2DGeneric< PixelType >
        ::getFileInfo() const
    {
        return m_fileInfo;
    }

    template< typename PixelType >
    Texture2DFileInfo& Texture2DGeneric< PixelType >
        ::getFileInfo()
    {
        return m_fileInfo;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnCpu( const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given width or height has zero or negative value." );

        m_dataMipmaps.clear();
        m_dataMipmaps.push_back( std::vector< PixelType >() );
        m_dataMipmaps.back().resize( width * height );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given data size doesn't match given width and height." );

        m_dataMipmaps.clear();
        m_dataMipmaps.push_back( std::vector< PixelType >() );
        m_dataMipmaps.back().insert( m_dataMipmaps.back().begin(), data.cbegin(), data.cend() );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnCpu( const char* data, const int width, const int height, const int lineSize )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given width or height has zero or negative value." );

        m_dataMipmaps.clear();
        m_dataMipmaps.push_back( std::vector< PixelType >() );

        std::vector< PixelType >& dstData = m_dataMipmaps.back();
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
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given line size is too small to store the pixels for the given width and pixel type." );
        }
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given width or height has zero or negative value." );

        // Note: for now, only texture which support mipmap generation on GPU can have mipmaps
        // TODO: add manual creation of mipmaps on CPU to allow for mipmaps for different kinds of textures.
        const UINT mipmapCount = supportsMipmapGenerationOnGpu() ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        generateMipmaps; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = mipmapCount;
        desc.ArraySize          = mipmapCount;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = getTextureMiscFlags();

        HRESULT result = device.CreateTexture2D( &desc, nullptr, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "Texture2DGeneric::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                              const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given data size doesn't match given width and height." );

        // Note: for now, olny texture which support mipmap generation on GPU can have mipmaps.
        // TODO: add manual creation of mipmaps on CPU to allow for mipmaps for different kinds of textures.
        const UINT mipmapCount = supportsMipmapGenerationOnGpu() ? (1 + (UINT)(floor( log2( std::max( width, height ) ) ))) : 1;

        generateMipmaps; // Unused.

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = mipmapCount;
        desc.ArraySize          = mipmapCount;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = getTextureMiscFlags();

        D3D11_SUBRESOURCE_DATA dataDesc;
        ZeroMemory( &dataDesc, sizeof(dataDesc) );
        dataDesc.pSysMem          = data.data();
        dataDesc.SysMemPitch      = sizeof(PixelType) * width;
        dataDesc.SysMemSlicePitch = sizeof(PixelType) * width * height;

        HRESULT result = device.CreateTexture2D( &desc, &dataDesc, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "Texture2DGeneric::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureViewsOnGpu( ID3D11Device& device, const int width, const int height, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                   DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat )
    {
        const UINT textureBindFlags = getTextureBindFlags();

        const int mipmapCount = supportsMipmapGenerationOnGpu() ? (1 + (int)(floor( log2( std::max( width, height ) ) ))) : 1;

        if ( (textureBindFlags & D3D11_BIND_SHADER_RESOURCE) != 0 ) {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	        ZeroMemory( &desc, sizeof( desc ) );
	        desc.Format                    = shaderResourceViewFormat;
	        desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	        desc.Texture2D.MostDetailedMip = 0;
	        desc.Texture2D.MipLevels       = (UINT)-1; // All mipmaps.

            HRESULT result = device.CreateShaderResourceView( m_texture.Get(), &desc, m_shaderResourceView.ReleaseAndGetAddressOf() );
            if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );

            m_shaderResourceViewFormat = shaderResourceViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_RENDER_TARGET) != 0 ) 
        {
            if ( renderTargetViewFormat != DXGI_FORMAT_UNKNOWN )
            {
                m_renderTargetViews.resize( mipmapCount );
                for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
                {
                    D3D11_RENDER_TARGET_VIEW_DESC desc;
                    ZeroMemory( &desc, sizeof(desc) );
                    desc.Format             = renderTargetViewFormat;
                    desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipmapIndex;

                    HRESULT result = device.CreateRenderTargetView( m_texture.Get(), &desc, m_renderTargetViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                    if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );
                }

                m_renderTargetViewFormat = renderTargetViewFormat;
            }
            else // Note: Should happen only when render target is created from the frame back buffer.
            {
                m_renderTargetViews.resize( 1 );
                HRESULT result = device.CreateRenderTargetView( m_texture.Get(), nullptr, m_renderTargetViews[ 0 ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );

                m_renderTargetViewFormat = renderTargetViewFormat;
            }
        }

        if ( (textureBindFlags & D3D11_BIND_DEPTH_STENCIL) != 0 ) 
        {
            m_depthStencilViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_DEPTH_STENCIL_VIEW_DESC desc;
                ZeroMemory( &desc, sizeof(desc) );
                desc.Format             = depthStencilViewFormat;
                desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = mipmapIndex;

                HRESULT result = device.CreateDepthStencilView( m_texture.Get(), &desc, m_depthStencilViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating depth stencil view on GPU failed." );
            }

            m_depthStencilViewFormat = depthStencilViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0 ) 
        {
            m_unorderedAccessViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
                ZeroMemory( &desc, sizeof(desc) );
                desc.Format             = unorderedAccessViewFormat;
                desc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = mipmapIndex;

                HRESULT result = device.CreateUnorderedAccessView( m_texture.Get(), &desc, m_unorderedAccessViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating unordered access view on GPU failed." );
            }

            m_unorderedAccessViewFormat = unorderedAccessViewFormat;
        }
    }

    template< typename PixelType >
    bool Texture2DGeneric< PixelType >
        ::isUsageBindingSupported( TexUsage usage, TexBind binding )
    {
        if ( usage == TexUsage::Immutable && binding == TexBind::ShaderResource )
            return true;
        else if ( usage == TexUsage::Dynamic && binding == TexBind::ShaderResource )
            return true;
        else if ( usage == TexUsage::Default )
            return true; // All bindings are supported for default usage.

        return false;
    }

    template< typename PixelType >
    bool Texture2DGeneric< PixelType >
        ::supportsLoadCpuToGpu()
    {
        return m_usage == TexUsage::Dynamic;
    }
    template< typename PixelType >
    bool Texture2DGeneric< PixelType >
        ::supportsMipmapGenerationOnGpu()
    {
        return m_binding == TexBind::RenderTarget_ShaderResource ||
               m_binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource;
    }

    template< typename PixelType >
    DXGI_FORMAT  Texture2DGeneric< PixelType >
        ::readShaderResourceViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) 
    {
        if ( m_binding == TexBind::ShaderResource )
            return viewFormat1;
        else if ( m_binding == TexBind::DepthStencil_ShaderResource || m_binding == TexBind::RenderTarget_ShaderResource || m_binding == TexBind::UnorderedAccess_ShaderResource ) 
            return viewFormat2;
        else if ( m_binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 
            return viewFormat3;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< typename PixelType >
    DXGI_FORMAT  Texture2DGeneric< PixelType >
        ::readRenderTargetViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat2; viewFormat3; // Unused.

        if ( m_binding == TexBind::RenderTarget || m_binding == TexBind::RenderTarget_ShaderResource || 
             m_binding == TexBind::RenderTarget_UnorderedAccess || m_binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource )

             return viewFormat1;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< typename PixelType >
    DXGI_FORMAT  Texture2DGeneric< PixelType >
        ::readDepthStencilViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat2; viewFormat3; // Unused.

        if ( m_binding == TexBind::DepthStencil || m_binding == TexBind::DepthStencil_ShaderResource )

             return viewFormat1;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< typename PixelType >
    DXGI_FORMAT  Texture2DGeneric< PixelType >
        ::readUnorderedAccessViewFormat( DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 )
    {
        viewFormat3; // Unused.

        if ( m_binding == TexBind::UnorderedAccess || m_binding == TexBind::UnorderedAccess_ShaderResource ) 
            return viewFormat1;
        else if ( m_binding == TexBind::RenderTarget_UnorderedAccess || m_binding == TexBind::RenderTarget_UnorderedAccess_ShaderResource ) 

            return viewFormat2;
        else
            return DXGI_FORMAT_UNKNOWN;
    }

    template< typename PixelType >
    Microsoft::WRL::ComPtr< ID3D11Texture2D > Texture2DGeneric< PixelType >
        ::getTextureResource()
    {
        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DGeneric::getTextureResource - Texture not in GPU memory." );

        return m_texture;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::loadCpuToGpu - texture is not in CPU memory." );

        if ( !isInGpuMemory() ) {
            createTextureOnGpu( device, m_dataMipmaps.front(), m_width, m_height, getMipMapCountOnCpu() > 1, m_textureFormat );
            createTextureViewsOnGpu( device, m_width, m_height, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        } else {
            if ( supportsLoadCpuToGpu() ) {
                D3D11_MAPPED_SUBRESOURCE mappedResource;

                HRESULT result = deviceContext.Map( m_texture.Get(), 0, getMapForWriteFlag(), 0, &mappedResource );
                if ( result < 0 ) throw std::exception( "Texture2DGeneric::loadCpuToGpu - mapping texture for write failed." );

                std::memcpy( mappedResource.pData, m_dataMipmaps.front().data(), getSize() );
                mappedResource.RowPitch   = getLineSize();
                mappedResource.DepthPitch = getSize();

                deviceContext.Unmap( m_texture.Get(), 0 );
            } else {
                unloadFromGpu();
                createTextureOnGpu( device, m_dataMipmaps.front(), m_width, m_height, getMipMapCountOnCpu() > 1, m_textureFormat );
                createTextureViewsOnGpu( device, m_width, m_height, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
            }
        }
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::unloadFromCpu()
    {
        m_dataMipmaps.clear();
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::unloadFromGpu()
    {
	    m_texture.Reset();

        m_shaderResourceView.Reset();

        m_renderTargetViews.clear();
        m_depthStencilViews.clear();
        m_unorderedAccessViews.clear();

	    m_hasMipmapsOnGpu = false;
    }

    template< typename PixelType >
    bool Texture2DGeneric< PixelType >
        ::isInCpuMemory() const
    {
	    return !m_dataMipmaps.empty() && !m_dataMipmaps.front().empty();
    }
    
    template< typename PixelType >
    bool Texture2DGeneric< PixelType >
        ::isInGpuMemory() const
    {
        //TODO: Should also check if required views are created.
	    return m_texture != nullptr;
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getMipMapCountOnCpu()  const
    {
        return (int)m_dataMipmaps.size();
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getMipMapCountOnGpu() const
    {
         if ( isInGpuMemory() ) {
	        if ( m_hasMipmapsOnGpu )
		        return 1 + (int)( floor( log2( std::max( m_width, m_height ) ) ) );
	        else
		        return 1;
        } else {
	        return 0;
        }
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getBytesPerPixel() const
    {
        return sizeof( PixelType );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getWidth( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getWidth - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, m_width / ( 1 + (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getHeight( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getHeight - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, m_height / ( 1 + (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getSize - Incorrect level requested. There is no mipmap with such level." );

	    return getLineSize( mipMapLevel ) * getHeight( mipMapLevel );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getLineSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getLineSize - Incorrect level requested. There is no mipmap with such level." );

	    return getWidth( mipMapLevel ) * getBytesPerPixel();
    }

    template< typename PixelType >
    D3D11_USAGE Texture2DGeneric< PixelType >
        ::getTextureUsageFlag()
    {
        switch ( m_usage ) {
            case TexUsage::Immutable:
                return D3D11_USAGE_IMMUTABLE;
            case TexUsage::Dynamic:
                return D3D11_USAGE_DYNAMIC;
            case TexUsage::Default:
                return D3D11_USAGE_DEFAULT;
        }

        assert( false );
        return D3D11_USAGE_DEFAULT; // To avoid warning.
    }

    template< typename PixelType >
    UINT Texture2DGeneric< PixelType >
        ::getTextureCPUAccessFlags()
    {
        switch ( m_usage ) {
            case TexUsage::Immutable:
                return 0;
            case TexUsage::Dynamic:
                return D3D11_CPU_ACCESS_WRITE;
            case TexUsage::Default:
                return 0;
        }

        assert( false );
        return 0; // To avoid warning.
    }

    template< typename PixelType >
    UINT Texture2DGeneric< PixelType >
        ::getTextureBindFlags()
    {
        switch ( m_binding ) {
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

    template< typename PixelType >
    D3D11_MAP Texture2DGeneric< PixelType >
        ::getMapForWriteFlag()
    {
        return D3D11_MAP_WRITE_DISCARD;
    }

    template< typename PixelType >
    UINT Texture2DGeneric< PixelType >::getTextureMiscFlags()
    {
        UINT flags = 0;

        if ( supportsMipmapGenerationOnGpu() )
            flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

        return flags;
    }
}

