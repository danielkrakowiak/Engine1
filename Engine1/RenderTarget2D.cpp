//#include "RenderTarget2D.h"
//
//#include <exception>
//
//#include <d3d11.h>
//
//using namespace Engine1;
//
//RenderTarget2D::RenderTarget2D()
//{}
//
//
//RenderTarget2D::~RenderTarget2D()
//{}
//
//void RenderTarget2D::initialize( ID3D11RenderTargetView& renderTarget )
//{
//	if ( this->renderTarget ) throw std::exception( "RenderTarget2D::initialize - render target has already been initialized." );
//
//	this->renderTarget = &renderTarget;
//}
//
//void RenderTarget2D::clearOnGpu( float4 colorRGBA, ID3D11DeviceContext& deviceContext )
//{
//	if ( !renderTarget ) throw std::exception( "RenderTarget2D::clearOnGpu - cannot clear, because it hasn't been initialized yet." );
//
//	deviceContext.ClearRenderTargetView( renderTarget.Get(), colorRGBA.getData() );
//}
//
//ID3D11RenderTargetView* RenderTarget2D::getRenderTarget()
//{
//	if ( !renderTarget ) throw std::exception( "RenderTarget2D::getRenderTarget - cannot get render target, because it hasn't been initialized yet." );
//
//	return renderTarget.Get();
//}