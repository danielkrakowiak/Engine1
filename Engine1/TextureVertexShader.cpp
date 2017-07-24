#include "TextureVertexShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureVertexShader::TextureVertexShader() {}

TextureVertexShader::~TextureVertexShader() {}

void TextureVertexShader::initialize( ComPtr< ID3D11Device3 >& device )
{
	{
		const unsigned int inputLayoutCount = 3;
		D3D11_INPUT_ELEMENT_DESC desc[ inputLayoutCount ];
		desc[ 0 ].SemanticName              = "POSITION";
		desc[ 0 ].SemanticIndex             = 0;
		desc[ 0 ].Format                    = DXGI_FORMAT_R32G32B32_FLOAT;
		desc[ 0 ].InputSlot                 = 0;
		desc[ 0 ].AlignedByteOffset         = 0;
		desc[ 0 ].InputSlotClass            = D3D11_INPUT_PER_VERTEX_DATA;
		desc[ 0 ].InstanceDataStepRate      = 0;

		desc[ 1 ].SemanticName         = "NORMAL";
		desc[ 1 ].SemanticIndex        = 0;
		desc[ 1 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
		desc[ 1 ].InputSlot            = 1;
		desc[ 1 ].AlignedByteOffset    = 0;
		desc[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		desc[ 1 ].InstanceDataStepRate = 0;

		desc[ 2 ].SemanticName         = "TEXCOORD";
		desc[ 2 ].SemanticIndex        = 0;
		desc[ 2 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
		desc[ 2 ].InputSlot            = 2;
		desc[ 2 ].AlignedByteOffset    = 0;
		desc[ 2 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		desc[ 2 ].InstanceDataStepRate = 0;

		// Create the vertex input layout.
		HRESULT result = device->CreateInputLayout( desc, inputLayoutCount, m_shaderBytecode->data(),
										   m_shaderBytecode->size(), m_inputLayout.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "TextureVertexShader::compileFromFile - creating input layout failed" );
	}

	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC desc;
		desc.Usage               = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth           = sizeof( ConstantBuffer ); // Note: has to be multiple of 16.
		desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags           = 0;
		desc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "BlockMeshVertexShader::compileFromFile - creating constant buffer failed" );
	}
}

void TextureVertexShader::setParameters( ID3D11DeviceContext3& deviceContext, float posX, float posY, float width, float height )
{
	if ( !m_compiled ) throw std::exception( "TextureVertexShader::setParameters - Shader hasn't been compiled yet" );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBuffer* dataPtr;

	HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( result < 0 ) throw std::exception( "TextureVertexShader::setParameters - mapping constant buffer to CPU memory failed" );

	dataPtr = (ConstantBuffer*)mappedResource.pData;

	//transpose from row-major to column-major to fit each column in one register
	dataPtr->posX = posX;
	dataPtr->posY = posY;
	dataPtr->width = width;
	dataPtr->height = height;

	deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

	deviceContext.VSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

ID3D11InputLayout& TextureVertexShader::getInputLauout( ) const
{
	if ( !m_compiled ) throw std::exception( "TextureVertexShader::getInputLauout() - Shader hasn't been compiled yet." );

	return *m_inputLayout.Get( );
}
