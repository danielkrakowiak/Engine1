#include "TextureRescaleComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureRescaleComputeShader::TextureRescaleComputeShader() {}

TextureRescaleComputeShader::~TextureRescaleComputeShader() {}

void TextureRescaleComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( m_compiled ) throw std::exception( "TextureRescaleComputeShader::compileFromFile - Shader has already been compiled." );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { // Compile the shader.
        ComPtr<ID3D10Blob> errorMessage;

        UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
        flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_PREFER_FLOW_CONTROL;
#endif

        result = D3DCompileFromFile( StringUtil::widen( path ).c_str(), nullptr, nullptr, "main", "cs_5_0", flags, 0,
                                        shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
        if ( result < 0 ) {
            if ( errorMessage ) {
                std::string compileMessage( (char*)(errorMessage->GetBufferPointer()) );

                throw std::exception( (std::string( "TextureRescaleComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "TextureRescaleComputeShader::compileFromFile - Failed to open file." );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TextureRescaleComputeShader::compileFromFile - Failed to create shader." );
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

        result = device.CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "TextureRescaleComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create sampler configuration.
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
		result = device.CreateSamplerState( &samplerConfiguration, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextureRescaleComputeShader::compileFromFile - Failed to create texture sampler state." );
	}

    this->m_device = &device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void TextureRescaleComputeShader::setParameters( ID3D11DeviceContext& deviceContext,
                                               const Texture2DSpecBind< TexBind::ShaderResource, float4 >& srcTexture,
                                               const int destTextureWidth, const int destTextureHeight,
                                               const unsigned char srcMipmapLevel )
{
    if ( !m_compiled ) throw std::exception( "TextureRescaleComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            srcTexture.getShaderResourceView( (int)srcMipmapLevel )
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "TextureRescaleComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->destTexturePixelSize = float2( 1.0f / (float)destTextureWidth, 1.0f / (float)destTextureHeight );
    dataPtr->pad1                 = float2( 0.0f, 0.0f ); // Padding.
    dataPtr->srcMipmapLevel       = srcMipmapLevel;
    dataPtr->pad2                 = float3( 0.0f, 0.0f, 0.0f );

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    deviceContext.CSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
}

void TextureRescaleComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "TextureRescaleComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 1, nullResources );

    ID3D11SamplerState* nullSampler = nullptr;
    deviceContext.CSSetSamplers( 0, 1, &nullSampler );
}
