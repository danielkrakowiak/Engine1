#include "ShadingNoShadowsComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingNoShadowsComputeShader::ShadingNoShadowsComputeShader() {}

ShadingNoShadowsComputeShader::~ShadingNoShadowsComputeShader() {}

void ShadingNoShadowsComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    {
        // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof( ConstantBuffer );
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "ShadingNoShadowsComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void ShadingNoShadowsComputeShader::setParameters( ID3D11DeviceContext3& deviceContext, const float3& cameraPos,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                          const std::vector< std::shared_ptr< Light > >& lights )
{
    if ( !m_compiled ) 
        throw std::exception( "ShadingNoShadowsComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 5;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            positionTexture->getShaderResourceView(),
            albedoTexture->getShaderResourceView(),
            metalnessTexture->getShaderResourceView(),
            roughnessTexture->getShaderResourceView(),
            normalTexture->getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) 
            throw std::exception( "ShadingNoShadowsComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        const unsigned int pointLightCount = std::min( maxPointLightCount, (unsigned int)lights.size() );

        dataPtr->cameraPos       = cameraPos;
        dataPtr->pad1            = 0.0f;
        dataPtr->pointLightCount = std::min( maxPointLightCount, (unsigned int)lights.size() );
        dataPtr->pad2            = float3( 0.0f, 0.0f, 0.0f );

        for ( unsigned int i = 0; i < pointLightCount; ++i )
            dataPtr->pointLightPositions[ i ] = float4( lights[ i ]->getPosition(), 0.0f );

        for ( unsigned int i = pointLightCount; i < maxPointLightCount; ++i )
            dataPtr->pointLightPositions[ i ] = float4::ZERO;

        for ( unsigned int i = 0; i < pointLightCount; ++i )
            dataPtr->pointLightColors[ i ] = float4( lights[ i ]->getColor(), 0.0f );

        for ( unsigned int i = 0; i < pointLightCount; ++i )
            dataPtr->lightLinearAttenuationFactor[ i ] = float4( lights[ i ]->getLinearAttenuationFactor() );

        for ( unsigned int i = 0; i < pointLightCount; ++i )
            dataPtr->lightQuadraticAttenuationFactor[ i ] = float4( lights[ i ]->getQuadraticAttenuationFactor() );

        for ( unsigned int i = pointLightCount; i < maxPointLightCount; ++i )
            dataPtr->pointLightColors[ i ] = float4::ZERO;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }
}

void ShadingNoShadowsComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "ShadingNoShadowsComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 5 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 5, nullResources );
}