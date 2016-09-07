#include "SkeletonMeshFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

SkeletonMeshFragmentShader::SkeletonMeshFragmentShader() {}

SkeletonMeshFragmentShader::~SkeletonMeshFragmentShader() {}

void SkeletonMeshFragmentShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( m_compiled ) throw std::exception( "SkeletonMeshFragmentShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ //compile shader
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DCompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "ps_5_0", flags, 0,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer() ) );

				throw std::exception( ( std::string( "SkeletonMeshFragmentShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "SkeletonMeshFragmentShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMeshFragmentShader::compileFromFile - Failed to create shader" );
	}

	this->m_device = &device;
	this->m_compiled = true;
	this->m_shaderId = ++compiledShadersCount;
}
