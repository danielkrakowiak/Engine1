#include "TextFragmentShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextFragmentShader::TextFragmentShader() {}

TextFragmentShader::~TextFragmentShader()
{}

void TextFragmentShader::initialize( ComPtr< ID3D11Device3 >& device )
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

    {
        // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof(ConstantBuffer);
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "TextFragmentShader::compileFromFile - creating constant buffer failed." );
    }
}

void TextFragmentShader::setParameters( ID3D11DeviceContext3& deviceContext, ID3D11ShaderResourceView* characterTextureResource, const float4 color )
{
	deviceContext.PSSetShaderResources( 0, 1, &characterTextureResource );
	deviceContext.PSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) {
            throw std::exception( "TextFragmentShader::setParameters - mapping constant buffer to CPU memory failed." );
        }

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->color = color;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.PSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }
}

void TextFragmentShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) 
        throw std::exception( "TextFragmentShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 1, nullResources );
}