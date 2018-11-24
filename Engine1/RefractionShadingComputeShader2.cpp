#include "RefractionShadingComputeShader2.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RefractionShadingComputeShader2::RefractionShadingComputeShader2() {}

RefractionShadingComputeShader2::~RefractionShadingComputeShader2() {}

void RefractionShadingComputeShader2::initialize( ComPtr< ID3D11Device3 >& device )
{
    device;
}

void RefractionShadingComputeShader2::setParameters( ID3D11DeviceContext3& deviceContext,
                                                    const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
                                                    const std::shared_ptr< Texture2D< float4 > > positionTexture,
                                                    const std::shared_ptr< Texture2D< float4 > > normalTexture,
                                                    /*const std::shared_ptr< Texture2D< uchar4 > > depthTexture,*/
                                                    /*const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,*/
                                                    const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
                                                    const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
                                                    const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
                                                    const std::shared_ptr< Texture2D< uchar4 > > previousContributionTermRoughnessTexture )
{
    if ( !m_compiled ) throw std::exception( "RefractionShadingComputeShader2::setParameters - Shader hasn't been compiled yet." );

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

void RefractionShadingComputeShader2::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "RefractionShadingComputeShader2::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 7 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 7, nullResources );
}