#include "GenerateFirstRefractedRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateFirstRefractedRaysComputeShader::GenerateFirstRefractedRaysComputeShader() :
    m_resourceCount( 0 )
{}

GenerateFirstRefractedRaysComputeShader::~GenerateFirstRefractedRaysComputeShader() {}

void GenerateFirstRefractedRaysComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
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
        desc.ByteWidth           = sizeof(ConstantBuffer);
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void GenerateFirstRefractedRaysComputeShader::setParameters( ID3D11DeviceContext3& deviceContext, const float3 cameraPos, const float3 viewportCenter, 
                                                             const float3 viewportUp, const float3 viewportRight, const float2 viewportSize,
                                                             const Texture2DSpecBind< TexBind::ShaderResource, float4 >& positionTexture,
                                                             const Texture2DSpecBind< TexBind::ShaderResource, float4 >& normalTexture,
                                                             const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture,
                                                             const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& refractiveIndexTexture,
                                                             const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,
                                                             const int outputTextureWidth, const int outputTextureHeight )
{
    if ( !m_compiled ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        m_resourceCount = 5 /*+ (int)refractiveIndexTextures.size()*/;
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( m_resourceCount );

        resources.push_back( positionTexture.getShaderResourceView() );
        resources.push_back( normalTexture.getShaderResourceView() );
        resources.push_back( roughnessTexture.getShaderResourceView() );
        resources.push_back( refractiveIndexTexture.getShaderResourceView() );
        resources.push_back( contributionTermTexture.getShaderResourceView() );

        /*for ( int i = 0; i < refractiveIndexTextures .size(); ++i )
            resources.push_back( refractiveIndexTextures[ i ]->getShaderResourceView() );*/

        deviceContext.CSSetShaderResources( 0, m_resourceCount, resources.data() );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->cameraPos          = cameraPos;
    dataPtr->viewportCenter     = viewportCenter;
    dataPtr->viewportUp         = viewportUp;
    dataPtr->viewportRight      = viewportRight;
    dataPtr->viewportSizeHalf   = viewportSize / 2.0f;
    dataPtr->outputTextureSize  = float2( (float)outputTextureWidth, (float)outputTextureHeight );

    // Padding.
    dataPtr->pad1 = 0.0f;
    dataPtr->pad2 = 0.0f;
    dataPtr->pad3 = 0.0f;
    dataPtr->pad4 = 0.0f;
    dataPtr->pad5 = float2( 0.0f, 0.0f );

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { m_samplerStateLinearFilter.Get() };
    deviceContext.CSSetSamplers( 0, 1, samplerStates );
}

void GenerateFirstRefractedRaysComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( m_resourceCount );
    for ( int i = 0; i < m_resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.CSSetShaderResources( 0, (int)nullResources.size(), nullResources.data() );

    ID3D11SamplerState* nullSampler[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSampler );
}