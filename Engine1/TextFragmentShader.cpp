#include "TextFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextFragmentShader::TextFragmentShader() {}

TextFragmentShader::~TextFragmentShader()
{}

void TextFragmentShader::initialize( ComPtr< ID3D11Device >& device )
{
	{ // Create sampler configuration
		D3D11_SAMPLER_DESC desc;
		desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MipLODBias       = 0.0f;
		desc.MaxAnisotropy    = 1;
		desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
		desc.BorderColor[ 0 ] = 0;
		desc.BorderColor[ 1 ] = 0;
		desc.BorderColor[ 2 ] = 0;
		desc.BorderColor[ 3 ] = 0;
		desc.MinLOD           = 0;
		desc.MaxLOD           = D3D11_FLOAT32_MAX; //0

		// Create the texture sampler state.
		HRESULT result = device->CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextFragmentShader::compileFromFile - Failed to create texture sampler state" );
	}
}

void TextFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, ID3D11ShaderResourceView* characterTextureResource )
{
	deviceContext.PSSetShaderResources( 0, 1, &characterTextureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}