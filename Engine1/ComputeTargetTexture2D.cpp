#include "ComputeTargetTexture2D.h"

#include <d3d11.h>

using namespace Engine1;

ComputeTargetTexture2D::ComputeTargetTexture2D( int width, int height, ID3D11Device& device ) :
Texture2D()
{
    // Set texture basic info.
    this->width = width;
    this->height = height;
    this->bytesPerPixel = 16;

    DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    { // Create texture.
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = desc.ArraySize = 1; // Without mipmaps for the creation.
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT result = device.CreateTexture2D( &desc, nullptr, texture.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ComputeTargetTexture2D::ComputeTargetTexture2D - creating texture on GPU failed." );
    }

    { // Create unordered access.
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Format = format;
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;//D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        HRESULT result = device.CreateUnorderedAccessView( texture.Get(), &desc, unorderedAccess.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ComputeTargetTexture2D::ComputeTargetTexture2D - creating unordered access on GPU failed." );
    }

    { // Create shader resource.
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        ZeroMemory( &desc, sizeof(desc) );
        desc.Format = format;
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.MipLevels = 1;

        HRESULT result = device.CreateShaderResourceView( texture.Get(), &desc, shaderResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ComputeTargetTexture2D::ComputeTargetTexture2D - creating shader resource on GPU failed." );
    }
}

ComputeTargetTexture2D::~ComputeTargetTexture2D()
{}

void ComputeTargetTexture2D::clearOnGpu( float4 colorRGBA, ID3D11DeviceContext& deviceContext )
{
    if ( !isInGpuMemory() ) throw std::exception( "ComputeTargetTexture2D::clearOnGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

    deviceContext.ClearUnorderedAccessViewFloat( unorderedAccess.Get(), colorRGBA.getData() );
}

void ComputeTargetTexture2D::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
    if ( !isInCpuMemory() ) throw std::exception( "ComputeTargetTexture2D::loadCpuToGpu - Texture not in RAM." );
    if ( !isInGpuMemory() ) throw std::exception( "ComputeTargetTexture2D::loadCpuToGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

    throw std::exception( "ComputeTargetTexture2D::loadCpuToGpu - Not yet implemented." );
    // #TODO: upload data to GPU.
}

void ComputeTargetTexture2D::unloadFromGpu()
{
    unorderedAccess.Reset();

    Texture2D::unloadFromGpu();
}

bool ComputeTargetTexture2D::isInGpuMemory() const
{
    return unorderedAccess && Texture2D::isInGpuMemory();
}

ID3D11UnorderedAccessView* ComputeTargetTexture2D::getComputeTarget()
{
    if ( !isInGpuMemory() ) throw std::exception( "ComputeTargetTexture2D::getComputeTarget - Failed, because texture is not in GPU memory." );

    return unorderedAccess.Get();
}