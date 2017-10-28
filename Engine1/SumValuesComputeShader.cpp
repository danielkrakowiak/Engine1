#include "SumValuesComputeShader.h"

#include "StringUtil.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

//template < typename PixelType >
//SumValuesComputeShader< PixelType >::SumValuesComputeShader() {}
//
//template < typename PixelType >
//SumValuesComputeShader< PixelType >::~SumValuesComputeShader() {}
//
//template < typename PixelType >
//void SumValuesComputeShader< PixelType >::initialize( ComPtr< ID3D11Device3 >& device )
//{}
//
//template < typename PixelType >
//void SumValuesComputeShader< PixelType >::setParameters( ID3D11DeviceContext3& deviceContext,
//                                           Texture2DSpecBind< TexBind::ShaderResource, PixelType >& texture1,
//                                           Texture2DSpecBind< TexBind::ShaderResource, PixelType >& texture2 )
//{
//    if ( !m_compiled )
//        throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );
//
//    // Set input buffers and textures.
//    const unsigned int resourceCount = 3;
//    ID3D11ShaderResourceView* resources[ resourceCount ] = {
//        texture1.getShaderResourceView(),
//        texture2.getShaderResourceView(),
//        nullptr
//    };
//
//    deviceContext.CSSetShaderResources( 0, resourceCount, resources );
//}
//
//template < typename PixelType >
//void SumValuesComputeShader< PixelType >::setParameters( ID3D11DeviceContext3& deviceContext,
//                                           Texture2DSpecBind< TexBind::ShaderResource, PixelType >& texture1,
//                                           Texture2DSpecBind< TexBind::ShaderResource, PixelType >& texture2,
//                                           Texture2DSpecBind< TexBind::ShaderResource, PixelType >& texture3 )
//{
//    if ( !m_compiled )
//        throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );
//
//    // Set input buffers and textures.
//    const unsigned int resourceCount = 3;
//    ID3D11ShaderResourceView* resources[ resourceCount ] = {
//        texture1.getShaderResourceView(),
//        texture2.getShaderResourceView(),
//        texture3.getShaderResourceView()
//    };
//
//    deviceContext.CSSetShaderResources( 0, resourceCount, resources );
//}
//
//template < typename PixelType >
//void SumValuesComputeShader< PixelType >::unsetParameters( ID3D11DeviceContext3& deviceContext )
//{
//    if ( !m_compiled )
//        throw std::exception( "SumValueComputeShader::unsetParameters - Shader hasn't been compiled yet." );
//
//    // Unset buffers and textures.
//    ID3D11ShaderResourceView* nullResources[ 3 ] = { nullptr };
//    deviceContext.CSSetShaderResources( 0, 3, nullResources );
//}



