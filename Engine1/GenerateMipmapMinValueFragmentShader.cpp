#include "GenerateMipmapMinValueFragmentShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateMipmapMinValueFragmentShader::GenerateMipmapMinValueFragmentShader() :
    m_resourceCount( 0 )
{}

GenerateMipmapMinValueFragmentShader::~GenerateMipmapMinValueFragmentShader()
{}


void GenerateMipmapMinValueFragmentShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    { // Create sampler configuration.
        D3D11_SAMPLER_DESC samplerConfiguration;
        samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
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
        if ( result < 0 ) 
            throw std::exception( "GenerateMipmapMinValueFragmentShader::compileFromFile - Failed to create texture sampler state." );
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

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "GenerateMipmapMinValueFragmentShader::compileFromFile - creating constant buffer failed." );
    }
}

void GenerateMipmapMinValueFragmentShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                                          RenderTargetTexture2D< float >& texture,
                                                          const int srcMipLevel )
{
    { // Set input textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            texture.getShaderResourceView( srcMipLevel )
        };

        deviceContext.PSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateMipmapMinValueFragmentShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    // HACK: Multiplied by 1.02f to avoid some issues during sampling. I don't really yet understand the source of the problem.
    // The problem is that values spread more to the left and down than right and up (more to the negative UVs).
    dataPtr->srcPixelSizeInTexcoords = (float2::ONE / (float2)texture.getDimensions( srcMipLevel )) * 1.02f; 
    dataPtr->srcMipmapLevel          = (float)srcMipLevel;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.PSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { m_samplerState.Get() };
    deviceContext.PSSetSamplers( 0, 1, samplerStates );
}

void GenerateMipmapMinValueFragmentShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.PSSetShaderResources( 0, 1, nullResources );

    ID3D11SamplerState* nullSampler[ 1 ] = { nullptr };
    deviceContext.PSSetSamplers( 0, 1, nullSampler );
}

