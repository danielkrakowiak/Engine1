#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <d3d11.h>
#include <wrl.h>

#include "ImageLibrary.h"
#include "Texture2DFileInfo.h"
#include "BinaryFile.h"

namespace Engine1
{
    enum class TexMode : int
    {
        Read = 0,
        Write,
        ReadWrite
    };

    template< typename PixelType >
    class StagingTexture2D
    {
        public:

        TexMode getMode() const;

        int getBytesPerPixel() const;
        int getWidth() const;
        int getHeight() const;
        int getSize() const;
        int getLineSize() const;

        protected:

        StagingTexture2D();
        ~StagingTexture2D() {};

        void initialize( TexMode mode, ID3D11Device& device, const int width, const int height, DXGI_FORMAT textureFormat );

        void createTextureOnCpu( const int width, const int height );
        void createTextureOnGpu( ID3D11Device& device, const int width, const int height, DXGI_FORMAT textureFormat );

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void loadGpuToCpu( ID3D11DeviceContext& deviceContext );

        bool supportsLoadCpuToGpu();
        bool supportsLoadGpuToCpu();

        UINT        getTextureCPUAccessFlags();
        D3D11_MAP   getMapForWriteFlag();
        D3D11_MAP   getMapForReadFlag();

        TexMode m_mode;

        int  m_width;
        int  m_height;

        std::vector< PixelType > m_data;
        Microsoft::WRL::ComPtr< ID3D11Texture2D > m_texture;
        
        DXGI_FORMAT m_textureFormat;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template< typename PixelType >
    StagingTexture2D< PixelType >
        ::StagingTexture2D() :
        m_mode( TexMode::Read ),
        m_width( 0 ),
        m_height( 0 ),
        m_textureFormat( DXGI_FORMAT_UNKNOWN )
    {};

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::initialize( TexMode mode, ID3D11Device& device, const int width, const int height, DXGI_FORMAT textureFormat )
    {
        
        m_mode          = mode;
        m_textureFormat = textureFormat;

        createTextureOnCpu( width, height );
        createTextureOnGpu( device, width, height, textureFormat );

        m_width           = width;
        m_height          = height;
    }

    template< typename PixelType >
    TexMode StagingTexture2D< PixelType >
        ::getMode() const
    {
        return m_mode;
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::createTextureOnCpu( const int width, const int height )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "StagingTexture2D::createTextureOnCpu - given width or height has zero or negative value." );

        m_data.clear();
        m_data.resize( width * height );
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::createTextureOnGpu( ID3D11Device& device, const int width, const int height, DXGI_FORMAT textureFormat )
    {
        if ( width <= 0 || height <= 0)
            throw std::exception( "StagingTexture2D::createTextureOnGpu - given width or height has zero or negative value." );

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = textureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_STAGING;
        desc.BindFlags          = 0;
        desc.CPUAccessFlags     = getTextureCPUAccessFlags();
        desc.MiscFlags          = 0;

        HRESULT result = device.CreateTexture2D( &desc, nullptr, m_texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "StagingTexture2D::createTextureOnGpu - creating texture on GPU failed." );
    }

    template< typename PixelType >
    bool StagingTexture2D< PixelType >
        ::supportsLoadCpuToGpu()
    {
        if ( m_mode == TexMode::Write || m_mode == TexMode::ReadWrite )
            return true;
        else
            return false;
    }

    template< typename PixelType >
    bool StagingTexture2D< PixelType >
        ::supportsLoadGpuToCpu()
    {
        if ( m_mode == TexMode::Read || m_mode == TexMode::ReadWrite )
            return true;
        else
            return false;
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
    {
        if (!m_data || !m_texture)
            throw std::exception( "StagingTexture2D::loadCpuToGpu - texture not initialized." );

        if ( supportsLoadCpuToGpu() ) {
            D3D11_MAPPED_SUBRESOURCE mappedResource;

            HRESULT result = deviceContext.Map( m_texture.Get(), 0, getMapForWriteFlag(), 0, &mappedResource );
            if ( result < 0 ) throw std::exception( "StagingTexture2D::loadCpuToGpu - mapping texture for write failed." );

            std::memcpy( mappedResource.pData, m_data.data(), getSize() );
            mappedResource.RowPitch   = getLineSize();
            mappedResource.DepthPitch = getSize();

            deviceContext.Unmap( m_texture.Get(), 0 );
        } else {
            throw std::exception( "StagingTexture2D::loadCpuToGpu - operation not available for this type (mode) of texture." );
        }
    }

    template< typename PixelType >
    void StagingTexture2D< PixelType >
        ::loadGpuToCpu( ID3D11DeviceContext& deviceContext )
    {
        if (!m_data || !m_texture)
            throw std::exception( "StagingTexture2D::loadCpuToGpu - texture not initialized." );

        if ( !supportsLoadGpuToCpu() )
            throw std::exception( "StagingTexture2D::loadGpuToCpu - operation not available for this type (mode) of texture." );
        
        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = deviceContext.Map( m_texture.Get(), 0, getMapForReadFlag(), 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "StagingTexture2D::loadGpuToCpu - mapping texture for read failed." );

        std::memcpy( m_data.data(), mappedResource.pData, getSize() );
        mappedResource.RowPitch   = getLineSize();
        mappedResource.DepthPitch = getSize();

        deviceContext.Unmap( m_texture.Get(), 0 );
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
	    return getLineSize() * getHeight();
    }

    template< typename PixelType >
    int StagingTexture2D< PixelType >
        ::getLineSize() const
    {
	    return getWidth() * getBytesPerPixel();
    }

    template< typename PixelType >
    UINT StagingTexture2D< PixelType >
        ::getTextureCPUAccessFlags()
    {
        switch ( m_mode ) {
            case TexMode::Read:
                return D3D11_CPU_ACCESS_READ;
            case TexMode::Write:
                return D3D11_CPU_ACCESS_WRITE;
            case TexMode::ReadWrite:
                return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        }

        assert( false );
        return 0; // To avoid warning.
    }

    template< typename PixelType >
    D3D11_MAP StagingTexture2D< PixelType >
        ::getMapForWriteFlag()
    {
        switch ( m_mode ) {
            case TexMode::Read:
                throw std::exception( "Engine1::getMapForWriteFlag - given mode doesn't support writing." );
            case TexMode::Write:
                return D3D11_MAP_WRITE;
            case TexMode::ReadWrite:
                return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
        }

        assert( false );
        return D3D11_MAP_READ; // To avoid warning.
    }

    template< typename PixelType >
    D3D11_MAP StagingTexture2D< PixelType >
        ::getMapForReadFlag()
    {
        switch ( m_mode ) {
            case TexMode::Write:
                throw std::exception( "Engine1::getMapForReadFlag - given usage doesn't support reading." );
            case TexMode::Read:
                return D3D11_MAP_READ;
            case TexMode::ReadWrite:
                return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
        }

        assert( false );
        return D3D11_MAP_READ; // To avoid warning.
    }
}

