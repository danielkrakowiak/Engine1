#include "DistanceToOccluderSearchComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"
#include "SpotLight.h"
#include "Settings.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

DistanceToOccluderSearchComputeShader::DistanceToOccluderSearchComputeShader() {}

DistanceToOccluderSearchComputeShader::~DistanceToOccluderSearchComputeShader() {}

void DistanceToOccluderSearchComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
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
            throw std::exception( "DistanceToOccluderSearchComputeShader::compileFromFile - creating constant buffer failed." );
    }

    { // Create linear sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
        HRESULT result = device->CreateSamplerState( &desc, m_linearSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 )
            throw std::exception( "DistanceToOccluderSearchComputeShader::compileFromFile - Failed to create texture sampler state." );
    }

    { // Create point sampler configuration.
        D3D11_SAMPLER_DESC desc;
        desc.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
        HRESULT result = device->CreateSamplerState( &desc, m_pointSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 )
            throw std::exception( "DistanceToOccluderSearchComputeShader::compileFromFile - Failed to create texture sampler state." );
    }
}

void DistanceToOccluderSearchComputeShader::setParameters( 
    ID3D11DeviceContext3& deviceContext, 
    const float3& cameraPos,
    const float positionThreshold,
    const float normalThreshold,
    const float searchRadiusInShadow,
    const float searchStepInShadow,
    const float searchRadiusInLight,
    const float searchStepInLight,
    const int searchMipmapLevel,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< float > > distanceToOccluder,
    const int2 outputImageDimensions,
    const Light& light )
{
    if ( !m_compiled )
        throw std::exception( "DistanceToOccluderSearchComputeShader::setParameters - Shader hasn't been compiled yet." );

    { // Set input buffers and textures.
        const unsigned int resourceCount = 3;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            positionTexture->getShaderResourceView(),
            normalTexture->getShaderResourceView(),
            distanceToOccluder->getShaderResourceView(searchMipmapLevel),
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    { // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 )
            throw std::exception( "DistanceToOccluderSearchComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        dataPtr->cameraPos = cameraPos;
        dataPtr->lightPosition = light.getPosition();
        dataPtr->lightEmitterRadius = light.getEmitterRadius();

        dataPtr->inputTextureSize  = float2( distanceToOccluder->getDimensions( searchMipmapLevel ) );
        dataPtr->outputTextureSize = float2( outputImageDimensions );

        dataPtr->positionThreshold = positionThreshold;
        dataPtr->normalThreshold   = normalThreshold;

        if ( light.getType() == Light::Type::SpotLight ) {
            const SpotLight& spotLight = static_cast<const SpotLight&>( light );

            dataPtr->lightConeMinDot = cos( spotLight.getConeAngle() );
            dataPtr->lightDirection = spotLight.getDirection();
        } else {
            // Note: Pass such values so that every point will be in that fake light cone.
            // #TODO: Could be handled through a different shader for point lights... Could save some calculations - but does it really matter?
            dataPtr->lightConeMinDot = -1.0f;
            dataPtr->lightDirection = float3( 0.0f, 1.0f, 0.0f );
        }

        dataPtr->searchRadiusInShadow = searchRadiusInShadow;
        dataPtr->searchStepInShadow   = searchStepInShadow;
        dataPtr->searchRadiusInLight  = searchRadiusInLight;
        dataPtr->searchStepInLight    = searchStepInLight;

        dataPtr->positionSampleMipmapLevel = (float)settings().rendering.optimization.distToOccluderPositionSampleMipmapLevel;
        dataPtr->normalSampleMipmapLevel   = (float)settings().rendering.optimization.distToOccluderNormalSampleMipmapLevel;

        deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

        deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
    }

    { // Set texture sampler.
        ID3D11SamplerState* samplers[] = { m_linearSamplerState.Get(), m_pointSamplerState.Get() };
        deviceContext.CSSetSamplers( 0, 2, samplers );
    }
}

void DistanceToOccluderSearchComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    if ( !m_compiled )
        throw std::exception( "DistanceToOccluderSearchComputeShader::unsetParameters - Shader hasn't been compiled yet." );

    // Unset buffers and textures.
    ID3D11ShaderResourceView* nullResources[ 3 ] = { nullptr };
    deviceContext.CSSetShaderResources( 0, 3, nullResources );

    // Unset samplers.
    ID3D11SamplerState* nullSamplers[ 2 ] = { nullptr };
    deviceContext.CSSetSamplers( 0, 2, nullSamplers );
}
