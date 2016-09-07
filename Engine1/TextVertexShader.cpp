#include "TextVertexShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextVertexShader::TextVertexShader() {}

TextVertexShader::~TextVertexShader() {}

void TextVertexShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( m_compiled ) throw std::exception( "TextVertexShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ // Compile the shader.
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DCompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "vs_5_0", flags, 0,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer() ) );

				throw std::exception( ( std::string( "TextVertexShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "TextVertexShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreateVertexShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextVertexShader::compileFromFile - Failed to create shader" );
	}

	{
		const unsigned int inputLayoutCount = 2;
		D3D11_INPUT_ELEMENT_DESC desc[ inputLayoutCount ];
		desc[ 0 ].SemanticName         = "POSITION";
		desc[ 0 ].SemanticIndex        = 0;
		desc[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
		desc[ 0 ].InputSlot            = 0;
		desc[ 0 ].AlignedByteOffset    = 0;
		desc[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		desc[ 0 ].InstanceDataStepRate = 0;

		desc[ 1 ].SemanticName         = "TEXCOORD";
		desc[ 1 ].SemanticIndex        = 0;
		desc[ 1 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
		desc[ 1 ].InputSlot            = 1;
		desc[ 1 ].AlignedByteOffset    = 0;
		desc[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		desc[ 1 ].InstanceDataStepRate = 0;

		// Create the vertex input layout.
		result = device.CreateInputLayout( desc, inputLayoutCount, shaderBuffer->GetBufferPointer(),
										   shaderBuffer->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextVertexShader::compileFromFile - creating input layout failed" );
	}

	{
		// Create constant buffer.
		D3D11_BUFFER_DESC desc;
		desc.Usage               = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth           = sizeof( ConstantBuffer );
		desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags           = 0;
		desc.StructureByteStride = 0;

		result = device.CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "TextVertexShader::compileFromFile - creating constant buffer failed" );
	}

	this->m_device = &device;
	this->m_compiled = true;
	this->m_shaderId = ++compiledShadersCount;
}

void TextVertexShader::setParameters( ID3D11DeviceContext& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix )
{
	if ( !m_compiled ) throw std::exception( "TextVertexShader::setParameters - Shader hasn't been compiled yet" );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBuffer* dataPtr;

	HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( result < 0 ) throw std::exception( "TextVertexShader::setParameters - mapping constant buffer to CPU memory failed" );

	dataPtr = (ConstantBuffer*)mappedResource.pData;

	//transpose from row-major to column-major to fit each column in one register
	dataPtr->world = float44( worldMatrix ).getTranspose();
	dataPtr->view = viewMatrix.getTranspose();
	dataPtr->projection = projectionMatrix.getTranspose();

	deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

	deviceContext.VSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

ID3D11InputLayout& TextVertexShader::getInputLauout( ) const
{
	if ( !m_compiled ) throw std::exception( "TextVertexShader::getInputLauout() - Shader hasn't been compiled yet." );

	return *m_inputLayout.Get( );
}
