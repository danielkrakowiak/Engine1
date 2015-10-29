#pragma once

#include "Texture2D.h"
#include "RenderTargetDepth2D.h"

class RenderTargetDepthTexture2D : public Texture2D, public RenderTargetDepth2D 
{

	public:

	RenderTargetDepthTexture2D( int width, int height, ID3D11Device& device );
	~RenderTargetDepthTexture2D();

	void clearOnGpu( bool clearDepth, float depth, bool clearStencil, unsigned char stencil, ID3D11DeviceContext& deviceContext );

	void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
	void unloadFromGpu( );

	bool isInGpuMemory() const;

	void generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext );
	bool hasMipMapsOnGpu()     const;
	int  getMipMapCountOnGpu() const;

	ID3D11DepthStencilView* getDepthRenderTarget();

	private:

	// Copying textures in not allowed.
	RenderTargetDepthTexture2D( const RenderTargetDepthTexture2D& ) = delete;
	RenderTargetDepthTexture2D& operator=( const RenderTargetDepthTexture2D& ) = delete;
};

