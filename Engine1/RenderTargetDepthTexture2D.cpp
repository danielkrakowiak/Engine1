#include "RenderTargetDepthTexture2D.h"

#include <d3d11.h>

using namespace Engine1;

RenderTargetDepthTexture2D::RenderTargetDepthTexture2D( int width, int height, ID3D11Device& device ) :
Texture2D(),
RenderTargetDepth2D()
{
	// Set texture basic info.
	this->mipmapsOnGpu = false; // Depth/stencil texture doesn't support mipmaps on GPU.
	this->width = width;
	this->height = height;
	this->bytesPerPixel = 4;

	{ // Create texture.
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS; /*DXGI_FORMAT_D24_UNORM_S8_UINT*/; // DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;/*D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;*/ // #WARNING: Tutorial used D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE (v. 10 not 11).
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT result = device.CreateTexture2D( &desc, nullptr, texture.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RenderTargetDepthTexture2D::RenderTargetDepthTexture2D - creating texture on GPU failed." );
	}

	{ // Create depth render target.
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_D32_FLOAT; // Note: Could be also: DXGI_FORMAT_D24_UNORM_S8_UINT (to allow stencil sampling).
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		HRESULT result = device.CreateDepthStencilView( texture.Get(), &desc, depthRenderTarget.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RenderTargetDepthTexture2D::RenderTargetDepthTexture2D - creating render target on GPU failed." );
	}

	{ // Create shader resource.
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;// DXGI_FORMAT_X24_TYPELESS_G8_UINT;// DXGI_FORMAT_R32_FLOAT; // Note: Could be DXGI_FORMAT_X24_TYPELESS_G8_UINT (to allow stencil sampling).
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = 1;

		HRESULT result = device.CreateShaderResourceView( texture.Get(), &desc, shaderResource.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "Texture2D::loadCpuToGpu - creating shader resource on GPU failed." );
	}
}


RenderTargetDepthTexture2D::~RenderTargetDepthTexture2D()
{}

void RenderTargetDepthTexture2D::clearOnGpu( bool clearDepth, float depth, bool clearStencil, unsigned char stencil, ID3D11DeviceContext& deviceContext )
{
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetDepthTexture2D::clearOnGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

	if ( !clearDepth && !clearStencil )
		throw std::exception( "RenderTargetDepthTexture2D::clearOnGpu - Incorrect arguments - neither depth or stencil buffer are asked to be cleared." );

	UINT flags = 0;
	if ( clearDepth )   flags |= D3D11_CLEAR_DEPTH;
	if ( clearStencil ) flags |= D3D11_CLEAR_STENCIL;

	deviceContext.ClearDepthStencilView( depthRenderTarget.Get(), flags, depth, stencil );
}

void RenderTargetDepthTexture2D::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
    device;
    deviceContext;

	if ( !isInCpuMemory() ) throw std::exception( "RenderTargetDepthTexture2D::loadCpuToGpu - Texture not in RAM." );
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetDepthTexture2D::loadCpuToGpu - Texture not in GPU memory. Render target should always be in GPU memory." );

	throw std::exception( "RenderTargetDepthTexture2D::loadCpuToGpu - Not yet implemented." );
	// #TODO: upload data to GPU.
}

void RenderTargetDepthTexture2D::unloadFromGpu()
{
	depthRenderTarget.Reset();

	Texture2D::unloadFromGpu();
}

bool RenderTargetDepthTexture2D::isInGpuMemory() const
{
	return depthRenderTarget && Texture2D::isInGpuMemory();
}

void RenderTargetDepthTexture2D::generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext )
{
    deviceContext;

	throw std::exception( "RenderTargetDepthTexture2D::generateMipMapsOnGpu - generating mipmaps on GPU for depth/stencil texture is not supported." );
}

bool RenderTargetDepthTexture2D::hasMipMapsOnGpu() const
{
	return false;
}

int  RenderTargetDepthTexture2D::getMipMapCountOnGpu() const
{
	return 0;
}

ID3D11DepthStencilView* RenderTargetDepthTexture2D::getDepthRenderTarget()
{
	if ( !isInGpuMemory() ) throw std::exception( "RenderTargetDepthTexture2D::getRenderTarget - Failed, because texture is not in GPU memory." );

	return depthRenderTarget.Get();
}