#include "GenerateRefractedRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateRefractedRaysComputeShader::GenerateRefractedRaysComputeShader() {}

GenerateRefractedRaysComputeShader::~GenerateRefractedRaysComputeShader() {}

void GenerateRefractedRaysComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::compileFromFile - Shader has already been compiled." );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { // Compile the shader.
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

                throw std::exception( (std::string( "GenerateRefractedRaysComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "GenerateRefractedRaysComputeShader::compileFromFile - Failed to open file." );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "GenerateRefractedRaysComputeShader::compileFromFile - Failed to create shader." );
    }

    this->m_device = &device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void GenerateRefractedRaysComputeShader::setParameters( ID3D11DeviceContext& deviceContext,
                                                                 const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                                                                 const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                                                                 const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                                                                 const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                                                                 const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& refractionTermTexture )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 5;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            rayDirectionTexture.getShaderResourceView(),
            rayHitPositionTexture.getShaderResourceView(), 
            rayHitNormalTexture.getShaderResourceView(),
            rayHitRoughnessTexture.getShaderResourceView(),
            refractionTermTexture.getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }
}

void GenerateRefractedRaysComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 5 ] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 5, nullResources );
}