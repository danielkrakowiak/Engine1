#include "ShadingComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingComputeShader::ShadingComputeShader() {}

ShadingComputeShader::~ShadingComputeShader() {}

void ShadingComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( m_compiled ) throw std::exception( "ShadingComputeShader::compileFromFile - Shader has already been compiled." );

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

                throw std::exception( (std::string( "ShadingComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "ShadingComputeShader::compileFromFile - Failed to open file." );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ShadingComputeShader::compileFromFile - Failed to create shader." );
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
        if ( result < 0 ) throw std::exception( "ShadingComputeShader::compileFromFile - creating constant buffer failed." );
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
        result = device.CreateSamplerState( &desc, m_linearSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "RaytracingSecondaryRaysComputeShader::compileFromFile - Failed to create texture sampler state." );
    }

    this->m_device = &device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void ShadingComputeShader::setParameters( ID3D11DeviceContext& deviceContext, const float3& cameraPos,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture, 
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture, 
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
										  const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > illuminationTexture,
									      const Light& light )
{
    if ( !m_compiled ) throw std::exception( "ShadingComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 6;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            positionTexture->getShaderResourceView(),
            albedoTexture->getShaderResourceView(),
            metalnessTexture->getShaderResourceView(),
            roughnessTexture->getShaderResourceView(),
            normalTexture->getShaderResourceView(),
			illuminationTexture->getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "ShadingComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->cameraPos         = cameraPos;
        dataPtr->pad1		       = 0.0f;
        dataPtr->lightPosition     = float4( light.getPosition(), 0.0f );
        dataPtr->lightColor        = float4( light.getColor(), 0.0f );
        dataPtr->outputTextureSize = float2( (float)positionTexture->getWidth(), (float)positionTexture->getHeight() ); // #TODO: Size should be taken from real output texture, not one of inputs (right now, we are assuming they have the same size).

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }

    { // Set texture sampler.
        deviceContext.CSSetSamplers( 0, 1, m_linearSamplerState.GetAddressOf() );
    }
}

void ShadingComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "ShadingComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 6 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 6, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}