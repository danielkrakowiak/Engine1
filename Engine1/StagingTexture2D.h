#pragma once

#include <d3d11_3.h>
#include <wrl.h>

#include "Direct3DUtil.h"

namespace Engine1
{
    template< typename PixelType >
    class StagingTexture2D
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

        PixelType getPixel( const int2 coords ) const;

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
    };

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

