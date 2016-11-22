#include "RaytracingShadowsComputeShader.h"

#include "StringUtil.h"
#include "BlockActor.h"
#include "BlockModel.h"
#include "BlockMesh.h"
#include "Light.h"
#include "SpotLight.h"

#include "MathUtil.h"
#include "TextFile.h"

#include <d3d11.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;
using namespace Engine1;

RaytracingShadowsComputeShader::RaytracingShadowsComputeShader() 
: m_resourceCount( 0 )
{}

RaytracingShadowsComputeShader::~RaytracingShadowsComputeShader() {}

void RaytracingShadowsComputeShader::initialize( ComPtr< ID3D11Device >& device )
{
	{
		// Create constant buffer.
		D3D11_BUFFER_DESC desc;
		desc.Usage				 = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth			 = sizeof( ConstantBuffer );
		desc.BindFlags			 = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags		 = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags           = 0;
		desc.StructureByteStride = 0;

		HRESULT result = device->CreateBuffer( &desc, nullptr, m_constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "RaytracingShadowsComputeShader::initialize - creating constant buffer failed." );
	}

    { // Create point sampler configuration.
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
        HRESULT result = device->CreateSamplerState( &desc, m_pointSamplerState.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "RaytracingShadowsComputeShader::initialize - Failed to create texture sampler state." );
    }

	{ // Create linear sampler configuration.
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
		HRESULT result = device->CreateSamplerState( &desc, m_linearSamplerState.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "RaytracingShadowsComputeShader::initialize - Failed to create texture sampler state." );
	}
}

