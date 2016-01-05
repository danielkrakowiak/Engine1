
#include "RenderTargetTexture2D.h"

#include "Direct3DUtil.h"

#include <d3d11.h>

using namespace Engine1;

RenderTargetTexture2D::RenderTargetTexture2D( int width, int height, ID3D11Device& device ) :
Texture2D(),
RenderTarget2D()
{
	// Set texture basic info.
	this->width = width;
	this->height = height;
	this->bytesPerPixel = 16;

	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	{ // Create texture.
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = desc.ArraySize = 1; //without mipmaps for the creation
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // #WARNING: different from tutorial. Is an acceptable value for a render target?

		HRESULT result = device.CreateTexture2D( &desc, nullptr, texture.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RenderTargetTexture2D::RenderTargetTexture2D - creating texture on GPU failed." );
	}

	{ // Create render target.
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Format = format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		HRESULT result = device.CreateRenderTargetView( texture.Get(), &desc, renderTarget.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RenderTargetTexture2D::RenderTargetTexture2D - creating render target on GPU failed." );
	}

	{ // Create shader resource.
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Format = format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = 1;

		HRESULT result = device.CreateShaderResourceView( texture.Get(), &desc, shaderResource.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RenderTargetTexture2D::RenderTargetTexture2D - creating shader resource on GPU failed." );
	}
}


RenderTargetTexture2D::~RenderTargetTexture2D()
{}

void RenderTargetTexture2D::clearOnGpu( float4 colorRGBA, ID3D11DeviceContext& deviceContext )
{
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetTexture2D::clearOnGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

	deviceContext.ClearRenderTargetView( renderTarget.Get(), colorRGBA.getData() );
}

void RenderTargetTexture2D::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
    device;
    deviceContext;

	if ( !isInCpuMemory() ) throw std::exception( "RenderTargetTexture2D::loadCpuToGpu - Texture not in RAM." );
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetTexture2D::loadCpuToGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

	throw std::exception( "RenderTargetTexture2D::loadCpuToGpu - Not yet implemented." );
	// #TODO: upload data to GPU.
}

void RenderTargetTexture2D::unloadFromGpu()
{
	renderTarget.Reset();

	Texture2D::unloadFromGpu();
}

bool RenderTargetTexture2D::isInGpuMemory() const
{
	return renderTarget && Texture2D::isInGpuMemory();
}

ID3D11RenderTargetView* RenderTargetTexture2D::getRenderTarget()
{
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetTexture2D::getRenderTarget - Failed, because texture is not in GPU memory." );

	return renderTarget.Get();
}
