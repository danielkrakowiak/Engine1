#include "BlockModelFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

BlockModelFragmentShader::BlockModelFragmentShader() : m_samplerState( nullptr ) {}

BlockModelFragmentShader::~BlockModelFragmentShader()
{}

void BlockModelFragmentShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( m_compiled ) throw std::exception( "BlockModelFragmentShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ // Compile the shader.
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DCompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "ps_5_0", flags, 0,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer() ) );

				throw std::exception( ( std::string( "BlockModelFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "BlockModelFragmentShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockModelFragmentShader::compileFromFile - Failed to create shader" );
	}

	{ // Create sampler configuration.
		D3D11_SAMPLER_DESC desc;
		desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias       = 0.0f;
		desc.MaxAnisotropy    = 1;
		desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
		desc.BorderColor[ 0 ] = 0;
		desc.BorderColor[ 1 ] = 0;
		desc.BorderColor[ 2 ] = 0;
		desc.BorderColor[ 3 ] = 0;
		desc.MinLOD           = 0;
		desc.MaxLOD           = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		result = device.CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockModelFragmentShader::compileFromFile - Failed to create texture sampler state" );
	}

    {
        // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof( ConstantBuffer );
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        result = device.CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockModelFragmentShader::compileFromFile - creating constant buffer failed" );
    }

	this->m_device = &device;
	this->m_compiled = true;
	this->m_shaderId = ++compiledShadersCount;
}

void BlockModelFragmentShader::setParameters( ID3D11DeviceContext& deviceContext, 
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture,
                            const float4& extraEmissive )
{
    const int resourceCount = 7;
	ID3D11ShaderResourceView* textureResource[ resourceCount ] = 
    {
        alphaTexture.getShaderResourceView(),
        emissiveTexture.getShaderResourceView(),
        albedoTexture.getShaderResourceView(),
        normalTexture.getShaderResourceView(),
        metalnessTexture.getShaderResourceView(),
        roughnessTexture.getShaderResourceView(),
        indexOfRefractionTexture.getShaderResourceView()
    };

	deviceContext.PSSetShaderResources( 0, resourceCount, textureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "BlockModelVertexShader::setParameters - mapping constant buffer to CPU memory failed" );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->extraEmissive = extraEmissive;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.PSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

void BlockModelFragmentShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    ID3D11ShaderResourceView* nullResources[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID3D11SamplerState*       nullSampler = nullptr;

	deviceContext.PSSetShaderResources( 0, 7, nullResources );
	deviceContext.PSSetSamplers( 0, 1, &nullSampler );
}
