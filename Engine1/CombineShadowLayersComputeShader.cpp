#include "CombineShadowLayersComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombineShadowLayersComputeShader::CombineShadowLayersComputeShader() {}

CombineShadowLayersComputeShader::~CombineShadowLayersComputeShader() {}

void CombineShadowLayersComputeShader::initialize()
{}

void CombineShadowLayersComputeShader::setParameters( 
    ID3D11DeviceContext3& deviceContext,
    Texture2D< unsigned char >& hardShadow,
    Texture2D< unsigned char >& mediumShadow,
    Texture2D< unsigned char >& softShadow )
{
    if ( !m_compiled )
        throw std::exception( "CombineShadowLayersComputeShader::setParameters - Shader hasn't been compiled yet." );

    // Set input buffers and textures.
    const unsigned int resourceCount = 3;
    ID3D11ShaderResourceView* resources[ resourceCount ] = {
        hardShadow.getShaderResourceView(),
        mediumShadow.getShaderResourceView(),
        softShadow.getShaderResourceView()
    };

    deviceContext.CSSetShaderResources( 0, resourceCount, resources );
}

void CombineShadowLayersComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "CombineShadowLayersComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 3 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 3, nullResources );
}