#include "RaytracingShadowsComputeShader.h"

#include "StringUtil.h"
#include "BlockMesh.h"
#include "Light.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;
using namespace Engine1;

RaytracingShadowsComputeShader::RaytracingShadowsComputeShader() {}

RaytracingShadowsComputeShader::~RaytracingShadowsComputeShader() {}

void RaytracingShadowsComputeShader::compileFromFile( std::string path, ID3D11Device& device )
{
	if ( m_compiled ) throw std::exception( "RaytracingShadowsComputeShader::compileFromFile - Shader has already been compiled." );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ // Compile the shader.
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_PREFER_FLOW_CONTROL;
#endif
		// Note: This shader with BVH actually works faster with optimizations skipped. (2.5x faster)
		// TODO: Find out why and modify shader code with optimization attributes (such as [branch] etc).
		flags |= D3D10_SHADER_SKIP_OPTIMIZATION;

		result = D3DCompileFromFile( StringUtil::widen( path ).c_str(), nullptr, nullptr, "main", "cs_5_0", flags, 0,
			shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf() );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage( (char*)errorMessage->GetBufferPointer() );

				throw std::exception( (std::string( "RaytracingShadowsComputeShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str() );
			}
			else {
				throw std::exception( "RaytracingShadowsComputeShader::compileFromFile - Failed to open file." );
			}
		}

		result = device.CreateComputeShader( shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), nullptr, m_shader.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RaytracingShadowsComputeShader::compileFromFile - Failed to create shader." );
	}

	{
		// Create constant buffer.
		D3D11_BUFFER_DESC desc;
		desc.Usage				 = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth			 = sizeof( ConstantBuffer );
		desc.BindFlags			 = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags		 = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags           = 0;
		desc.StructureByteStride = 0;

		result = device.CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RaytracingShadowsComputeShader::compileFromFile - creating constant buffer failed." );
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
		result = device.CreateSamplerState( &desc, m_samplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RaytracingShadowsComputeShader::compileFromFile - Failed to create texture sampler state." );
	}

	this->m_device   = &device;
	this->m_compiled = true;
	this->m_shaderId = ++compiledShadersCount;
}

void RaytracingShadowsComputeShader::setParameters(
	ID3D11DeviceContext& deviceContext,
	const Light& light,
	const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayOriginTexture,
	/*const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,*/
	const BlockMesh& mesh,
	const float43& worldMatrix,
	const float3 boundingBoxMin,
	const float3 boundingBoxMax,
	const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture,
	const int outputTextureWidth, const int outputTextureHeight )
{
	if ( !m_compiled ) throw std::exception( "RaytracingShadowsComputeShader::setParameters - Shader hasn't been compiled yet." );

	{ // Set input buffers and textures.
		const unsigned int resourceCount = 8;
		ID3D11ShaderResourceView* resources[ resourceCount ] = {
			rayOriginTexture.getShaderResourceView(),
			/*contributionTermTexture.getShaderResourceView(),*/
			mesh.getVertexBufferResource(),
			!mesh.getTexcoordBufferResources().empty() ? mesh.getTexcoordBufferResources().front() : nullptr,
			mesh.getTriangleBufferResource(),
			mesh.getBvhTreeBufferNodesShaderResourceView().Get(),
			mesh.getBvhTreeBufferNodesExtentsShaderResourceView().Get(),
			mesh.getBvhTreeBufferTrianglesShaderResourceView().Get(),
			alphaTexture.getShaderResourceView(),
		};

		deviceContext.CSSetShaderResources( 0, resourceCount, resources );
	}

	{ // Set constant buffer.
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ConstantBuffer* dataPtr;

		HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if ( result < 0 ) throw std::exception( "RaytracingShadowsComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

		dataPtr = (ConstantBuffer*)mappedResource.pData;

		dataPtr->localToWorldMatrix = float44( worldMatrix ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
		dataPtr->worldToLocalMatrix = float44( worldMatrix.getScaleOrientationTranslationInverse() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
		dataPtr->boundingBoxMin     = boundingBoxMin;
		dataPtr->boundingBoxMax     = boundingBoxMax;
		dataPtr->outputTextureSize  = float2( (float)outputTextureWidth, (float)outputTextureHeight );
		dataPtr->lightPosition      = light.getPosition();

		deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

		deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
	}

	{ // Set texture sampler.
		deviceContext.CSSetSamplers( 0, 1, m_samplerState.GetAddressOf() );
	}
}

void RaytracingShadowsComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{ 
	if ( !m_compiled ) throw std::exception( "RaytracingShadowsComputeShader::unsetParameters - Shader hasn't been compiled yet." );

	// Unset buffers and textures.
	ID3D11ShaderResourceView* nullResources[ 8 ] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	deviceContext.CSSetShaderResources( 0, 8, nullResources );

	// Unset samplers.
	ID3D11SamplerState* nullSamplers[ 1 ] = { nullptr };
	deviceContext.CSSetSamplers( 0, 1, nullSamplers );
}
