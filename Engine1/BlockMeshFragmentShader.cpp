#include "BlockMeshFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using Microsoft::WRL::ComPtr;

BlockMeshFragmentShader::BlockMeshFragmentShader() {}


BlockMeshFragmentShader::~BlockMeshFragmentShader() {}

void BlockMeshFragmentShader::compileFromFile( std::string path, ID3D11Device& device ) {
	if ( compiled ) throw std::exception( "BlockMeshFragmentShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ //compile shader
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "ps_5_0", flags, 0, nullptr,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer( ) ) );

				throw std::exception( ( std::string( "BlockMeshFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str( ) );
			} else {
				throw std::exception( "BlockMeshFragmentShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer( ), shaderBuffer->GetBufferSize( ), nullptr, shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMeshFragmentShader::compileFromFile - Failed to create shader" );
	}

	this->device = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}
