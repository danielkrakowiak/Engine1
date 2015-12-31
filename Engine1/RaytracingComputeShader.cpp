#include "RaytracingComputeShader.h"

#include "StringUtil.h"
#include "Texture2D.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RaytracingComputeShader::RaytracingComputeShader() {}

RaytracingComputeShader::~RaytracingComputeShader() {}

void RaytracingComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( compiled ) throw std::exception( "RaytracingComputeShader::compileFromFile - Shader has already been compiled." );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { // Compile the shader.
        ComPtr<ID3D10Blob> errorMessage;

        UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
        flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_PREFER_FLOW_CONTROL;
#endif

        result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str(), nullptr, nullptr, "main", "cs_5_0", flags, 0, nullptr,
                                        shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
        if ( result < 0 ) {
            if ( errorMessage ) {
                std::string compileMessage( (char*)(errorMessage->GetBufferPointer()) );

                throw std::exception( (std::string( "RaytracingComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "RaytracingComputeShader::compileFromFile - Failed to open file." );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "RaytracingComputeShader::compileFromFile - Failed to create shader." );
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

        result = device.CreateBuffer( &desc, nullptr, constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "RaytracingComputeShader::compileFromFile - creating constant buffer failed." );
    }

    this->device = &device;
    this->compiled = true;
    this->shaderId = ++compiledShadersCount;
}

void RaytracingComputeShader::setParameters( ID3D11DeviceContext& deviceContext, const float3 rayOrigin, const Texture2D& rayDirectionsTexture,  const float43& worldMatrix, const float3 boundingBoxMin, const float3 boundingBoxMax )
{
    if ( !compiled ) throw std::exception( "RaytracingComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set texture.
        ID3D11ShaderResourceView* textureResource = rayDirectionsTexture.getShaderResource();
        deviceContext.CSSetShaderResources( 0, 1, &textureResource );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "RaytracingComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->rayOrigin      = rayOrigin;
        dataPtr->worldMatrixInv = float44( worldMatrix ); // This gets transposed when passed to the shader.
        dataPtr->boundingBoxMin = boundingBoxMin;
        dataPtr->boundingBoxMax = boundingBoxMax;

        // Padding.
        dataPtr->pad1 = 0.0f;
        dataPtr->pad2 = 0.0f;
        dataPtr->pad3 = 0.0f;

        deviceContext.Unmap( constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, constantInputBuffer.GetAddressOf() );
    }
}

void RaytracingComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !compiled ) throw std::exception( "RaytracingComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset texture.
    ID3D11ShaderResourceView* nullResource = nullptr;
    deviceContext.CSSetShaderResources( 0, 1, &nullResource );
}