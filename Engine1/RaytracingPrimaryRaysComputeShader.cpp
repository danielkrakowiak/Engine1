#include "RaytracingPrimaryRaysComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

RaytracingPrimaryRaysComputeShader::RaytracingPrimaryRaysComputeShader() {}

RaytracingPrimaryRaysComputeShader::~RaytracingPrimaryRaysComputeShader() {}

void RaytracingPrimaryRaysComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    {
        // Create constant buffer.
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth           = sizeof(ConstantBuffer);
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "RaytracingPrimaryRaysComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create sampler configuration.
		D3D11_SAMPLER_DESC desc;
		desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
		if ( result < 0 ) throw std::exception( "RaytracingPrimaryRaysComputeShader::compileFromFile - Failed to create texture sampler state." );
	}

    this->m_device   = device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void RaytracingPrimaryRaysComputeShader::setParameters( ID3D11DeviceContext3& deviceContext, 
                                                        const float3 rayOrigin, 
                                                        const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionsTexture, 
                                                        const BlockMesh& mesh, const float43& worldMatrix, 
                                                        const float3 boundingBoxMin, 
                                                        const float3 boundingBoxMax,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture, const float3& emissiveMul,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture, const float3& albedoMul,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture, const float3& normalMul,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture, const float metalnessMul,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture, const float roughnessMul,
                                                        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture, const float indexOfRefractionMul )
{
    if ( !m_compiled ) throw std::exception( "RaytracingPrimaryRaysComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 14;
        ID3D11ShaderResourceView* resources[ resourceCount ] = { 
            rayDirectionsTexture.getShaderResourceView(), 
            mesh.getVertexBufferResource(), 
            mesh.getNormalBufferResource(), 
            mesh.getTangentBufferResource(),
            !mesh.getTexcoordBufferResources().empty() ? mesh.getTexcoordBufferResources().front() : nullptr,
            mesh.getTriangleBufferResource(),
            mesh.getBvhTreeBufferNodesShaderResourceView().Get(),
            mesh.getBvhTreeBufferNodesExtentsShaderResourceView().Get(),
            emissiveTexture.getShaderResourceView(),
            albedoTexture.getShaderResourceView(),
            normalTexture.getShaderResourceView(),
            metalnessTexture.getShaderResourceView(),
            roughnessTexture.getShaderResourceView(),
            indexOfRefractionTexture.getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 ) throw std::exception( "RaytracingPrimaryRaysComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->rayOrigin            = rayOrigin;
        dataPtr->localToWorldMatrix   = float44( worldMatrix ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
        dataPtr->worldToLocalMatrix   = float44( worldMatrix.getScaleOrientationTranslationInverse() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
        dataPtr->boundingBoxMin       = boundingBoxMin;
        dataPtr->boundingBoxMax       = boundingBoxMax;
        dataPtr->emissiveMul          = emissiveMul;
        dataPtr->albedoMul            = albedoMul;
        dataPtr->normalMul            = normalMul;
        dataPtr->metalnessMul         = metalnessMul;
        dataPtr->roughnessMul         = roughnessMul;
        dataPtr->indexOfRefractionMul = indexOfRefractionMul;

        // Padding.
        dataPtr->pad1 = 0.0f;
        dataPtr->pad2 = 0.0f;
        dataPtr->pad3 = 0.0f;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }

    { // Set texture sampler.
        deviceContext.CSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
    }
}

void RaytracingPrimaryRaysComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled ) throw std::exception( "RaytracingPrimaryRaysComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 14 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    deviceContext.CSSetShaderResources( 0, 14, nullResources );
    
    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}