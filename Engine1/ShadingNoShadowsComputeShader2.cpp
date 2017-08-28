#include "ShadingNoShadowsComputeShader2.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadingNoShadowsComputeShader2::ShadingNoShadowsComputeShader2() {}

ShadingNoShadowsComputeShader2::~ShadingNoShadowsComputeShader2() {}

void ShadingNoShadowsComputeShader2::initialize( ComPtr< ID3D11Device3 >& device )
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
        if ( result < 0 ) 
            throw std::exception( "ShadingNoShadowsComputeShader2::compileFromFile - creating constant buffer failed." );
    }
}

void ShadingNoShadowsComputeShader2::setParameters( ID3D11DeviceContext3& deviceContext,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                                    const std::vector< std::shared_ptr< Light > >& lights )
{
    if ( !m_compiled ) 
        throw std::exception( "ShadingNoShadowsComputeShader2::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 6;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            rayOriginTexture->getShaderResourceView(),
            rayHitPositionTexture->getShaderResourceView(),
            rayHitAlbedoTexture->getShaderResourceView(),
            rayHitMetalnessTexture->getShaderResourceView(),
            rayHitRoughnessTexture->getShaderResourceView(),
            rayHitNormalTexture->getShaderResourceView(),
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) 
            throw std::exception( "ShadingNoShadowsComputeShader2::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        const unsigned int pointLightCount = std::min( maxPointLightCount, (unsigned int)lights.size() );

        dataPtr->pointLightCount = std::min( maxPointLightCount, (unsigned int)lights.size() );

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

void ShadingNoShadowsComputeShader2::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "ShadingNoShadowsComputeShader2::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 6 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 6, nullResources );
}
