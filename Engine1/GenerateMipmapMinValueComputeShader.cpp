#include "GenerateMipmapMinValueComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

GenerateMipmapMinValueComputeShader::GenerateMipmapMinValueComputeShader() {}

GenerateMipmapMinValueComputeShader::~GenerateMipmapMinValueComputeShader() {}

void GenerateMipmapMinValueComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    { // Create sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
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
        if ( result < 0 ) throw std::exception( "GenerateMipmapMinValueComputeShader::compileFromFile - Failed to create texture sampler state." );
    }
}

void GenerateMipmapMinValueComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                                         Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float >& texture,
                                                         const int srcMipLevel )
{
    if ( !m_compiled ) 
        throw std::exception( "GenerateMipmapMinValueComputeShader::setParameters - Shader hasn't been compiled yet." );

    const int mipmapCount = texture.getMipMapCountOnGpu();

    if ( srcMipLevel + 1 >= mipmapCount )
        throw std::exception( "GenerateMipmapMinValueComputeShader::setParameters - Source mipmap level is too high (source or destination mipmap is not exisiting)." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 1;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            texture.getShaderResourceView( srcMipLevel )
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set texture sampler.
        deviceContext.CSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
    }
}

void GenerateMipmapMinValueComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "GenerateMipmapMinValueComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 1 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 1, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}
