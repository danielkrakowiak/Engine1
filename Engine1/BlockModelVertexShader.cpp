#include "BlockModelVertexShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

BlockModelVertexShader::BlockModelVertexShader() {}

BlockModelVertexShader::~BlockModelVertexShader() {}

void BlockModelVertexShader::compileFromFile( std::string path, ID3D11Device& device ) {
	if ( compiled ) throw std::exception( "BlockModelVertexShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ //compile shader
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "vs_5_0", flags, 0, nullptr,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer( ) ) );

				throw std::exception( ( std::string( "BlockModelVertexShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str( ) );
			} else {
				throw std::exception( "BlockModelVertexShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreateVertexShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockModelVertexShader::compileFromFile - Failed to create shader" );
	}

	{
		const unsigned int inputLayoutCount = 3;
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[ inputLayoutCount ];
		inputLayoutDesc[ 0 ].SemanticName = "POSITION";
		inputLayoutDesc[ 0 ].SemanticIndex = 0;
		inputLayoutDesc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[ 0 ].InputSlot = 0;
		inputLayoutDesc[ 0 ].AlignedByteOffset = 0;
		inputLayoutDesc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 0 ].InstanceDataStepRate = 0;

		inputLayoutDesc[ 1 ].SemanticName = "NORMAL";
		inputLayoutDesc[ 1 ].SemanticIndex = 0;
		inputLayoutDesc[ 1 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[ 1 ].InputSlot = 1;
		inputLayoutDesc[ 1 ].AlignedByteOffset = 0;
		inputLayoutDesc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 1 ].InstanceDataStepRate = 0;

		inputLayoutDesc[ 2 ].SemanticName = "TEXCOORD";
		inputLayoutDesc[ 2 ].SemanticIndex = 0;
		inputLayoutDesc[ 2 ].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputLayoutDesc[ 2 ].InputSlot = 2;
		inputLayoutDesc[ 2 ].AlignedByteOffset = 0;
		inputLayoutDesc[ 2 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 2 ].InstanceDataStepRate = 0;

		// Create the vertex input layout.
		result = device.CreateInputLayout( inputLayoutDesc, inputLayoutCount, shaderBuffer->GetBufferPointer( ),
										   shaderBuffer->GetBufferSize( ), inputLayout.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockModelVertexShader::compileFromFile - creating input layout failed" );
	}

	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof( ConstantBuffer );
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device.CreateBuffer( &matrixBufferDesc, nullptr, constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMeshVertexShader::compileFromFile - creating constant buffer failed" );
	}

	this->device = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}

void BlockModelVertexShader::setParameters( ID3D11DeviceContext& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix ) {
	if ( !compiled ) throw std::exception( "BlockModelVertexShader::setParameters - Shader hasn't been compiled yet" );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBuffer* dataPtr;

	HRESULT result = deviceContext.Map( constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( result < 0 ) throw std::exception( "BlockModelVertexShader::setParameters - mapping constant buffer to CPU memory failed" );

	dataPtr = (ConstantBuffer*)mappedResource.pData;

	//transpose from row-major to column-major to fit each column in one register
	dataPtr->world = float44( worldMatrix ).getTranspose();
	dataPtr->view = viewMatrix.getTranspose();
	dataPtr->projection = projectionMatrix.getTranspose();

	deviceContext.Unmap( constantInputBuffer.Get(), 0 );

	deviceContext.VSSetConstantBuffers( 0, 1, constantInputBuffer.GetAddressOf() );
}

ID3D11InputLayout& BlockModelVertexShader::getInputLauout( ) const
{
	if ( !compiled ) throw std::exception( "BlockModelVertexShader::getInputLauout() - Shader hasn't been compiled yet." );

	return *inputLayout.Get( );
}
