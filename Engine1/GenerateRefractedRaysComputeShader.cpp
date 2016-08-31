#include "GenerateRefractedRaysComputeShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateRefractedRaysComputeShader::GenerateRefractedRaysComputeShader() :
    m_resourceCount( 0 )
{}

GenerateRefractedRaysComputeShader::~GenerateRefractedRaysComputeShader() {}

void GenerateRefractedRaysComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
    if ( m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::compileFromFile - Shader has already been compiled." );

    HRESULT result;
    ComPtr<ID3D10Blob> shaderBuffer;
    { // Compile the shader.
        ComPtr<ID3D10Blob> errorMessage;

        UINT flags = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_SKIP_OPTIMIZATION; // #TODO: Why HLSL compiler crashes without "skip optimization" flag?

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
        if ( result < 0 ) throw std::exception( "GenerateFirstRefractedRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }

    this->m_device = &device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void GenerateRefractedRaysComputeShader::setParameters( ID3D11DeviceContext& deviceContext, const unsigned int refractionLevel,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRefractiveIndexTexture,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,
                                                        const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevRefractiveIndexTexture, // Only makes sense for refraction level >= 2.
                                                        const std::shared_ptr< const Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > currentRefractiveIndexTexture )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        m_resourceCount = 8;
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( m_resourceCount );

        resources.push_back( rayDirectionTexture.getShaderResourceView() );
        resources.push_back( rayHitPositionTexture.getShaderResourceView() );
        resources.push_back( rayHitNormalTexture.getShaderResourceView() );
        resources.push_back( rayHitRoughnessTexture.getShaderResourceView() );
        resources.push_back( rayHitRefractiveIndexTexture.getShaderResourceView() );
        resources.push_back( contributionTermTexture.getShaderResourceView() );

        resources.push_back( prevRefractiveIndexTexture ? prevRefractiveIndexTexture->getShaderResourceView() : nullptr );
        resources.push_back( currentRefractiveIndexTexture ? currentRefractiveIndexTexture->getShaderResourceView() : nullptr );

        deviceContext.CSSetShaderResources( 0, m_resourceCount, resources.data() );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "GenerateRefractedRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->refractionLevel = refractionLevel;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
}

void GenerateRefractedRaysComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateRefractedRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( m_resourceCount );
    for ( int i = 0; i < m_resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.CSSetShaderResources( 0, (int)nullResources.size(), nullResources.data() );
}