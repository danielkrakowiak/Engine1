#include "GenerateMipmapMinValueVertexShader.h"

#include "CombiningVertexShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateMipmapMinValueVertexShader::GenerateMipmapMinValueVertexShader() {}

GenerateMipmapMinValueVertexShader::~GenerateMipmapMinValueVertexShader() {}

void GenerateMipmapMinValueVertexShader::initialize( ComPtr< ID3D11Device >& device )
{
    {
        const unsigned int inputLayoutCount = 3;
        D3D11_INPUT_ELEMENT_DESC desc[ inputLayoutCount ];
        desc[ 0 ].SemanticName = "POSITION";
        desc[ 0 ].SemanticIndex = 0;
        desc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        desc[ 0 ].InputSlot = 0;
        desc[ 0 ].AlignedByteOffset = 0;
        desc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc[ 0 ].InstanceDataStepRate = 0;

        desc[ 1 ].SemanticName = "NORMAL";
        desc[ 1 ].SemanticIndex = 0;
        desc[ 1 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        desc[ 1 ].InputSlot = 1;
        desc[ 1 ].AlignedByteOffset = 0;
        desc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc[ 1 ].InstanceDataStepRate = 0;

        desc[ 2 ].SemanticName = "TEXCOORD";
        desc[ 2 ].SemanticIndex = 0;
        desc[ 2 ].Format = DXGI_FORMAT_R32G32_FLOAT;
        desc[ 2 ].InputSlot = 2;
        desc[ 2 ].AlignedByteOffset = 0;
        desc[ 2 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc[ 2 ].InstanceDataStepRate = 0;

        // Create the vertex input layout.
        HRESULT result = device->CreateInputLayout( desc, inputLayoutCount, m_shaderBytecode->data(),
                                                    m_shaderBytecode->size(), m_inputLayout.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "GenerateMipmapMinValueVertexShader::compileFromFile - creating input layout failed" );
    }
}

void GenerateMipmapMinValueVertexShader::setParameters( ID3D11DeviceContext& deviceContext )
{
    deviceContext;
}

ID3D11InputLayout& GenerateMipmapMinValueVertexShader::getInputLauout() const
{
    if ( !m_compiled ) throw std::exception( "GenerateMipmapMinValueVertexShader::getInputLauout() - Shader hasn't been compiled yet." );

    return *m_inputLayout.Get();
}

