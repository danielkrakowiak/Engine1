#pragma once

#include <d3d11_3.h>
#include <wrl.h>

#include "TextureBase.h"
#include "Direct3DUtil.h"

namespace Engine1
{
    template< typename PixelType >
    class StagingTexture2D : public TextureBase< PixelType >
    {
        public:
        StagingTexture2D( ID3D11Device3& device, const int width, const int height, DXGI_FORMAT textureFormat );
        ~StagingTexture2D();

        Microsoft::WRL::ComPtr< ID3D11Texture2D > getTextureResource();

        void loadGpuToCpu( ID3D11DeviceContext3& deviceContext );

        // Loads a fragment of the texture. 
        // @param coords - left top x, y coordinates (in pixels).
        // @param dimensions - width, height of the fragment to be loaded.
        void loadGpuToCpu( ID3D11DeviceContext3& deviceContext, const int2 coords, const int2 dimensions );

        Asset::Type getType() const;

        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();

        void swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        const Texture2DFileInfo& getFileInfo() const;
        Texture2DFileInfo&       getFileInfo();

        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        const std::vector< PixelType >& getData() const;
        std::vector< PixelType >& getData();

        PixelType getPixel( const int2 coords ) const;
        void setPixel( const PixelType& pixel, const int2 coords );

        int getBytesPerPixel() const;

        int getWidth() const;
        int getHeight() const;
        int getSize() const;

        private:

        void createTextureOnCpu( const int width, const int height );

        void createTextureOnGpu( ID3D11Device3& device, const int width, const int height, DXGI_FORMAT textureFormat );

        int m_width;
        int m_height;

        std::vector< PixelType > m_data;

        Microsoft::WRL::ComPtr< ID3D11Texture2D > m_texture;

        // Temporary fix for base class Asset requiring getFileInfo() to always return some file-info.
        // #TODO: getFileInfo should return std::optional.
        static Texture2DFileInfo s_emptyFileinfo;
    };

    template< typename PixelType >
    Texture2DFileInfo StagingTexture2D< PixelType >::s_emptyFileinfo;

    template< typename PixelType >
    StagingTexture2D< PixelType >
        ::StagingTexture2D( ID3D11Device3& device, const int width, const int height, DXGI_FORMAT textureFormat ) : 
        m_width( width ),
        m_height( height )
    {
        createTextureOnCpu( width, height );
        createTextureOnGpu( device, width, height, textureFormat );
    }

    template< typename PixelType >
    StagingTexture2D< PixelType >
        ::~StagingTexture2D()
    {}

