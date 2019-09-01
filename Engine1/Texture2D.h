#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <d3d11_3.h>
#include <wrl.h>

#include "int2.h"
#include "uchar4.h"
#include "uint4.h"
#include "ImageLibrary.h"
#include "BinaryFile.h"
#include "TextureBase.h"

#include "DX11Util.h"

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
    class Texture2D : public TextureBase< PixelType >
    {
        public:

		virtual ~Texture2D() = default;

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();

        void swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                     setFileInfo( const Texture2DFileInfo& fileInfo );
        const Texture2DFileInfo& getFileInfo() const;
        Texture2DFileInfo&       getFileInfo();

        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        Microsoft::WRL::ComPtr< ID3D11Texture2D > getTextureResource();
        const Microsoft::WRL::ComPtr< ID3D11Texture2D > getTextureResource() const;

		// mipmapLevel = -1 - returns view to all mipmaps.
		// mipmapLevel >= 0 - returns view to a specific mipmap.
		ID3D11ShaderResourceView* getShaderResourceView(int mipMapLevel = -1) const;

        DXGI_FORMAT getTextureFormat() const;

        void loadCpuToGpu( ID3D11Device3& device, ID3D11DeviceContext3& deviceContext, const bool reload = false );
        void unloadFromCpu();
        void unloadFromGpu();

		void createMipmapsOnCpu(const int width, const int height, const int maxMipmapLevel = 0);
		void createMipMapsOnGpu(ID3D11DeviceContext3& deviceContext);

        int  getMipMapCountOnCpu()  const;
        int  getMipMapCountOnGpu() const;
        int  getBytesPerPixel() const;
        int  getWidth() const { return getWidth( 0 ); };
        int  getHeight() const { return getHeight( 0 ); };
        int  getWidth( unsigned int mipMapLevel ) const;
        int  getHeight( unsigned int mipMapLevel ) const;
        int2 getDimensions( unsigned int mipMapLevel = 0 ) const;
        int  getSize( unsigned int mipMapLevel = 0 ) const;
        int  getLineSize( unsigned int mipMapLevel = 0 ) const;

        PixelType sampleBilinearData( float2 texcoords, unsigned int mipMapLevel = 0 ) const;

        void setDataPixel( float2 texcoords, PixelType color, unsigned int mipMapLevel = 0 );
        void setDataPixel( int2 position, PixelType color, unsigned int mipMapLevel = 0 );

        PixelType getDataPixel( float2 texcoords, unsigned int mipMapLevel = 0 );
        PixelType getDataPixel( int2 position, unsigned int mipMapLevel = 0 );

        const std::vector< PixelType >& getData() const { return getData( 0 ); };
        std::vector< PixelType >&       getData()       { return getData( 0 ); };
        const std::vector< PixelType >& getData( unsigned int mipMapLevel ) const;
        std::vector< PixelType >&       getData( unsigned int mipMapLevel );

        protected:

		Texture2D(
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags );

        Texture2D( 
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags,
			ID3D11Device3& device, 
			const Texture2DFileInfo& fileInfo, 
			const bool storeOnCpu, const bool storeOnGpu, 
            const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, 
			DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        Texture2D( 
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags,
			ID3D11Device3& device, 
			std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
            const Texture2DFileInfo::Format format, 
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps,
            DXGI_FORMAT textureFormat, 
			DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        Texture2D( 
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags,
			ID3D11Device3& device, 
			const int width, const int height, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool hasMipmaps, 
			DXGI_FORMAT textureFormat, 
			DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        Texture2D( 
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags,
			ID3D11Device3& device, 
			const std::vector< PixelType >& data, 
			const int width, const int height, 
            const bool storeOnCpu, const bool storeOnGpu,
			const bool generateMipmaps, 
            DXGI_FORMAT textureFormat,
			DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        Texture2D( 
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags,
			ID3D11Device3& device, 
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
            DXGI_FORMAT textureFormat,
		    DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        void initializeFromFileData( 
			ID3D11Device3& device, std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
            const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
            DXGI_FORMAT textureFormat, 
			DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat );

        void createTextureOnCpu( const int width, const int height, const bool generateMipmaps );
        void createTextureOnCpu( const std::vector< PixelType >& data, const int width, const int height, const bool generateMipmaps );
        // Re-interprets source data as the desired pixel type, removes padding at the end of each line and sets the data for the texture.
        void createTextureOnCpu( const char* data, const int width, const int height, const int lineSize, const bool generateMipmaps );

        void createTextureOnGpu( ID3D11Device3& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureOnGpu( ID3D11Device3& device, const std::vector< std::vector< PixelType > >& dataMipmaps, const int width, const int height, 
                                 const bool generateMipmaps, DXGI_FORMAT textureFormat );

        void createTextureViewsOnGpu( ID3D11Device3& device, const int width, const int height, const bool hasMipmaps, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                      DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat );

        bool supportsLoadCpuToGpu();
        bool supportsMipmapGenerationOnGpu();

        D3D11_MAP    getMapForWriteFlag();
		unsigned int getTextureMiscFlags();

        // Special averaging method used to avoid overflow for small integer types during mipmap calculations.
        PixelType average( PixelType val1, PixelType val2, PixelType val3, PixelType val4 ) const;

        PixelType weightedAverage( 
            PixelType val1, float weight1, 
            PixelType val2, float weight2, 
            PixelType val3, float weight3, 
            PixelType val4, float weight4 ) const;

		const D3D11_USAGE m_usageFlag;
		const unsigned int m_bindFlags;
		const unsigned int m_CPUAccessFlags;

        Texture2DFileInfo m_fileInfo;

        int  m_width;
        int  m_height;
        bool m_hasMipmapsOnGpu;

        std::vector< std::vector< PixelType > >   m_dataMipmaps;
        Microsoft::WRL::ComPtr< ID3D11Texture2D > m_texture;

        DXGI_FORMAT m_textureFormat;
        DXGI_FORMAT m_srvFormat;  // Shader Resource View Format
        DXGI_FORMAT m_rtvFormat;  // Render Target View Format
        DXGI_FORMAT m_dsvFormat;  // Depth Stencil View Format
        DXGI_FORMAT m_uavFormat;  // Unordered Access View Format

        
        Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_shaderResourceView;

        // Resource views for each mipmap level.
        std::vector< Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > >  m_srViews;
        std::vector< Microsoft::WRL::ComPtr< ID3D11DepthStencilView > >    m_dsViews;
        std::vector< Microsoft::WRL::ComPtr< ID3D11RenderTargetView > >    m_rtViews;
        std::vector< Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > > m_uaViews;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template< typename PixelType >
    Texture2D< PixelType >
		::Texture2D(
			const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags ) :
		m_usageFlag( usageFlag ),
		m_bindFlags( bindFlags ),
		m_CPUAccessFlags( CPUAccessFlags ),
        m_width( 0 ),
        m_height( 0 ),
        m_hasMipmapsOnGpu( false ),
        m_textureFormat( DXGI_FORMAT_UNKNOWN ),
        m_srvFormat( DXGI_FORMAT_UNKNOWN ),
        m_rtvFormat( DXGI_FORMAT_UNKNOWN ),
        m_dsvFormat( DXGI_FORMAT_UNKNOWN ),
        m_uavFormat( DXGI_FORMAT_UNKNOWN )
    {};

    template< typename PixelType >
    Texture2D< PixelType >::Texture2D( 
		const D3D11_USAGE usageFlag,
		const unsigned int bindFlags,
		const unsigned int CPUAccessFlags,
		ID3D11Device3& device, 
		const Texture2DFileInfo& fileInfo, 
		const bool storeOnCpu, const bool storeOnGpu, 
        const bool generateMipmaps, 
		DXGI_FORMAT textureFormat, 
        DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat ) 
		: Texture2D( usageFlag, bindFlags, CPUAccessFlags )
    {
		std::shared_ptr< std::vector<char> > fileData = BinaryFile::load(fileInfo.getPath());

		initializeFromFileData(
			device, 
			fileData->cbegin(), fileData->cend(), 
			fileInfo.getFormat(), 
			storeOnCpu, storeOnGpu, 
			generateMipmaps,
			textureFormat, 
			uavFormat, srvFormat, rtvDepthFormat );

		setFileInfo(fileInfo);
    }

    template< typename PixelType >
    Texture2D< PixelType >::Texture2D( 
		const D3D11_USAGE usageFlag,
		const unsigned int bindFlags,
		const unsigned int CPUAccessFlags,
		ID3D11Device3& device, 
		std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
        const Texture2DFileInfo::Format format, 
		const bool storeOnCpu, const bool storeOnGpu, 
        const bool generateMipmaps, 
		DXGI_FORMAT textureFormat, 
        DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat ) 
		: Texture2D( usageFlag, bindFlags, CPUAccessFlags )
    {
		initializeFromFileData( 
			device, 
			dataIt, dataEndIt, 
			format, 
			storeOnCpu, storeOnGpu, 
			generateMipmaps, 
			textureFormat, 
			uavFormat, srvFormat, rtvDepthFormat );
    }

    template< typename PixelType >
    Texture2D< PixelType >::Texture2D( 
		const D3D11_USAGE usageFlag,
		const unsigned int bindFlags,
		const unsigned int CPUAccessFlags,
		ID3D11Device3& device, 
		const int width, const int height, 
		const bool storeOnCpu, const bool storeOnGpu,
        const bool hasMipmaps, 
		DXGI_FORMAT textureFormat,
        DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat ) 
		: Texture2D( usageFlag, bindFlags, CPUAccessFlags )
    {
		const bool isRenderTarget = (m_bindFlags & D3D11_BIND_RENDER_TARGET) != 0;
		const bool isDepthStencil = (m_bindFlags & D3D11_BIND_DEPTH_STENCIL) != 0;

		m_textureFormat = textureFormat;
		m_srvFormat     = srvFormat;
		m_rtvFormat     = (isRenderTarget ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_dsvFormat     = (isDepthStencil ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_uavFormat     = uavFormat;

		if (!storeOnCpu && !storeOnGpu)
			throw std::exception("Texture2DGeneric::initialize - texture is set to be stored neither on CPU or GPU.");

		if (storeOnCpu)
			createTextureOnCpu(width, height, hasMipmaps);

		if (storeOnGpu) {
			createTextureOnGpu(device, width, height, hasMipmaps, textureFormat);
			createTextureViewsOnGpu(device, width, height, hasMipmaps, m_srvFormat, m_rtvFormat, m_dsvFormat, m_uavFormat);
		}

		m_width = width;
		m_height = height;
		m_hasMipmapsOnGpu = hasMipmaps;
    }

    template< typename PixelType >
    Texture2D< PixelType >::Texture2D( 
		const D3D11_USAGE usageFlag,
		const unsigned int bindFlags,
		const unsigned int CPUAccessFlags,
		ID3D11Device3& device, 
		const std::vector< PixelType >& data, 
		const int width, const int height, 
        const bool storeOnCpu, const bool storeOnGpu, 
		const bool generateMipmaps, 
        DXGI_FORMAT textureFormat,
		DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat ) 
		: Texture2D( usageFlag, bindFlags, CPUAccessFlags )
    {
		const bool isRenderTarget = (m_bindFlags & D3D11_BIND_RENDER_TARGET) != 0;
		const bool isDepthStencil = (m_bindFlags & D3D11_BIND_DEPTH_STENCIL) != 0;

		m_textureFormat = textureFormat;
		m_srvFormat     = srvFormat;
		m_rtvFormat     = (isRenderTarget ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_dsvFormat     = (isDepthStencil ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_uavFormat     = uavFormat;

		if (!storeOnCpu && !storeOnGpu)
			throw std::exception("Texture2DGeneric::initialize - texture is set to be stored neither on CPU or GPU.");

		if (storeOnCpu) {
			createTextureOnCpu(data, width, height, generateMipmaps);
		}

		if (storeOnGpu)
		{
			std::vector< std::vector< PixelType > > inputDataMipmaps;
			if (!storeOnCpu)
				inputDataMipmaps.push_back(data);

			createTextureOnGpu(device, storeOnCpu ? m_dataMipmaps : inputDataMipmaps, width, height, generateMipmaps, textureFormat);
			createTextureViewsOnGpu(device, width, height, generateMipmaps, m_srvFormat, m_rtvFormat, m_dsvFormat, m_uavFormat);
		}

		m_width = width;
		m_height = height;
		m_hasMipmapsOnGpu = generateMipmaps;
    }

    template< typename PixelType >
    Texture2D< PixelType >
		::Texture2D(const D3D11_USAGE usageFlag,
			const unsigned int bindFlags,
			const unsigned int CPUAccessFlags, 
			ID3D11Device3& device, 
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
            DXGI_FORMAT textureFormat,
		    DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat )
		: Texture2D( usageFlag, bindFlags, CPUAccessFlags )
    {
		//TODO: read texture details from GPU - such as dimensions, texture formats etc. If possible..
        const bool isRenderTarget = (m_bindFlags & D3D11_BIND_RENDER_TARGET) != 0;
		const bool isDepthStencil = (m_bindFlags & D3D11_BIND_DEPTH_STENCIL) != 0;

        m_textureFormat = textureFormat;
		m_srvFormat     = srvFormat;
		m_rtvFormat     = (isRenderTarget ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_dsvFormat     = (isDepthStencil ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_uavFormat     = uavFormat;

		m_texture         = texture;
		m_width           = 0;
		m_height          = 0;
		m_hasMipmapsOnGpu = false;

		createTextureViewsOnGpu(
			device, 0, 0, 
			false, 
			m_srvFormat, 
			m_rtvFormat, m_dsvFormat, m_uavFormat );
    }

    template< typename PixelType >
    void Texture2D< PixelType >::initializeFromFileData( 
		ID3D11Device3& device, 
		std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, 
		const Texture2DFileInfo::Format format, 
		const bool storeOnCpu, const bool storeOnGpu, 
		const bool generateMipmaps,
		DXGI_FORMAT textureFormat,
		DXGI_FORMAT uavFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvDepthFormat )
    {
		const bool isRenderTarget = (m_bindFlags & D3D11_BIND_RENDER_TARGET) != 0;
		const bool isDepthStencil = (m_bindFlags & D3D11_BIND_DEPTH_STENCIL) != 0;

		m_textureFormat = textureFormat;
		m_srvFormat     = srvFormat;
		m_rtvFormat     = (isRenderTarget ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_dsvFormat     = (isDepthStencil ? rtvDepthFormat : DXGI_FORMAT_UNKNOWN);
		m_uavFormat     = uavFormat;

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

        createTextureOnCpu( 
			(char*)image.accessPixels(), 
			image.getWidth(), image.getHeight(), 
			image.getLine(), 
			generateMipmaps );

        if ( storeOnGpu ) 
		{
            createTextureOnGpu( 
				device, 
				m_dataMipmaps, 
				image.getWidth(), image.getHeight(), 
				generateMipmaps, 
				textureFormat );

            createTextureViewsOnGpu( 
				device, 
				image.getWidth(), image.getHeight(), 
				generateMipmaps, 
				m_srvFormat, m_rtvFormat, m_dsvFormat, m_uavFormat );
        }

		if ( !storeOnCpu ) {
			m_dataMipmaps.clear();
		}

        m_width           = image.getWidth();
        m_height          = image.getHeight();
        m_hasMipmapsOnGpu = generateMipmaps;
    }

    template< typename PixelType >
    Asset::Type Texture2D< PixelType >
        ::getType() const
    {
	    return Asset::Type::Texture2D;
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<const Asset> > Texture2D< PixelType >
        ::getSubAssets( ) const
    {
	    return std::vector< std::shared_ptr<const Asset> >();
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<Asset> > Texture2D< PixelType >
        ::getSubAssets()
    {
	    return std::vector< std::shared_ptr<Asset> >( );
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
    {
	    throw std::exception( "Texture2D::swapSubAsset - there are no sub-assets to be swapped." );
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::setFileInfo( const Texture2DFileInfo& fileInfo )
    {
        m_fileInfo = fileInfo;

        #if defined(_DEBUG) 
        if ( m_texture ) {
		    std::string resourceName = std::string( "Texture2D (" + fileInfo.getPath() + ")" );
		    DX11Util::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif
    }

    template< typename PixelType >
    const Texture2DFileInfo& Texture2D< PixelType >
        ::getFileInfo() const
    {
        return m_fileInfo;
    }

    template< typename PixelType >
    Texture2DFileInfo& Texture2D< PixelType >
        ::getFileInfo()
    {
        return m_fileInfo;
    }

    template< typename PixelType >
    void Texture2D< PixelType >
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
    void Texture2D< PixelType >
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
    void Texture2D< PixelType >
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
    void Texture2D< PixelType >
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
	void Texture2D< PixelType >
		::createMipMapsOnGpu(ID3D11DeviceContext3& deviceContext)
	{
		if (!supportsMipmapGenerationOnGpu())
			throw std::exception("Texture2DGeneric::createMipMapsOnGpu - this texture type doesn't support mipmap generation on GPU.");

		if (!isInGpuMemory())
			throw std::exception("Texture2DGeneric::createMipMapsOnGpu - Can't generate because texture is not in GPU memory.");

		deviceContext.GenerateMips(m_shaderResourceView.Get());

		m_hasMipmapsOnGpu = true;
	}

    template< typename PixelType >
    void Texture2D< PixelType >
        ::createTextureOnGpu( ID3D11Device3& device, const int width, const int height, const bool generateMipmaps, DXGI_FORMAT textureFormat )
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
        desc.Usage              = m_usageFlag;
        desc.BindFlags          = m_bindFlags;
        desc.CPUAccessFlags     = m_CPUAccessFlags;
        desc.MiscFlags          = getTextureMiscFlags();

        HRESULT result = device.CreateTexture2D( &desc, nullptr, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "Texture2DGeneric::createTextureOnGpu - creating texture on GPU failed." );

        #if defined(_DEBUG) 
        if ( !getFileInfo().getPath().empty() ) {
		    std::string resourceName = std::string( "Texture2D (" + getFileInfo().getPath() + ")" );
		    DX11Util::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif

        m_hasMipmapsOnGpu = mipmapCount > 1;
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::createTextureOnGpu( ID3D11Device3& device, const std::vector< std::vector< PixelType > >& dataMipmaps, const int width, const int height, 
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
        desc.Usage              = m_usageFlag;
        desc.BindFlags          = m_bindFlags;
        desc.CPUAccessFlags     = m_CPUAccessFlags;
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
		    DX11Util::setResourceName( *m_texture.Get(), resourceName );
        }
        #endif

        m_hasMipmapsOnGpu = mipmapCount > 1;
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::createTextureViewsOnGpu( ID3D11Device3& device, const int width, const int height, const bool hasMipmaps, DXGI_FORMAT shaderResourceViewFormat, DXGI_FORMAT renderTargetViewFormat,
                                   DXGI_FORMAT depthStencilViewFormat, DXGI_FORMAT unorderedAccessViewFormat )
    {
        const UINT textureBindFlags = m_bindFlags;

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
            m_srViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	            ZeroMemory( &desc, sizeof( desc ) );
	            desc.Format                    = shaderResourceViewFormat;
	            desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	            desc.Texture2D.MostDetailedMip = mipmapIndex;
	            desc.Texture2D.MipLevels       = 1;

                HRESULT result = device.CreateShaderResourceView( m_texture.Get(), &desc, m_srViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating shader resource view on GPU failed." );
            }

            m_srvFormat = shaderResourceViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_RENDER_TARGET) != 0 ) 
        {
            if ( renderTargetViewFormat != DXGI_FORMAT_UNKNOWN )
            { // Create render target view to access each mipmap separately.
                m_rtViews.resize( mipmapCount );
                for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
                {
                    D3D11_RENDER_TARGET_VIEW_DESC desc;
                    ZeroMemory( &desc, sizeof(desc) );
                    desc.Format             = renderTargetViewFormat;
                    desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipmapIndex;

                    HRESULT result = device.CreateRenderTargetView( m_texture.Get(), &desc, m_rtViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                    if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );
                }

                m_rtvFormat = renderTargetViewFormat;
            }
            else // Note: Should happen only when render target is created from the frame back buffer.
            {
                m_rtViews.resize( 1 );
                HRESULT result = device.CreateRenderTargetView( m_texture.Get(), nullptr, m_rtViews[ 0 ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating render target view on GPU failed." );

                m_rtvFormat = renderTargetViewFormat;
            }
        }

        if ( (textureBindFlags & D3D11_BIND_DEPTH_STENCIL) != 0 ) 
        { // Create depth stencil view to access each mipmap separately.
            m_dsViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_DEPTH_STENCIL_VIEW_DESC desc;
                ZeroMemory( &desc, sizeof(desc) );
                desc.Format             = depthStencilViewFormat;
                desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = mipmapIndex;

                HRESULT result = device.CreateDepthStencilView( m_texture.Get(), &desc, m_dsViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating depth stencil view on GPU failed." );
            }

            m_dsvFormat = depthStencilViewFormat;
        }

        if ( (textureBindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0 ) 
        { // Create unordered access view to access each mipmap separately.
            m_uaViews.resize( mipmapCount );
            for ( int mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
            {
                D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
                ZeroMemory( &desc, sizeof(desc) );
                desc.Format             = unorderedAccessViewFormat;
                desc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = mipmapIndex;

                HRESULT result = device.CreateUnorderedAccessView( m_texture.Get(), &desc, m_uaViews[ mipmapIndex ].ReleaseAndGetAddressOf() );
                if ( result < 0 ) throw std::exception( "Texture2D::createTextureViewsOnGpu - creating unordered access view on GPU failed." );
            }

            m_uavFormat = unorderedAccessViewFormat;
        }
    }

    template< typename PixelType >
    bool Texture2D< PixelType >
        ::supportsLoadCpuToGpu()
    {
		return (m_usageFlag & D3D11_USAGE_DYNAMIC) != 0;
    }
    template< typename PixelType >
    bool Texture2D< PixelType >
        ::supportsMipmapGenerationOnGpu()
    {
		return (m_bindFlags & D3D11_BIND_SHADER_RESOURCE) != 0
			&& (m_bindFlags & D3D11_BIND_RENDER_TARGET) != 0;

		// #TODO: Check if immutable texture can have mipmaps generated on GPU.
		// #TODO: Check if depth stencil can have mipmaps generated on GPU.
    }

    template< typename PixelType >
    Microsoft::WRL::ComPtr< ID3D11Texture2D > Texture2D< PixelType >
        ::getTextureResource()
    {
        if ( !isInGpuMemory() ) 
                throw std::exception( "Texture2DGeneric::getTextureResource - Texture not in GPU memory." );

        return m_texture;
    }

    template< typename PixelType >
    const Microsoft::WRL::ComPtr< ID3D11Texture2D > Texture2D< PixelType >
        ::getTextureResource() const
    {
        if ( !isInGpuMemory() )
            throw std::exception( "Texture2DGeneric::getTextureResource - Texture not in GPU memory." );

        return m_texture;
    }

    template< typename PixelType >
    DXGI_FORMAT Texture2D< PixelType >::getTextureFormat() const
    {
        return m_textureFormat;
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::loadCpuToGpu( ID3D11Device3& device, ID3D11DeviceContext3& deviceContext, const bool reload )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::loadCpuToGpu - texture is not in CPU memory." );

        if ( !isInGpuMemory() ) 
        {
            createTextureOnGpu( device, m_dataMipmaps, m_width, m_height, getMipMapCountOnCpu() > 1, m_textureFormat );
            createTextureViewsOnGpu( device, m_width, m_height, getMipMapCountOnCpu() > 1, m_srvFormat, m_rtvFormat, m_dsvFormat, m_uavFormat );
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
                createTextureViewsOnGpu( device, m_width, m_height, getMipMapCountOnCpu() > 1, m_srvFormat, m_rtvFormat, m_dsvFormat, m_uavFormat );
            }
        }
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::unloadFromCpu()
    {
        m_dataMipmaps.clear();
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::unloadFromGpu()
    {
	    m_texture.Reset();

        m_shaderResourceView.Reset();

        m_srViews.clear();
        m_rtViews.clear();
        m_dsViews.clear();
        m_uaViews.clear();

	    m_hasMipmapsOnGpu = false;
    }

    template< typename PixelType >
    bool Texture2D< PixelType >
        ::isInCpuMemory() const
    {
	    return !m_dataMipmaps.empty() && !m_dataMipmaps.front().empty();
    }
    
    template< typename PixelType >
    bool Texture2D< PixelType >
        ::isInGpuMemory() const
    {
        //TODO: Should also check if required views are created.
	    return m_texture != nullptr;
    }

    template< typename PixelType >
    int Texture2D< PixelType >
        ::getMipMapCountOnCpu()  const
    {
        return (int)m_dataMipmaps.size();
    }

    template< typename PixelType >
    int Texture2D< PixelType >
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
    int Texture2D< PixelType >
        ::getBytesPerPixel() const
    {
        return sizeof( PixelType );
    }

    template< typename PixelType >
    int Texture2D< PixelType >
        ::getWidth( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getWidth - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, m_width / (int)pow( 2, (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int Texture2D< PixelType >
        ::getHeight( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getHeight - Incorrect level requested. There is no mipmap with such level." );

	    return std::max( 1, m_height / (int)pow( 2, (int)mipMapLevel ) );
    }

    template< typename PixelType >
    int2 Texture2D< PixelType >
        ::getDimensions( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
            throw std::exception( "Texture2DGeneric::getDimensions - Incorrect level requested. There is no mipmap with such level." );

        return int2( std::max( 1, m_width / (int)pow( 2, (int)mipMapLevel ) ), std::max( 1, m_height / (int)pow( 2, (int)mipMapLevel ) ) );
    }

    template< typename PixelType >
    int Texture2D< PixelType >
        ::getSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getSize - Incorrect level requested. There is no mipmap with such level." );

	    return getLineSize( mipMapLevel ) * getHeight( mipMapLevel );
    }

    template< typename PixelType >
    int Texture2D< PixelType >
        ::getLineSize( unsigned int mipMapLevel = 0 ) const
    {
        if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		    throw std::exception( "Texture2DGeneric::getLineSize - Incorrect level requested. There is no mipmap with such level." );

	    return getWidth( mipMapLevel ) * getBytesPerPixel();
    }

	template< typename PixelType >
	ID3D11ShaderResourceView* Texture2D< PixelType >
		::getShaderResourceView( int mipmapLevel = -1 ) const
	{
		if (mipmapLevel < -1 || mipmapLevel >= (int)m_srViews.size())
			throw std::exception("Texture2DGeneric::getShaderResourceView - Tried to access shader resource view for non-existing mipmap level.");

		if (mipmapLevel == -1)
			return m_shaderResourceView.Get();
		else
			return m_srViews[mipmapLevel].Get();
	}

    template< typename PixelType >
    const std::vector< PixelType >& Texture2D< PixelType >
        ::getData( unsigned int mipMapLevel ) const
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::getData - texture is not in CPU memory." );
        else if ( (int)mipMapLevel >= getMipMapCountOnCpu() )
            throw std::exception( "Texture2DGeneric::getData - Incorrect level requested. There is no mipmap with such level." );

        return m_dataMipmaps.at( mipMapLevel );
    }

    template< typename PixelType >
    std::vector< PixelType >& Texture2D< PixelType >
        ::getData( unsigned int mipMapLevel )
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::getData - texture is not in CPU memory." );
        else if ( (int)mipMapLevel >= getMipMapCountOnCpu() )
            throw std::exception( "Texture2DGeneric::getData - Incorrect level requested. There is no mipmap with such level." );

        return m_dataMipmaps.at( mipMapLevel );
    }

    template< typename PixelType >
    D3D11_MAP Texture2D< PixelType >
        ::getMapForWriteFlag()
    {
        return D3D11_MAP_WRITE_DISCARD;
    }

    template< typename PixelType >
	unsigned int Texture2D< PixelType >::getTextureMiscFlags()
    {
		unsigned int flags = 0;

        if ( supportsMipmapGenerationOnGpu() )
            flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

        return flags;
    }

    template< typename PixelType >
    PixelType Texture2D< PixelType >
        ::sampleBilinearData( float2 texcoords, unsigned int mipMapLevel ) const
    {
        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::sampleData - texture is not in CPU memory." );

        // Wrap texcoords in case they exceed <0 - 1> range.
        texcoords.x = fmod( texcoords.x, 1.0f );
        texcoords.y = fmod( texcoords.y, 1.0f );

        mipMapLevel = std::min( getMipMapCountOnCpu() - 1, (int)mipMapLevel );

        const auto& data       = m_dataMipmaps.at( mipMapLevel );
        const int2  dimensions = getDimensions( mipMapLevel );

        const float2 posF = max( float2::ZERO, texcoords * (float2)dimensions - float2( 0.5f, 0.5f ) );
        const int2   posI( (int2)posF );

        const float2 ratio     = posF - (float2)posI;
        const float2 ratio_inv = float2::ONE - ratio;

        const int dataOffsetTopLeft     = posI.y * dimensions.x + posI.x;
        const int dataOffsetTopRight    = posI.y * dimensions.x + std::min( dimensions.x - 1, posI.x + 1 );
        const int dataOffsetBottomLeft  = std::min( dimensions.y - 1, posI.y + 1 ) * dimensions.x + posI.x;
        const int dataOffsetBottomRight = std::min( dimensions.y - 1, posI.y + 1 ) * dimensions.x + std::min( dimensions.x - 1, posI.x + 1 );
        
        const PixelType result = weightedAverage(
            data[dataOffsetTopLeft], ratio_inv.x * ratio_inv.y,
            data[dataOffsetTopRight], ratio.x * ratio_inv.y, 
            data[dataOffsetBottomLeft], ratio_inv.x * ratio.y,
            data[dataOffsetBottomRight], ratio.x * ratio.y
        );

        return result;
    }

    template<>
    inline unsigned char Texture2D< unsigned char >
        ::average( unsigned char val1, unsigned char val2, unsigned char val3, unsigned char val4 ) const
    {
        // Note: Unsafe cast to smaller type.
        return (unsigned char)( ((int)val1 + (int)val2 + (int)val3 + (int)val4) / 4 );
    }

    template<>
    inline uchar4 Texture2D< uchar4 >
        ::average( uchar4 val1, uchar4 val2, uchar4 val3, uchar4 val4 ) const
    {
        uint4 value = ((uint4)val1 + (uint4)val2 + (uint4)val3 + (uint4)val4) / 4;

        // Note: Unsafe cast to smaller type.
        return uchar4( (unsigned char)value.x, (unsigned char)value.y, (unsigned char)value.z, (unsigned char)value.w );
    }

    template<>
    inline float Texture2D< float >
        ::average( float val1, float val2, float val3, float val4 ) const
    {
        return (val1 + val2 + val3 + val4) / 4.0f;
    }

    template<>
    inline float3 Texture2D< float3 >
        ::average( float3 val1, float3 val2, float3 val3, float3 val4 ) const
    {
        return (val1 + val2 + val3 + val4) / 4.0f;
    }

    template<>
    inline float4 Texture2D< float4 >
        ::average( float4 val1, float4 val2, float4 val3, float4 val4 ) const
    {
        return (val1 + val2 + val3 + val4) / 4.0f;
    }

    template<>
    inline unsigned char Texture2D< unsigned char >
        ::weightedAverage( 
            unsigned char val1, float weight1, 
            unsigned char val2, float weight2, 
            unsigned char val3, float weight3, 
            unsigned char val4, float weight4 ) const
    {
        // Note: Unsafe cast to smaller type.
        return (unsigned char)( 
            (float)val1 * weight1 + 
            (float)val2 * weight2 + 
            (float)val3 * weight3 + 
            (float)val4 * weight4 
        );
    }

    template<>
    inline uchar4 Texture2D< uchar4 >
        ::weightedAverage( 
            uchar4 val1, float weight1, 
            uchar4 val2, float weight2, 
            uchar4 val3, float weight3, 
            uchar4 val4, float weight4 ) const
    {
        uint4 value = (uint4)
            ((float4)val1 * weight1 + 
            (float4)val2 * weight2 + 
            (float4)val3 * weight3 + 
            (float4)val4 * weight4);

        // Note: Unsafe cast to smaller type.
        return uchar4( (unsigned char)value.x, (unsigned char)value.y, (unsigned char)value.z, (unsigned char)value.w );
    }

    template<>
    inline float Texture2D< float >
        ::weightedAverage( 
            float val1, float weight1,
            float val2, float weight2,
            float val3, float weight3,
            float val4, float weight4 ) const
    {
        return 
            val1 * weight1 + 
            val2 * weight2 + 
            val3 * weight3 + 
            val4 * weight4;
    }

    template<>
    inline float3 Texture2D< float3 >
        ::weightedAverage( 
            float3 val1, float weight1,
            float3 val2, float weight2,
            float3 val3, float weight3,
            float3 val4, float weight4 ) const
    {
        return 
            val1 * weight1 + 
            val2 * weight2 + 
            val3 * weight3 + 
            val4 * weight4;
    }

    template<>
    inline float4 Texture2D< float4 >
        ::weightedAverage( 
            float4 val1, float weight1,
            float4 val2, float weight2,
            float4 val3, float weight3,
            float4 val4, float weight4 ) const
    {
        return 
            val1 * weight1 + 
            val2 * weight2 + 
            val3 * weight3 + 
            val4 * weight4;
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::setDataPixel( float2 texcoords, PixelType color, unsigned int mipMapLevel )
    {
        // Clamp texcoords to <0, 1> range.
        texcoords.x = std::max( 0.0f, texcoords.x );
        texcoords.y = std::max( 0.0f, texcoords.y );
        texcoords.x = std::min( 1.0f, texcoords.x );
        texcoords.y = std::min( 1.0f, texcoords.y );

        setDataPixel( 
            (int2)(texcoords * (float2)getDimensions( mipMapLevel )), 
            color,
            mipMapLevel
        );
    }

    template< typename PixelType >
    void Texture2D< PixelType >
        ::setDataPixel( int2 position, PixelType color, unsigned int mipMapLevel )
    {
        const int2 dimensions = getDimensions( mipMapLevel );
        position.x = std::min( dimensions.x - 1, position.x );
        position.y = std::min( dimensions.y - 1, position.y );

        getData( mipMapLevel )[ position.y * dimensions.x + position.x ] = color;
    }

    template< typename PixelType >
    PixelType Texture2D< PixelType >
    ::getDataPixel( float2 texcoords, unsigned int mipMapLevel = 0 )
    {
        // Clamp texcoords to <0, 1> range.
        texcoords.x = std::max( 0.0f, texcoords.x );
        texcoords.y = std::max( 0.0f, texcoords.y );
        texcoords.x = std::min( 1.0f, texcoords.x );
        texcoords.y = std::min( 1.0f, texcoords.y );

        getDataPixel( 
            (int2)(texcoords * (float2)getDimensions( mipMapLevel )), 
            mipMapLevel
        );
    }

    template< typename PixelType >
    PixelType Texture2D< PixelType >
    ::getDataPixel( int2 position, unsigned int mipMapLevel = 0 )
    {
        const int2 dimensions = getDimensions( mipMapLevel );
        position.x = std::min( dimensions.x - 1, position.x );
        position.y = std::min( dimensions.y - 1, position.y );

        return getData( mipMapLevel )[ position.y * dimensions.x + position.x ];
    }
}

