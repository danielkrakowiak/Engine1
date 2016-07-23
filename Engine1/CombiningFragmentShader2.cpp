#include "CombiningFragmentShader2.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

#include "uchar4.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombiningFragmentShader2::CombiningFragmentShader2() :
resourceCount( 0 )
{}

CombiningFragmentShader2::~CombiningFragmentShader2()
{}


void CombiningFragmentShader2::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( compiled ) throw std::exception( "CombiningFragmentShader2::compileFromFile - Shader has already been compiled." );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ // Compile the shader.
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "ps_5_0", flags, 0, nullptr,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)( errorMessage->GetBufferPointer() ) );

				throw std::exception( ( std::string( "CombiningFragmentShader2::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			} else {
				throw std::exception( "CombiningFragmentShader2::compileFromFile - Failed to open file." );
			}
		}

		result = device.CreatePixelShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader2::compileFromFile - Failed to create shader." );
	}

    { // Create point filter sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
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
		result = device.CreateSamplerState( &samplerConfiguration, samplerStatePointFilter.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader2::compileFromFile - Failed to create texture sampler state." );
	}

    { // Create linear filter sampler configuration
		D3D11_SAMPLER_DESC samplerConfiguration;
		samplerConfiguration.Filter           = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerConfiguration.AddressU         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressV         = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerConfiguration.AddressW         = D3D11_TEXTURE_ADDRESS_WRAP;
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
		result = device.CreateSamplerState( &samplerConfiguration, samplerStateLinearFilter.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "CombiningFragmentShader2::compileFromFile - Failed to create texture sampler state." );
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

        result = device.CreateBuffer( &desc, nullptr, constantInputBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "EdgeDistanceComputeShader::compileFromFile - creating constant buffer failed." );
    }

	this->device = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}

void CombiningFragmentShader2::setParameters( ID3D11DeviceContext& deviceContext, 
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture, 
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitNormalTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitPositionTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  previousHitDistanceTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > previousHitAlbedoTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > previousHitMetalnessTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > previousHitRoughnessTexture,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousRayOriginTexture,
                                             const float normalThreshold,
                                             const float positionThreshold )
{
    { // Set input textures.
        resourceCount = 10;
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.reserve( resourceCount );

        resources.push_back( srcTexture->getShaderResourceView() );
        resources.push_back( reflectionTermTexture->getShaderResourceView() );
        resources.push_back( previousHitNormalTexture->getShaderResourceView() );
        resources.push_back( previousHitPositionTexture->getShaderResourceView() );
        resources.push_back( previousHitDistanceTexture->getShaderResourceView() );
        resources.push_back( hitDistanceTexture->getShaderResourceView() );
        resources.push_back( previousHitAlbedoTexture->getShaderResourceView() );
        resources.push_back( previousHitMetalnessTexture->getShaderResourceView() );
        resources.push_back( previousHitRoughnessTexture->getShaderResourceView() );
        resources.push_back( previousRayOriginTexture->getShaderResourceView() );

        deviceContext.PSSetShaderResources( 0, (UINT)resources.size(), resources.data() );
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ConstantBuffer* dataPtr;

    HRESULT result = deviceContext.Map( constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    if ( result < 0 ) throw std::exception( "CombiningFragmentShader2::setParameters - mapping constant buffer to CPU memory failed." );

    dataPtr = (ConstantBuffer*)mappedResource.pData;

    dataPtr->normalThreshold         = normalThreshold;
    dataPtr->positionThresholdSquare = positionThreshold * positionThreshold;

    deviceContext.Unmap( constantInputBuffer.Get(), 0 );

    deviceContext.PSSetConstantBuffers( 0, 1, constantInputBuffer.GetAddressOf() );

    ID3D11SamplerState* samplerStates[] = { samplerStatePointFilter.Get(), samplerStateLinearFilter.Get() };
	deviceContext.PSSetSamplers( 0, 2, samplerStates );
}

void CombiningFragmentShader2::unsetParameters( ID3D11DeviceContext& deviceContext )
{
    std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( resourceCount );
    for ( int i = 0; i < resourceCount; ++i )
        nullResources[ i ] = nullptr;

    deviceContext.PSSetShaderResources( 0, (unsigned int)nullResources.size(), nullResources.data() );

    ID3D11SamplerState* nullSampler[2] = { nullptr, nullptr };
	deviceContext.PSSetSamplers( 0, 2, nullSampler );
}
