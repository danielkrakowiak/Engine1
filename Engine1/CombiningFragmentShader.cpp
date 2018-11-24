#include "CombiningFragmentShader.h"

#include "StringUtil.h"
#include "Settings.h"

#include <d3d11_3.h>
#include <d3dcompiler.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombiningFragmentShader::CombiningFragmentShader() :
m_resourceCount( 0 )
{}

CombiningFragmentShader::~CombiningFragmentShader()
{}


void CombiningFragmentShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    { // Create point filter sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.MipLODBias       = 0.0f;
		samplerConfiguration.MaxAnisotropy    = 1;
		samplerConfiguration.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
		samplerConfiguration.BorderColor[ 0 ] = 0;
		samplerConfiguration.BorderColor[ 1 ] = 0;
		samplerConfiguration.BorderColor[ 2 ] = 0;
		samplerConfiguration.BorderColor[ 3 ] = 0;
		samplerConfiguration.MinLOD           = 0;
		samplerConfiguration.MaxLOD           = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		HRESULT result = device->CreateSamplerState( &samplerConfiguration, m_samplerStatePointFilter.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader::compileFromFile - Failed to create texture sampler state." );
	}

    { // Create linear filter sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerConfiguration.MipLODBias       = 0.0f;
		samplerConfiguration.MaxAnisotropy    = 1;
		samplerConfiguration.ComparisonFunc   = D3D11_COMPARISON_ALWAYS;
		samplerConfiguration.BorderColor[ 0 ] = 0;
		samplerConfiguration.BorderColor[ 1 ] = 0;
		samplerConfiguration.BorderColor[ 2 ] = 0;
		samplerConfiguration.BorderColor[ 3 ] = 0;
		samplerConfiguration.MinLOD           = 0;
		samplerConfiguration.MaxLOD           = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		HRESULT result = device->CreateSamplerState( &samplerConfiguration, m_samplerStateLinearFilter.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader::compileFromFile - Failed to create texture sampler state." );
	}

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
        if ( result < 0 ) throw std::exception( "EdgeDistanceComputeShader::compileFromFile - creating constant buffer failed." );
    }
}

void CombiningFragmentShader::setParameters( ID3D11DeviceContext3& deviceContext, 
                                             const std::shared_ptr< Texture2D< float4 > > srcTexture,
                                             const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture, 
                                             const std::shared_ptr< Texture2D< float4 > > normalTexture,
                                             const std::shared_ptr< Texture2D< float4 > > positionTexture,
                                             const std::shared_ptr< Texture2D< uchar4 > > depthTexture,
                                             const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,
                                             const float normalThreshold,
                                             const float positionThreshold,
                                             const float3 cameraPosition,
                                             const int contributionTextureFilledWidth, const int contributionTextureFilledHeight,
                                             const int srcTextureFilledWidth, const int srcTextureFilledHeight )
{
    { // Set input textures.
        m_resourceCount = 6;
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( m_resourceCount );

        resources.push_back( srcTexture->getShaderResourceView() );
        resources.push_back( contributionTermTexture->getShaderResourceView() );
        resources.push_back( normalTexture->getShaderResourceView() );
        resources.push_back( positionTexture->getShaderResourceView() );
        resources.push_back( depthTexture->getShaderResourceView() );
        resources.push_back( hitDistanceTexture->getShaderResourceView() );

        deviceContext.PSSetShaderResources( 0, (UINT)resources.size(), resources.data() );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "CombiningFragmentShader::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->normalThreshold             = normalThreshold;
    dataPtr->cameraPosition              = cameraPosition;
    dataPtr->imageSize                   = float2( (float)srcTexture->getWidth(), (float)srcTexture->getHeight() );
    dataPtr->contributionTextureFillSize = float2( (float)contributionTextureFilledWidth, (float)contributionTextureFilledHeight );
    dataPtr->srcTextureFillSize          = float2( (float)srcTextureFilledWidth, (float)srcTextureFilledHeight );

    dataPtr->positionDiffMul              = settings().rendering.combining.positionDiffMul;
    dataPtr->normalDiffMul                = settings().rendering.combining.normalDiffMul;
    dataPtr->positionNormalThreshold      = settings().rendering.combining.positionNormalThreshold;
    dataPtr->roughnessMul                 = settings().rendering.reflectionsRefractions.roughnessBlurMul;
    dataPtr->reflectionSamplingQualityInv = 1.0f - settings().rendering.reflectionsRefractions.samplingQuality;

    dataPtr->elongationMul     = settings().rendering.reflectionsRefractions.elongationMul;
    dataPtr->radialBlurEnabled = settings().rendering.reflectionsRefractions.radialBlurEnabled ? 1.0f : 0.0f;

    dataPtr->debugHitDistPower = settings().rendering.reflectionsRefractions.debugHitDistPower;

    deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

    deviceContext.PSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { m_samplerStatePointFilter.Get(), m_samplerStateLinearFilter.Get() };
	deviceContext.PSSetSamplers( 0, 2, samplerStates );
}

void CombiningFragmentShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
{
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( m_resourceCount );
    for ( int i = 0; i < m_resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.PSSetShaderResources( 0, (unsigned int)nullResources.size(), nullResources.data() );

    ID3D11SamplerState* nullSampler[2] = { nullptr, nullptr };
	deviceContext.PSSetSamplers( 0, 2, nullSampler );
}
