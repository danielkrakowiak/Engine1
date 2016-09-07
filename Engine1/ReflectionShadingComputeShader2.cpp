#include "ReflectionShadingComputeShader2.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ReflectionShadingComputeShader2::ReflectionShadingComputeShader2() {}

ReflectionShadingComputeShader2::~ReflectionShadingComputeShader2() {}

void ReflectionShadingComputeShader2::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( m_compiled ) throw std::exception( "ReflectionShadingComputeShader2::compileFromFile - Shader has already been compiled." );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { // Compile the shader.
        ComPtr<ID3D10Blob> errorMessage;

        UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
        flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_PREFER_FLOW_CONTROL;
#endif

        result = D3DCompileFromFile( StringUtil::widen( path ).c_str(), nullptr, nullptr, "main", "cs_5_0", flags, 0,
                                        shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
        if ( result < 0 ) {
            if ( errorMessage ) {
                std::string compileMessage( (char*)(errorMessage->GetBufferPointer()) );

                throw std::exception( (std::string( "ReflectionShadingComputeShader2::compileFromFile - Compilation failed with errors: " ) + compileMessage).c_str() );
            } else {
                throw std::exception( "ReflectionShadingComputeShader2::compileFromFile - Failed to open file." );
            }
        }

        result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ReflectionShadingComputeShader2::compileFromFile - Failed to create shader." );
    }

    this->m_device = &device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void ReflectionShadingComputeShader2::setParameters( ID3D11DeviceContext& deviceContext,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                                    /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,*/
                                                    /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,*/
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > previousContributionTermRoughnessTexture )
{
    if ( !m_compiled ) throw std::exception( "ReflectionShadingComputeShader2::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 7;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            rayOriginTexture->getShaderResourceView(),
            positionTexture->getShaderResourceView(),
            normalTexture->getShaderResourceView(),
            albedoTexture->getShaderResourceView(),
            metalnessTexture->getShaderResourceView(),
            roughnessTexture->getShaderResourceView(),
            previousContributionTermRoughnessTexture->getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }
}

void ReflectionShadingComputeShader2::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "ReflectionShadingComputeShader2::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 7 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 7, nullResources );
}