void RaytracingShadowsComputeShader::setParameters(
	ID3D11DeviceContext& deviceContext,
	const Light& light,
	const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayOriginTexture,
	const Texture2DSpecBind< TexBind::ShaderResource, float4 >& surfaceNormalTexture,
	/*const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,*/
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > preIlluminationTexture,
	const std::vector< std::shared_ptr< const BlockActor > >& actors,
	const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& defaultAlphaTexture,
	const int outputTextureWidth, const int outputTextureHeight )
{
	if ( !m_compiled ) 
        throw std::exception( "RaytracingShadowsComputeShader::setParameters - Shader hasn't been compiled yet." );

    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > shadowMap;

    if ( light.getType() == Light::Type::SpotLight )
        shadowMap = static_cast< const SpotLight& >( light ).getShadowMap();
    
	{ // Set input buffers and textures.
        const int lightRelatedResourceCount = 3;
        const int meshRelatedResourceCount  = 6;

        int passedActorsCount = 0;

        // Note: We initialize resources to nullptr - so we only need to fill non-null resources.
        std::vector< ID3D11ShaderResourceView* > resources;
        resources.resize( lightRelatedResourceCount + s_maxActorCount * meshRelatedResourceCount, nullptr );

        resources[ 0 ] = rayOriginTexture.getShaderResourceView();
        resources[ 1 ] = surfaceNormalTexture.getShaderResourceView();
        resources[ 2 ] = preIlluminationTexture ? preIlluminationTexture->getShaderResourceView() : nullptr;

        for ( int actorIdx = 0; actorIdx < actors.size() && passedActorsCount < s_maxActorCount; ++actorIdx ) 
        {
            const auto& actor = actors[ actorIdx ];
            if ( !actor || !actor->getModel() || !actor->getModel()->getMesh() )
                continue;

            const auto& mesh = *actor->getModel()->getMesh();

            const auto& alphaTexture = actor->getModel()->getAlphaTexturesCount() > 0 ? *actor->getModel()->getAlphaTexture( 0 ).getTexture() : defaultAlphaTexture;

            const int resourceBaseIndex = lightRelatedResourceCount + passedActorsCount * meshRelatedResourceCount;
            
            resources[ resourceBaseIndex ]     = mesh.getVertexBufferResource();
            resources[ resourceBaseIndex + 1 ] = !mesh.getTexcoordBufferResources().empty() ? mesh.getTexcoordBufferResources().front() : nullptr;
            resources[ resourceBaseIndex + 2 ] = mesh.getTriangleBufferResource();
            resources[ resourceBaseIndex + 3 ] = mesh.getBvhTreeBufferNodesShaderResourceView().Get();
            resources[ resourceBaseIndex + 4 ] = mesh.getBvhTreeBufferNodesExtentsShaderResourceView().Get();
            resources[ resourceBaseIndex + 5 ] = alphaTexture.getShaderResourceView();

            ++passedActorsCount;
        }

        // Save resource count to unset them properly afterwards.
        m_resourceCount = (int)resources.size();

        // Note: There is a restriction of 128 slots for Nvidia GTX1080.
		deviceContext.CSSetShaderResources( 0, (int)resources.size(), resources.data() );
	}

	{ // Set constant buffer.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ConstantBuffer* dataPtr;

        HRESULT result = deviceContext.Map( m_constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if ( result < 0 )
            throw std::exception( "RaytracingShadowsComputeShader::setParameters - mapping constant buffer to CPU memory failed." );

        dataPtr = (ConstantBuffer*)mappedResource.pData;

        int passedActorsCount = 0;
        for ( int actorIdx = 0; actorIdx < actors.size() && passedActorsCount < s_maxActorCount; ++actorIdx )
        {
            const auto& actor = actors[ actorIdx ];
            if ( !actor || !actor->getModel() || !actor->getModel()->getMesh() )
                continue;

            const auto& alphaTexture = actor->getModel()->getAlphaTexturesCount() > 0 ? *actor->getModel()->getAlphaTexture( 0 ).getTexture() : defaultAlphaTexture;
            const bool  isOpaque     = alphaTexture.getWidth() * alphaTexture.getHeight() == 1 && alphaTexture.isInCpuMemory() && alphaTexture.getData()[ 0 ] == 255;

            const BoundingBox& boundingBox = actor->getModel()->getMesh()->getBoundingBox();

            dataPtr->localToWorldMatrix[ passedActorsCount ] = float44( actor->getPose() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
            dataPtr->worldToLocalMatrix[ passedActorsCount ] = float44( actor->getPose().getScaleOrientationTranslationInverse() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
            dataPtr->boundingBoxMin[ passedActorsCount ]     = float4( boundingBox.getMin(), 0.0f );
            dataPtr->boundingBoxMax[ passedActorsCount ]     = float4( boundingBox.getMax(), 0.0f );
            dataPtr->isOpaque[ passedActorsCount ]           = isOpaque ? float4::ONE : float4::ZERO;
            dataPtr->outputTextureSize                       = float2( (float)outputTextureWidth, (float)outputTextureHeight );
            dataPtr->lightPosition                           = light.getPosition();
            dataPtr->isPreIlluminationAvailable              = preIlluminationTexture ? 1 : 0;

            ++passedActorsCount;
        }

        dataPtr->actorCount = passedActorsCount;

        if ( shadowMap ) {
            const SpotLight& spotLight = static_cast<const SpotLight&>( light );

            // #TODO: Should be calculated inside a spotlight ( like getViewMatrix() and getProjectionMatrix() ) 
            // to ensure that all classes using a spotlight have the same view and projection matrices.
            dataPtr->shadowMapViewMatrix = MathUtil::lookAtTransformation( spotLight.getPosition() + spotLight.getDirection(), spotLight.getPosition(), float3( 0.0f, 1.0f, 0.0f ) ).getTranspose();
            dataPtr->shadowMapProjectionMatrix = MathUtil::perspectiveProjectionTransformation( spotLight.getConeAngle() * 2.0f, (float)SpotLight::s_shadowMapDimensions.x / (float)SpotLight::s_shadowMapDimensions.y, 0.1f, 100.0f ).getTranspose();
            dataPtr->lightConeMinDot = cos( spotLight.getConeAngle() );
            dataPtr->lightDirection = spotLight.getDirection();
            //dataPtr->shadowMapProjectionMatrix =  MathUtil::orthographicProjectionTransformation( (float)SpotLight::s_shadowMapDimensions.x, (float)SpotLight::s_shadowMapDimensions.y, 0.1f, 50.0f ).getTranspose();
        } else {
            dataPtr->shadowMapViewMatrix = float44::IDENTITY;
            dataPtr->shadowMapProjectionMatrix = float44::IDENTITY;
        }

		deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

		deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
	}

	{ // Set texture samplers.
        ID3D11SamplerState* samplerStates[] = { m_pointSamplerState.Get(), m_linearSamplerState.Get() };
		deviceContext.CSSetSamplers( 0, 2, samplerStates );
	}
}

void RaytracingShadowsComputeShader::unsetParameters( ID3D11DeviceContext& deviceContext )
{ 
	if ( !m_compiled ) 
        throw std::exception( "RaytracingShadowsComputeShader::unsetParameters - Shader hasn't been compiled yet." );

	// Unset buffers and textures.
	std::vector< ID3D11ShaderResourceView* > nullResources;
    nullResources.resize( m_resourceCount, nullptr );

	deviceContext.CSSetShaderResources( 0, (int)nullResources.size(), nullResources.data() );

	// Unset samplers.
	ID3D11SamplerState* nullSamplers[ 2 ] = {};
	deviceContext.CSSetSamplers( 0, 2, nullSamplers );
}
