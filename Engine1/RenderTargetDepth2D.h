#pragma once

#include <wrl.h>

struct ID3D11DepthStencilView;
struct ID3D11DeviceContext;

class RenderTargetDepth2D {

	public:

	RenderTargetDepth2D( );
	~RenderTargetDepth2D( );

	void initialize( ID3D11DepthStencilView& depthRenderTarget );

	virtual void clearOnGpu( bool clearDepth, float depth, bool clearStencil, unsigned char stencil, ID3D11DeviceContext& deviceContext );

	virtual ID3D11DepthStencilView* getDepthRenderTarget( );

	protected:

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthRenderTarget;

	private:

	// Copying is not allowed.
	RenderTargetDepth2D( const RenderTargetDepth2D& ) = delete;
	RenderTargetDepth2D& operator=( const RenderTargetDepth2D& ) = delete;
};

