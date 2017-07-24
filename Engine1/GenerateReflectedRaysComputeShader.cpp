#include "GenerateReflectedRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateReflectedRaysComputeShader::GenerateReflectedRaysComputeShader() {}

GenerateReflectedRaysComputeShader::~GenerateReflectedRaysComputeShader() {}

void GenerateReflectedRaysComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    { // Create linear filter sampler configuration
        D3D11_SAMPLER_DESC samplerConfiguration;
        samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
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
        HRESULT result = device->CreateSamplerState( &samplerConfiguration, m_samplerStateLinearFilter.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "GenerateFirstReflectedRaysComputeShader::compileFromFile - Failed to create texture sampler state." );
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
        if ( result < 0 ) throw std::exception( "GenerateFirstReflectedRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void GenerateReflectedRaysComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,
                                                        const int outputTextureWidth, const int outputTextureHeight )
{
    if ( !m_compiled ) throw std::exception( "GenerateReflectedRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 5;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            rayDirectionTexture.getShaderResourceView(),
            rayHitPositionTexture.getShaderResourceView(), 
            rayHitNormalTexture.getShaderResourceView(),
            rayHitRoughnessTexture.getShaderResourceView(),
            contributionTermTexture.getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateReflectedRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->outputTextureSize = float2( (float)outputTextureWidth, (float)outputTextureHeight );

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { m_samplerStateLinearFilter.Get() };
    deviceContext.CSSetSamplers( 0, 1, samplerStates );
}

void GenerateReflectedRaysComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateReflectedRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 5 ] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 5, nullResources );

    ID3D11SamplerState* nullSampler[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSampler );
}