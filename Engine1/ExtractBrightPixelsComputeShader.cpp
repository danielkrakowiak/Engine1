#include "ExtractBrightPixelsComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ExtractBrightPixelsComputeShader::ExtractBrightPixelsComputeShader() {}

ExtractBrightPixelsComputeShader::~ExtractBrightPixelsComputeShader() {}

void ExtractBrightPixelsComputeShader::initialize( ComPtr< ID3D11Device >& device )
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
            throw std::exception( "ExtractBrightPixelsComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void ExtractBrightPixelsComputeShader::setParameters( ID3D11DeviceContext& deviceContext, 
                                                      const Texture2DSpecBind< TexBind::ShaderResource, float4 >& colorTexture,
                                                      const float minBrightness )
{
    if ( !m_compiled ) throw std::exception( "ExtractBrightPixelsComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            colorTexture.getShaderResourceView() 
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) 
        throw std::exception( "ExtractBrightPixelsComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->minBrightness = minBrightness;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

void ExtractBrightPixelsComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) 
        throw std::exception( "ExtractBrightPixelsComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = {};
    deviceContext.CSSetShaderResources( 0, 1, nullResources );
}
