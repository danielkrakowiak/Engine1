#include "GenerateRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateRaysComputeShader::GenerateRaysComputeShader() {}

GenerateRaysComputeShader::~GenerateRaysComputeShader() {}

void GenerateRaysComputeShader::initialize( ComPtr< ID3D11Device >& device )
{
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
        if ( result < 0 ) throw std::exception( "GenerateRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void GenerateRaysComputeShader::setParameters( ID3D11DeviceContext& deviceContext, const float3 cameraPos, const float3 viewportCenter, 
                                               const float3 viewportUp, const float3 viewportRight, const float2 viewportSize )
{
    if ( !m_compiled ) throw std::exception( "GenerateRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->cameraPos          = cameraPos;
    dataPtr->viewportCenter     = viewportCenter;
    dataPtr->viewportUp         = viewportUp;
    dataPtr->viewportRight      = viewportRight;
    dataPtr->viewportSizeHalf   = viewportSize / 2.0f;

    // Padding.
    dataPtr->pad1 = 0.0f;
    dataPtr->pad2 = 0.0f;
    dataPtr->pad3 = 0.0f;
    dataPtr->pad4 = 0.0f;
    dataPtr->pad5 = float2( 0.0f, 0.0f );

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}