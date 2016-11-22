#include "GenerateRefractedRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateRefractedRaysComputeShader::GenerateRefractedRaysComputeShader() :
    m_resourceCount( 0 )
{}

GenerateRefractedRaysComputeShader::~GenerateRefractedRaysComputeShader() {}

void GenerateRefractedRaysComputeShader::initialize( ComPtr< ID3D11Device >& device )
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
        if ( result < 0 ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void GenerateRefractedRaysComputeShader::setParameters( ID3D11DeviceContext& deviceContext, const unsigned int refractionLevel,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRefractiveIndexTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,
                                                        const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevRefractiveIndexTexture, // Only makes sense for refraction level >= 2.
                                                        const std::shared_ptr< const Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > currentRefractiveIndexTexture,
                                                        const int outputTextureWidth, const int outputTextureHeight )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        m_resourceCount = 8;
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( m_resourceCount );

        resources.push_back( rayDirectionTexture.getShaderResourceView() );
        resources.push_back( rayHitPositionTexture.getShaderResourceView() );
        resources.push_back( rayHitNormalTexture.getShaderResourceView() );
        resources.push_back( rayHitRoughnessTexture.getShaderResourceView() );
        resources.push_back( rayHitRefractiveIndexTexture.getShaderResourceView() );
        resources.push_back( contributionTermTexture.getShaderResourceView() );

        resources.push_back( prevRefractiveIndexTexture ? prevRefractiveIndexTexture->getShaderResourceView() : nullptr );
        resources.push_back( currentRefractiveIndexTexture ? currentRefractiveIndexTexture->getShaderResourceView() : nullptr );

        deviceContext.CSSetShaderResources( 0, m_resourceCount, resources.data() );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateRefractedRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->refractionLevel   = refractionLevel;
    dataPtr->outputTextureSize = float2( (float)outputTextureWidth, (float)outputTextureHeight );

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { m_samplerStateLinearFilter.Get() };
    deviceContext.CSSetSamplers( 0, 1, samplerStates );
}

void GenerateRefractedRaysComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( m_resourceCount );
    for ( int i = 0; i < m_resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.CSSetShaderResources( 0, (int)nullResources.size(), nullResources.data() );

    ID3D11SamplerState* nullSampler[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSampler );
}