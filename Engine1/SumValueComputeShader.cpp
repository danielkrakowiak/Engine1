#include "SumValueComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

SumValueComputeShader::SumValueComputeShader() {}

SumValueComputeShader::~SumValueComputeShader() {}

void SumValueComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{}

void SumValueComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                           Texture2DSpecBind< TexBind::ShaderResource, float >& texture1 )
{
    if ( !m_compiled )
        throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );

    // Set input buffers and textures.
    const unsigned int resourceCount = 2;
    ID3D11ShaderResourceView* resources[ resourceCount ] = {
        texture1.getShaderResourceView(),
        nullptr
    };

    deviceContext.CSSetShaderResources( 0, resourceCount, resources );
}

void SumValueComputeShader::setParameters( ID3D11DeviceContext3& deviceContext,
                                           Texture2DSpecBind< TexBind::ShaderResource, float >& texture1,
                                           Texture2DSpecBind< TexBind::ShaderResource, float >& texture2 )
{
    if ( !m_compiled )
        throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );

    // Set input buffers and textures.
    const unsigned int resourceCount = 2;
    ID3D11ShaderResourceView* resources[ resourceCount ] = {
        texture1.getShaderResourceView(),
        texture2.getShaderResourceView()
    };

    deviceContext.CSSetShaderResources( 0, resourceCount, resources );
}

void SumValueComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "SumValueComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 2 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 2, nullResources );
}



