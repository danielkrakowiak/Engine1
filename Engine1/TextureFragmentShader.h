#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

struct ID3D11SamplerState;

class TextureFragmentShader : public FragmentShader
{
	public:

	TextureFragmentShader();
	virtual ~TextureFragmentShader( );

	void compileFromFile( std::string path, ID3D11Device& device );
	void setParameters( ID3D11DeviceContext& deviceContext, const Texture2D& albedoTexture );
	void unsetParameters( ID3D11DeviceContext& deviceContext );

	private:

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	// Copying is not allowed.
	TextureFragmentShader( const TextureFragmentShader& ) = delete;
	TextureFragmentShader& operator=( const TextureFragmentShader& ) = delete;
};

