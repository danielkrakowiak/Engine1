#pragma once

#include "Texture2D.h"
#include "RenderTarget2D.h"

#include "float4.h"

// #TODO: support for various pixel formats - uint, float etc.

class RenderTargetTexture2D : public Texture2D, public RenderTarget2D 
{
	public:
	RenderTargetTexture2D( int width, int height, ID3D11Device& device );
	~RenderTargetTexture2D();

	void clearOnGpu( float4 color, ID3D11DeviceContext& deviceContext );

	void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
	void unloadFromGpu( );

	bool isInGpuMemory( ) const;

	ID3D11RenderTargetView* getRenderTarget();

	private:

	// Copying textures in not allowed.
	RenderTargetTexture2D( const RenderTargetTexture2D& ) = delete;
	RenderTargetTexture2D& operator=( const RenderTargetTexture2D& ) = delete;
};

