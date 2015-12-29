#include "GenerateRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateRaysComputeShader::GenerateRaysComputeShader() {}

GenerateRaysComputeShader::~GenerateRaysComputeShader() {}

void GenerateRaysComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( compiled ) throw std::exception( "GenerateRaysComputeShader::compileFromFile - Shader has already been compiled" );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { //compile shader
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

                throw std::exception( (std::string( "GenerateRaysComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "GenerateRaysComputeShader::compileFromFile - Failed to open file" );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "GenerateRaysComputeShader::compileFromFile - Failed to create shader" );
    }

    this->device = &device;
    this->compiled = true;
    this->shaderId = ++compiledShadersCount;
}