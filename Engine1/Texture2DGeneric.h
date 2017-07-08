#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <d3d11.h>
#include <wrl.h>

#include "int2.h"
#include "uchar4.h"
#include "Texture2DEnums.h"
#include "ImageLibrary.h"
#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

#include "Direct3DUtil.h"

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
        const Microsoft::WRL::ComPtr< ID3D11Texture2D > getTextureResource() const;

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext, const bool reload = false );
        void unloadFromCpu();
        void unloadFromGpu();

        int  getMipMapCountOnCpu()  const;
        int  getMipMapCountOnGpu() const;
        int  getBytesPerPixel() const;
        int  getWidth( unsigned int mipMapLevel = 0 ) const;
        int  getHeight( unsigned int mipMapLevel = 0 ) const;
        int2 getDimensions( unsigned int mipMapLevel = 0 ) const;
        int  getSize( unsigned int mipMapLevel = 0 ) const;
        int  getLineSize( unsigned int mipMapLevel = 0 ) const;

        const std::vector< PixelType >& getData( unsigned int mipMapLevel = 0 ) const;
        std::vector< PixelType >& getData( unsigned int mipMapLevel = 0 );

        void saveToFile( const std::string path, const Texture2DFileInfo::Format format ) const;

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
                    const bool hasMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, 
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
                         const bool hasMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, const std::vector< PixelType >& data, const int width, const int height, 
                    const bool storeOnCpu, const bool storeOnGpu,const bool generateMipmaps, 
                    TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                    DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 );

        void initialize( ID3D11Device& device, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture );

        void createTextureOnCpu( const int width, const int height, const bool generateMipmaps );
        void createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height, const bool generateMipmaps );
        // Re-interprets source data as the desired pixel type, removes padding at the end of each line and sets the data for the texture.
        void createTextureOnCpu( const char* data, const int width, const int height, const int lineSize, const bool generateMipmaps );

        void createMipmapsOnCpu( const int width, const int height, const int maxMipmapLevel = 0 );

        void createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureOnGpu( ID3D11Device& device, const std::vector< std::vector< PixelType > >& dataMipmaps, const int width, const int height, 
                                 const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureViewsOnGpu( ID3D11Device& device, const int width, const int height, const bool hasMipmaps, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
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

        int formatToFreeImagePlusSaveFlag( Texture2DFileInfo::Format format ) const;

        // Special averaging method used to avoid overflow for small integer types during mipmap calculations.
        PixelType average( PixelType val1, PixelType val2, PixelType val3, PixelType val4 );

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
        std::vector< Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > >  m_shaderResourceViews;
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
                            const bool hasMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
                            DXGI_FORMAT viewFormat1, DXGI_FORMAT viewFormat2, DXGI_FORMAT viewFormat3 ) : Texture2DGeneric()
    {
        initialize( device, width, height, storeOnCpu, storeOnGpu, hasMipmaps, usage, binding, textureFormat, viewFormat1, viewFormat2, viewFormat3 );
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

        setFileInfo( fileInfo );
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

        if ( image.getBitsPerPixel() != ( sizeof( PixelType ) * 8 ) ) {
            throw std::exception( ( "Texture2DGeneric::initialize - loaded texture has different pixel size ("
            + std::to_string( image.getBitsPerPixel() / 8 ) + " bytes) than texture PixelType (" + std::to_string( sizeof( PixelType ) )  + " bytes)." ).c_str() );
        }

        createTextureOnCpu( (char*)image.accessPixels(), image.getWidth(), image.getHeight(), image.getLine(), generateMipmaps );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, m_dataMipmaps, image.getWidth(), image.getHeight(), generateMipmaps, textureFormat );
            createTextureViewsOnGpu( device, image.getWidth(), image.getHeight(), generateMipmaps, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
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
                      const bool hasMipmaps, TexUsage usage, TexBind binding, DXGI_FORMAT textureFormat,
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
            createTextureOnCpu( width, height, hasMipmaps );

        if ( storeOnGpu ) {
            createTextureOnGpu( device, width, height, hasMipmaps, textureFormat );
            createTextureViewsOnGpu( device, width, height, hasMipmaps, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        }

        m_width           = width;
        m_height          = height;
        m_hasMipmapsOnGpu = hasMipmaps;
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
            createTextureOnCpu( data, width, height, generateMipmaps );
        }

        if ( storeOnGpu ) 
        {
            std::vector< std::vector< PixelType > > inputDataMipmaps;
            if ( !storeOnCpu )
                inputDataMipmaps.push_back( data );

            createTextureOnGpu( device, storeOnCpu ? m_dataMipmaps : inputDataMipmaps, width, height, generateMipmaps, textureFormat );
            createTextureViewsOnGpu( device, width, height, generateMipmaps, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
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

        createTextureViewsOnGpu( device, 0, 0, false, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::saveToFile( const std::string path, const Texture2DFileInfo::Format format ) const
    {
        if ( path.empty() )
            throw std::exception( "Texture2DGeneric::saveToFile - path cannot be empty." );

        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::saveToFile - texture not in CPU memory." );

        // #TODO: Could be improved significantly by avoiding copying memory to FreeImage image. Is it possible?

        fipImage image( FIT_BITMAP, getWidth(), getHeight(), sizeof( PixelType ) * 8 );

        const bool isPadded = image.getWidth() * sizeof( PixelType ) != image.getScanWidth();

        if ( !isPadded ) 
        {
            // Copy entire image.
            std::memcpy( image.accessPixels(), getData().data(), getWidth() * getHeight() * sizeof( PixelType ) );
        } 
        else 
        {
            // Copy image - line by line to account for padding at the end of each line.
            for ( int y = 0; y < getHeight(); ++y )
                std::memcpy( image.getScanLine( y ), getData().data() + y * getWidth(), getWidth() * sizeof( PixelType ) );
        }

        if ( !image.save( path.c_str(), formatToFreeImagePlusSaveFlag( format ) ) )
            throw std::exception( "Texture2DGeneric::saveToFile - saving texture failed." );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::formatToFreeImagePlusSaveFlag( const Texture2DFileInfo::Format format ) const
    {
        //#TODO: How about quality of the output file? Compression etc?

        if ( format == Texture2DFileInfo::Format::BMP )
            return BMP_DEFAULT;
        else if ( format == Texture2DFileInfo::Format::JPEG )
            return JPEG_QUALITYNORMAL;
        else if ( format == Texture2DFileInfo::Format::PNG )
            return PNG_Z_DEFAULT_COMPRESSION;
        else if ( format == Texture2DFileInfo::Format::TIFF )
            return TIFF_DEFAULT;
        else {
            assert( false );
            return 0;
        }
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

        #if defined(_DEBUG) 
        if ( m_texture ) {
		    std::string resourceName = std::string( "Texture2D (" + fileInfo.getPath() + ")" );
		    Direct3DUtil::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif
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
        ::createTextureOnCpu( const int width, const int height, const bool generateMipmaps )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given width or height has zero or negative value." );

        m_dataMipmaps.clear();
        m_dataMipmaps.push_back( std::vector< PixelType >() );
        m_dataMipmaps.back().resize( width * height );

        if ( generateMipmaps )
            createMipmapsOnCpu( width, height );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height, const bool generateMipmaps )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given width or height has zero or negative value." );

        if ( (int)data.size() != width * height )
            throw std::exception( "Texture2DGeneric::createTextureOnCpu - given data size doesn't match given width and height." );

        m_dataMipmaps.clear();
        m_dataMipmaps.push_back( std::vector< PixelType >() );
        m_dataMipmaps.back().insert( m_dataMipmaps.back().begin(), data.cbegin(), data.cend() );

        if ( generateMipmaps )
            createMipmapsOnCpu( width, height );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnCpu( const char* data, const int width, const int height, const int lineSize, const bool generateMipmaps )
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

        if ( generateMipmaps )
            createMipmapsOnCpu( width, height );
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createMipmapsOnCpu( const int width, const int height, const int maxMipmapLevel )
    {
        // If there is no image or it's 1x1 pixel - return.
        if ( m_dataMipmaps.empty() || m_dataMipmaps.front().size() <= 1 )
            return;

        // Reset mipmaps - leave only the first one.
        const int mipmapCount = 1 + (UINT)(floor( log2( std::max( width, height ) ) ));
        m_dataMipmaps.reserve( mipmapCount );
        m_dataMipmaps.resize( 1 );
        

        int prevMipmapWidth  = width;
        int prevMipmapHeight = height;

        // Loop until last mipmap is 1x1 pixel or we hit max given mipmap level.
        while ( m_dataMipmaps.back().size() > 1 && ( maxMipmapLevel <= 0 || m_dataMipmaps.size() <= maxMipmapLevel ) )
        {
            const std::vector< PixelType >& prevMipmap = m_dataMipmaps.back();

            // Add next empty mipmap.
            m_dataMipmaps.push_back( std::vector< PixelType >() );
            auto& nextMipmap = m_dataMipmaps.back();

            const int nextMipmapWidth  = std::max( 1, prevMipmapWidth / 2 );
            const int nextMipmapHeight = std::max( 1, prevMipmapHeight / 2 );

            nextMipmap.resize( nextMipmapWidth * nextMipmapHeight );

            for ( int y = 0; y < nextMipmapHeight; ++y )
            {
                for ( int x = 0; x < nextMipmapWidth; ++x ) 
                {
                    // Restrict pixel coords to avoid reading outside of texture.
                    const int topRowY      = 2 * y;
                    const int bottomRowY   = std::min( prevMipmapHeight - 1, 2 * y + 1 );
                    const int leftColumnX  = 2 * x;
                    const int rightColumnX = std::min( prevMipmapWidth - 1, 2 * x + 1 );

                    const int topLeftIdx     = topRowY    * prevMipmapWidth + leftColumnX;
                    const int topRightIdx    = topRowY    * prevMipmapWidth + rightColumnX;
                    const int bottomLeftIdx  = bottomRowY * prevMipmapWidth + leftColumnX;
                    const int bottomRightIdx = bottomRowY * prevMipmapWidth + rightColumnX;

                    nextMipmap[ y * nextMipmapWidth + x ] = 
                        average(
                            prevMipmap[ topLeftIdx ],
                            prevMipmap[ topRightIdx ],
                            prevMipmap[ bottomLeftIdx ],
                            prevMipmap[ bottomRightIdx ]
                        );
                }
            }

            prevMipmapWidth  = nextMipmapWidth;
            prevMipmapHeight = nextMipmapHeight;
        }
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given width or height has zero or negative value." );

        const UINT mipmapCount = generateMipmaps ? (1 + (UINT)(floor( log2( std::max( width, height ))))) : 1;

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = mipmapCount;
        desc.ArraySize          = 1;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = getTextureMiscFlags();

        HRESULT result = device.CreateTexture2D( &desc, nullptr, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "Texture2DGeneric::createTextureOnGpu - creating texture on GPU failed." );

        #if defined(_DEBUG) 
        if ( !getFileInfo().getPath().empty() ) {
		    std::string resourceName = std::string( "Texture2D (" + getFileInfo().getPath() + ")" );
		    Direct3DUtil::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif

        m_hasMipmapsOnGpu = mipmapCount > 1;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const std::vector< std::vector< PixelType > >& dataMipmaps, const int width, const int height, 
                              const bool generateMipmaps, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given width or height has zero or negative value." );

        if ( dataMipmaps.empty() )
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given data is empty." );

        if ( (int)dataMipmaps[0].size() != width * height )
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - given data size doesn't match given width and height." );

        // If it's possible - generate mipmaps on GPU, otherwise use the mipmaps generated on CPU.
        const UINT mipmapCount = std::max(
            (UINT)dataMipmaps.size(),
            supportsMipmapGenerationOnGpu() && generateMipmaps ? ( 1 + (UINT)( floor( log2( std::max( width, height ))))) : 1
        );

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = mipmapCount;
        desc.ArraySize          = 1;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = getTextureUsageFlag();
        desc.BindFlags          = getTextureBindFlags();
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = getTextureMiscFlags();

        std::vector< D3D11_SUBRESOURCE_DATA > dataDesc;
        if ( supportsMipmapGenerationOnGpu() )
        {
            dataDesc.resize( 1 );

            dataDesc[0].pSysMem          = dataMipmaps[0].data();
            dataDesc[0].SysMemPitch      = sizeof( PixelType ) * width;
            dataDesc[0].SysMemSlicePitch = sizeof( PixelType ) * width * height;
        }
        else
        {
            dataDesc.resize( mipmapCount );

            int mipmapWidth  = width;
            int mipmapHeight = height;

            for ( int i = 0; i < (int)mipmapCount; ++i )
            {
                // Check if mipmap data size is correct.
                if ( dataMipmaps[ i ].size() != mipmapWidth * mipmapHeight )
                    throw std::exception( ("Texture2DGeneric::createTextureOnGpu - given data size for mipmap level " + std::to_string( i ) + " doesn't match expected width and height.").c_str() );

                dataDesc[ i ].pSysMem          = dataMipmaps[ i ].data();
                dataDesc[ i ].SysMemPitch      = sizeof( PixelType ) * mipmapWidth;
                dataDesc[ i ].SysMemSlicePitch = sizeof( PixelType ) * mipmapWidth * mipmapHeight;

                mipmapWidth  = std::max( 1, mipmapWidth / 2 );
                mipmapHeight = std::max( 1, mipmapHeight / 2 );
            }
        }

        HRESULT result = device.CreateTexture2D( &desc, dataDesc.data(), m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "Texture2DGeneric::createTextureOnGpu - creating texture on GPU failed." );

        #if defined(_DEBUG) 
        if ( !getFileInfo().getPath().empty() ) {
		    std::string resourceName = std::string( "Texture2D (" + getFileInfo().getPath() + ")" );
		    Direct3DUtil::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif

        m_hasMipmapsOnGpu = mipmapCount > 1;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::createTextureViewsOnGpu( ID3D11Device& device, const int width, const int height, const bool hasMipmaps, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                   DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat )
    {
        const UINT textureBindFlags = getTextureBindFlags();

        const int mipmapCount = hasMipmaps ? (1 + (int)(floor( log2( std::max( width, height ) ) ))) : 1;

        if ( (textureBindFlags & D3D11_BIND_SHADER_RESOURCE) != 0 ) {
            { // Create shader resource view to access all mipmap levels.
                D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	            ZeroMemory( &desc, sizeof( desc ) );
	            desc.Format                    = shaderResourceViewFormat;
	            desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	            desc.Texture2D.MostDetailedMip = 0;
	            desc.Texture2D.MipLevels       = (UINT)-1; // All mipmaps.

                HRESULT result = device.CreateShaderResourceView( m_texture.Get(), &desc, m_shaderResourceView.ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );
            }

            // Create shader resource view to access each mipmap separately.
            m_shaderResourceViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	            ZeroMemory( &desc, sizeof( desc ) );
	            desc.Format                    = shaderResourceViewFormat;
	            desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	            desc.Texture2D.MostDetailedMip = mipmapIndex;
	            desc.Texture2D.MipLevels       = 1;

                HRESULT result = device.CreateShaderResourceView( m_texture.Get(), &desc, m_shaderResourceViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );
            }

            m_shaderResourceViewFormat = shaderResourceViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_RENDER_TARGET) != 0 ) 
        {
            if ( renderTargetViewFormat != DXGI_FORMAT_UNKNOWN )
            { // Create render target view to access each mipmap separately.
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
        { // Create depth stencil view to access each mipmap separately.
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
        { // Create unordered access view to access each mipmap separately.
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
    const Microsoft::WRL::ComPtr< ID3D11Texture2D > Texture2DGeneric< PixelType >
        ::getTextureResource() const
    {
        if ( !isInGpuMemory() )
            throw std::exception( "Texture2DGeneric::getTextureResource - Texture not in GPU memory." );

        return m_texture;
    }

    template< typename PixelType >
    void Texture2DGeneric< PixelType >
        ::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext, const bool reload )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::loadCpuToGpu - texture is not in CPU memory." );

        if ( !isInGpuMemory() ) 
        {
            createTextureOnGpu( device, m_dataMipmaps, m_width, m_height, getMipMapCountOnCpu() > 1, m_textureFormat );
            createTextureViewsOnGpu( device, m_width, m_height, getMipMapCountOnCpu() > 1, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
        } 
        else if ( reload ) 
        {
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
                createTextureOnGpu( device, m_dataMipmaps, m_width, m_height, getMipMapCountOnCpu() > 1, m_textureFormat );
                createTextureViewsOnGpu( device, m_width, m_height, getMipMapCountOnCpu() > 1, m_shaderResourceViewFormat, m_renderTargetViewFormat, m_depthStencilViewFormat, m_unorderedAccessViewFormat );
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

        m_shaderResourceViews.clear();
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

	    return std::max( 1, m_width / (int)pow( 2, (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int Texture2DGeneric< PixelType >
        ::getHeight( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getHeight - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, m_height / (int)pow( 2, (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int2 Texture2DGeneric< PixelType >
        ::getDimensions( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
            throw std::exception( "Texture2DGeneric::getDimensions - Incorrect level requested. There is no mipmap with such level." );

        return int2( std::max( 1, m_width / (int)pow( 2, (int)mipMapLevel ) ), std::max( 1, m_height / (int)pow( 2, (int)mipMapLevel ) ) );
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
    const std::vector< PixelType >& Texture2DGeneric< PixelType >
        ::getData( unsigned int mipMapLevel = 0 ) const
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::getData - texture is not in CPU memory." );
        else if ( (int)mipMapLevel >= getMipMapCountOnCpu() )
            throw std::exception( "Texture2DGeneric::getData - Incorrect level requested. There is no mipmap with such level." );

        return m_dataMipmaps.at( mipMapLevel );
    }

    template< typename PixelType >
    std::vector< PixelType >& Texture2DGeneric< PixelType >
        ::getData( unsigned int mipMapLevel = 0 )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::getData - texture is not in CPU memory." );
        else if ( (int)mipMapLevel >= getMipMapCountOnCpu() )
            throw std::exception( "Texture2DGeneric::getData - Incorrect level requested. There is no mipmap with such level." );

        return m_dataMipmaps.at( mipMapLevel );
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

    template<>
    inline unsigned char Texture2DGeneric< unsigned char >
        ::average( unsigned char val1, unsigned char val2, unsigned char val3, unsigned char val4 )
    {
        // Note: Unsafe cast to smaller type.
        return (unsigned char)( ((int)val1 + (int)val2 + (int)val3 + (int)val4) / 4 );
    }

    template<>
    inline uchar4 Texture2DGeneric< uchar4 >
        ::average( uchar4 val1, uchar4 val2, uchar4 val3, uchar4 val4 )
    {
        uint4 value = ((uint4)val1 + (uint4)val2 + (uint4)val3 + (uint4)val4) / 4;

        // Note: Unsafe cast to smaller type.
        return uchar4( (unsigned char)value.x, (unsigned char)value.y, (unsigned char)value.z, (unsigned char)value.w );
    }

    template<>
    inline float Texture2DGeneric< float >
        ::average( float val1, float val2, float val3, float val4 )
    {
        return (val1 + val2 + val3 + val4) / 4.0f;
    }

    template<>
    inline float4 Texture2DGeneric< float4 >
        ::average( float4 val1, float4 val2, float4 val3, float4 val4 )
    {
        return (val1 + val2 + val3 + val4) / 4.0f;
    }

    //template<typename PixelType>
    //inline PixelType Texture2DGeneric<PixelType>::sum( PixelType val1, PixelType val2, PixelType val3, PixelType val4 )
    //{
    //    return PixelType();
    //}
}

