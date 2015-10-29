#include "RenderTargetDepth2D.h"

#include <exception>

#include <d3d11.h>

RenderTargetDepth2D::RenderTargetDepth2D() :
depthRenderTarget( nullptr )
{}


RenderTargetDepth2D::~RenderTargetDepth2D()
{}

void RenderTargetDepth2D::initialize( ID3D11DepthStencilView& renderTarget )
{
	if ( this->depthRenderTarget ) throw std::exception( "RenderTargetDepth2D::initialize - render target has already been initialized." );

	this->depthRenderTarget = &renderTarget;
}

void RenderTargetDepth2D::clearOnGpu( bool clearDepth, float depth, bool clearStencil, unsigned char stencil, ID3D11DeviceContext& deviceContext )
{
	if ( !depthRenderTarget ) throw std::exception( "RenderTargetDepth2D::clearOnGpu - cannot clear, because it hasn't been initialized yet." );

	if ( !clearDepth && !clearStencil )
		throw std::exception( "RenderTargetDepth2D::clearOnGpu - Incorrect arguments - neither depth or stencil buffer are asked to be cleared." );

	UINT flags = 0;
	if ( clearDepth )   flags |= D3D11_CLEAR_DEPTH;
	if ( clearStencil ) flags |= D3D11_CLEAR_STENCIL;

	deviceContext.ClearDepthStencilView( depthRenderTarget.Get(), flags, depth, stencil );
}

ID3D11DepthStencilView* RenderTargetDepth2D::getDepthRenderTarget()
{
	if ( !depthRenderTarget ) throw std::exception( "RenderTargetDepth2D::getDepthRenderTarget - cannot get depth render target, because it hasn't been initialized yet." );

	return depthRenderTarget.Get();
}
