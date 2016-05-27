#include "CombiningFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombiningFragmentShader::CombiningFragmentShader() :
resourceCount( 0 )
{}

CombiningFragmentShader::~CombiningFragmentShader()
{}


void CombiningFragmentShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( compiled ) throw std::exception( "CombiningFragmentShader::compileFromFile - Shader has already been compiled." );

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

				throw std::exception( ( std::string( "CombiningFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "CombiningFragmentShader::compileFromFile - Failed to open file." );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader::compileFromFile - Failed to create shader." );
	}

	{ // Create sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR; //D3D11_FILTER_MIN_MAG_MIP_POINT;
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
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader::compileFromFile - Failed to create texture sampler state." );
	}

	this->device = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}

void CombiningFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, 
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > edgeDistanceTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                                             const std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >& srcTextureUpscaledMipmaps )
{
    { // Set input textures.
        resourceCount = 2 + (int)srcTextureUpscaledMipmaps.size();
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( resourceCount );

        resources.push_back( edgeDistanceTexture->getShaderResourceView() );
        resources.push_back( srcTexture->getShaderResourceView() );
        for ( int i = 0; i < (int)srcTextureUpscaledMipmaps.size(); ++i )
            resources.push_back( srcTextureUpscaledMipmaps[ i ]->getShaderResourceView() );

        deviceContext.PSSetShaderResources( 0, resourceCount, resources.data() );
    }

	deviceContext.PSSetSamplers( 0, 1, samplerState.GetAddressOf() );
}

void CombiningFragmentShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( resourceCount );
    for ( int i = 0; i < resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.CSSetShaderResources( 0, 2, nullResources.data() );

    ID3D11SamplerState* nullSampler = nullptr;
	deviceContext.PSSetSamplers( 0, 1, &nullSampler );
}