    template< typename PixelType >
    Microsoft::WRL::ComPtr< ID3D11Texture2D > StagingTexture2D< PixelType >
        ::getTextureResource()
    {
        return m_texture;
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::createTextureOnCpu( const int width, const int height )
    {
        if ( width <= 0 || height <= 0 )
            throw std::exception( "StagingTexture::createTextureOnCpu - given width or height has zero or negative value." );

        m_data.clear();
        m_data.resize( width * height );
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::createTextureOnGpu( ID3D11Device3& device, const int width, const int height, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0 )
            throw std::exception( "StagingTexture::createTextureOnGpu - given width or height has zero or negative value." );

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof( desc ) );
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = textureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        HRESULT result = device.CreateTexture2D( &desc, nullptr, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "StagingTexture::createTextureOnGpu - creating texture on GPU failed." );

#if defined(_DEBUG) 
        std::string resourceName = std::string( "StagingTexture2D" );
        Direct3DUtil::setResourceName( *m_texture.Get(), resourceName );
#endif
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::loadGpuToCpu( ID3D11DeviceContext3& deviceContext )
    {
        if ( !m_texture )
            throw std::exception( "StagingTexture2D::loadGpuToCpu - texture not initialized." );

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = deviceContext.Map( m_texture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "StagingTexture2D::loadGpuToCpu - mapping texture for read failed." );

        const int size = getSize();
        m_data.resize( size );
        std::memcpy( m_data.data(), mappedResource.pData, size );

        deviceContext.Unmap( m_texture.Get(), 0 );
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::loadGpuToCpu( ID3D11DeviceContext3& deviceContext, const int2 coords, const int2 dimensions )
    {
        if ( !m_texture ) {
            throw std::exception( "StagingTexture2D::loadGpuToCpu - texture not initialized." );
        }

        if ( coords.x < 0 ||
             coords.y < 0 ||
             dimensions.x < 0 ||
             dimensions.y < 0 ||
             coords.x + dimensions.x > getWidth() ||
             coords.y + dimensions.y > getHeight() )
        {
            throw std::exception( "StagingTexture2D::loadGpuToCpu - given fragment exceeds boundaries of the texture." );
        }

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = deviceContext.Map( m_texture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "StagingTexture2D::loadGpuToCpu - mapping texture for read failed." );

        m_data.resize( getWidth() * getHeight() );

        const int maxX = coords.x + dimensions.x;
        const int maxY = coords.y + dimensions.y;
        
        for ( int cY = coords.y; cY < maxY; ++cY ) {
            const int dataShift = ( cY * getWidth() + coords.x ) * getBytesPerPixel();
            std::memcpy( (char*)m_data.data() + dataShift, (char*)mappedResource.pData + dataShift, dimensions.x * getBytesPerPixel() );
        }

        deviceContext.Unmap( m_texture.Get(), 0 );
    }

    template< typename PixelType >
    Asset::Type StagingTexture2D< PixelType >
        ::getType() const
    {
	    return Asset::Type::StagingTexture2D;
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<const Asset> > StagingTexture2D< PixelType >
        ::getSubAssets( ) const
    {
        return {};
    }

    template< typename PixelType >
    std::vector< std::shared_ptr<Asset> > StagingTexture2D< PixelType >
        ::getSubAssets()
    {
        return {};
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
    {
	    throw std::exception( "StagingTexture2D::swapSubAsset - there are no sub-assets to be swapped." );
    }

    template< typename PixelType >
    const Texture2DFileInfo& StagingTexture2D< PixelType >
        ::getFileInfo() const
    {
        return s_emptyFileinfo;
    }

    template< typename PixelType >
    Texture2DFileInfo& StagingTexture2D< PixelType >
        ::getFileInfo()
    {
        return s_emptyFileinfo;
    }

    template< typename PixelType >
    bool StagingTexture2D< PixelType >
        ::isInCpuMemory() const
    {
	    return !m_data.empty();
    }
    
    template< typename PixelType >
    bool StagingTexture2D< PixelType >
        ::isInGpuMemory() const
    {
	    return m_texture != nullptr;
    }

    template< typename PixelType >
    const std::vector< PixelType >& StagingTexture2D< PixelType >
        ::getData() const
    {
        if ( !isInCpuMemory() )
            throw std::exception( "StagingTexture2D::getData - texture is not in CPU memory." );

        return m_data;
    }

    template< typename PixelType >
    std::vector< PixelType >& StagingTexture2D< PixelType >
        ::getData()
    {
        if ( !isInCpuMemory() )
            throw std::exception( "StagingTexture2D::getData - texture is not in CPU memory." );

        return m_data;
    }

    template< typename PixelType >
    PixelType StagingTexture2D< PixelType >
        ::getPixel( const int2 coords ) const
    {
        if ( coords.x < 0 ||
             coords.y < 0 ||
             coords.x >= getWidth() ||
             coords.y >= getHeight() )
        {
            throw std::exception( "StagingTexture2D::getPixel - coordinates are out of bounds." );
        }

        return m_data[ coords.y * getWidth() + coords.x ];
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::setPixel( const PixelType& pixel, const int2 coords )
    {
        assert(coords.x >= 0 && coords.y >= 0 && coords.x < getWidth() && coords.y < getHeight());

        m_data[ coords.y * getWidth() + coords.x ] = pixel;
    }

    template< typename PixelType >
    int StagingTexture2D< PixelType >
        ::getBytesPerPixel() const
    {
        return sizeof( PixelType );
    }

    template< typename PixelType >
    int StagingTexture2D< PixelType >
        ::getWidth() const
    {
        return m_width;
    }

    template< typename PixelType >
    int StagingTexture2D< PixelType >
        ::getHeight() const
    {
        return m_height;
    }

    template< typename PixelType >
    int StagingTexture2D< PixelType >
        ::getSize() const
    {
        return getWidth() * getHeight() * getBytesPerPixel();
    }
}

