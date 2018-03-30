#include "ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader::ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader() {}

ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader::~ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader() {}

void ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
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
            throw std::exception( "ConvertValueFromScreenSpaceToWorldSpaceComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create linear sampler configuration.
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
        desc.MaxLOD           = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        HRESULT result = device->CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 )
            throw std::exception( "BlurShadowsComputeShader::compileFromFile - Failed to create texture sampler state." );
    }
}

void ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                                                          const float3& cameraPos,
                                                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float3 > > positionTexture,
                                                                          const int2 outputTextureSize )
{
    if ( !m_compiled )
        throw std::exception( "ConvertValueFromScreenSpaceToWorldSpaceComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            positionTexture->getShaderResourceView(),
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 )
        throw std::exception( "ConvertValueFromScreenSpaceToWorldSpaceComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->cameraPos         = cameraPos;
    dataPtr->outputTextureSize = (float2)outputTextureSize;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

void ConvertDistanceFromScreenSpaceToWorldSpaceComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "ConvertValueFromScreenSpaceToWorldSpaceComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 1, nullResources );
}

