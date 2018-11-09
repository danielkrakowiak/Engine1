#include "BokehBlurComputeShader.h"

#include "Settings.h"

using namespace Engine1;
using namespace Microsoft::WRL;

void BokehBlurComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
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
        if ( result < 0 ) throw std::exception( "BokehBlurComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW         = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MipLODBias       = 0.0f;
        desc.MaxAnisotropy    = 1;
        desc.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
        desc.BorderColor[ 0 ] = 0;
        desc.BorderColor[ 1 ] = 0;
        desc.BorderColor[ 2 ] = 0;
        desc.BorderColor[ 3 ] = 0;
        desc.MinLOD           = 0;
        desc.MaxLOD           = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        HRESULT result = device->CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 )
            throw std::exception( "BokehBlurComputeShader::compileFromFile - Failed to create texture sampler state." );
    }
}

void BokehBlurComputeShader::setParameters( 
    ID3D11DeviceContext3& deviceContext,
    const Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture,
    const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& depthTexture )
{
    if ( !m_compiled ) 
        throw std::exception( "BokehBlurComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 2;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            texture.getShaderResourceView(),
            depthTexture.getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 )
            throw std::exception( "BokehBlurComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->textureSize             = static_cast< float2 >( texture.getDimensions() );
        dataPtr->apertureDiameter        = settings().rendering.postProcess.depthOfField.apertureDiameter;
        dataPtr->cameraFocusDist         = settings().rendering.postProcess.depthOfField.cameraFocusDist;
        dataPtr->focalLength             = settings().rendering.postProcess.depthOfField.focalLength;
        dataPtr->coCMul                  = settings().rendering.postProcess.depthOfField.coCMul;
        dataPtr->maxCoC                  = settings().rendering.postProcess.depthOfField.maxCoC;
        dataPtr->relativeDepthThreshold  = settings().rendering.postProcess.depthOfField.relativeDepthThreshold;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }

    { // Set texture sampler.
        ID3D11SamplerState* samplers[] = { m_samplerState.Get() };
        deviceContext.CSSetSamplers( 0, 1, samplers );
    }
}

void BokehBlurComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "BokehBlurComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 2 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 2, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}