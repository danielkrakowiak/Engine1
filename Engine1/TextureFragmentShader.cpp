#include "TextureFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureFragmentShader::TextureFragmentShader() {}

TextureFragmentShader::~TextureFragmentShader()
{}


void TextureFragmentShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( compiled ) throw std::exception( "TextureFragmentShader::compileFromFile - Shader has already been compiled." );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ // Compile the shader.
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "ps_5_0", flags, 0, nullptr,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer() ) );

				throw std::exception( ( std::string( "TextureFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "TextureFragmentShader::compileFromFile - Failed to open file." );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextureFragmentShader::compileFromFile - Failed to create shader." );
	}

	{ // Create sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
		result = device.CreateSamplerState( &samplerConfiguration, samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextureFragmentShader::compileFromFile - Failed to create texture sampler state." );
	}

	this->device = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& texture )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView();

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView();

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float2 >& texture )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView();

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, samplerState.GetAddressOf() );
}

void TextureFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, const Texture2DSpecBind< TexBind::ShaderResource, float >& texture )
{
	ID3D11ShaderResourceView* textureResource = texture.getShaderResourceView();

	deviceContext.PSSetShaderResources( 0, 1, &textureResource );
	deviceContext.PSSetSamplers( 0, 1, samplerState.GetAddressOf() );
}

void TextureFragmentShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
	ID3D11ShaderResourceView* nullResource = nullptr;
	ID3D11SamplerState*       nullSampler = nullptr;

	deviceContext.PSSetShaderResources( 0, 1, &nullResource );
	deviceContext.PSSetSamplers( 0, 1, &nullSampler );
}
