#include "ReplaceValueComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ReplaceValueComputeShader::ReplaceValueComputeShader() {}

ReplaceValueComputeShader::~ReplaceValueComputeShader() {}

void ReplaceValueComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
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
        if ( result < 0 ) throw std::exception( "ReplaceValueComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void ReplaceValueComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                               const float replaceFromValue,
                                               const float replaceToValue )
{
    if ( !m_compiled ) 
        throw std::exception( "ReplaceValueComputeShader::setParameters - Shader hasn't been compiled yet." );

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) 
        throw std::exception( "ReplaceValueComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->replaceFromValue = replaceFromValue;
    dataPtr->replaceToValue   = replaceToValue;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

void ReplaceValueComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    deviceContext;

    if ( !m_compiled ) 
        throw std::exception( "ReplaceValueComputeShader::unsetParameters - Shader hasn't been compiled yet." );
}

