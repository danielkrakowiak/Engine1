#include "TextFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextFragmentShader::TextFragmentShader() {}

TextFragmentShader::~TextFragmentShader()
{}

void TextFragmentShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( m_compiled ) throw std::exception( "TextFragmentShader::compileFromFile - Shader has already been compiled" );

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

				throw std::exception( ( std::string( "TextFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "TextFragmentShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextFragmentShader::compileFromFile - Failed to create shader" );
	}

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
		result = device.CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextFragmentShader::compileFromFile - Failed to create texture sampler state" );
	}

	this->m_device = &device;
	this->m_compiled = true;
	this->m_shaderId = ++compiledShadersCount;
}

void TextFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, ID3D11ShaderResourceView* characterTextureResource )
{
	deviceContext.PSSetShaderResources( 0, 1, &characterTextureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}