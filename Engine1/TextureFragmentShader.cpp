#include "TextureFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureFragmentShader::TextureFragmentShader() {}

TextureFragmentShader::~TextureFragmentShader()
{}


void TextureFragmentShader::initialize( ComPtr< ID3D11Device >& device )
{
	{ // Create sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = /*D3D11_FILTER_MIN_MAG_MIP_POINT;*/D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.MipLODBias       = 0.0f;
		samplerConfiguration.MaxAnisotropy    = 1;
		samplerConfiguration.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
		samplerConfiguration.BorderColor[ 0 ] = 0;
		samplerConfiguration.BorderColor[ 1 ] = 0;
		samplerConfiguration.BorderColor[ 2 ] = 0;
		samplerConfiguration.BorderColor[ 3 ] = 0;
		samplerConfiguration.MinLOD           = 0;
		samplerConfiguration.MaxLOD           = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		HRESULT result = device->CreateSamplerState( &samplerConfiguration, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextureFragmentShader::compileFromFile - Failed to create texture sampler state." );
	}
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& texture, int mipmapLevel )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView( mipmapLevel );

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& texture, int mipmapLevel )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView( mipmapLevel );

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture, int mipmapLevel )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView( mipmapLevel );

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float2 >& texture, int mipmapLevel )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView( mipmapLevel );

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float >& texture, int mipmapLevel )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView( mipmapLevel );

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureFragmentShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
	ID3D11ShaderResourceView* nullResource = nullptr;
	ID3D11SamplerState*       nullSampler = nullptr;

	deviceContext.PSSetShaderResources( 0, 1, &nullResource );
	deviceContext.PSSetSamplers( 0, 1, &nullSampler );
}
