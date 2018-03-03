#include "RaytracingShadowsComputeShader.h"

#include "StringUtil.h"
#include "BlockActor.h"
#include "BlockModel.h"
#include "BlockMesh.h"
#include "Light.h"
#include "SpotLight.h"

#include "MathUtil.h"
#include "TextFile.h"
#include "Settings.h"

#include <d3d11_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;
using namespace Engine1;

RaytracingShadowsComputeShader::RaytracingShadowsComputeShader() 
: m_resourceCount( 0 )
{}

RaytracingShadowsComputeShader::~RaytracingShadowsComputeShader() {}

void RaytracingShadowsComputeShader::initialize( ComPtr< ID3D11Device3 >& device )
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
	ID3D11DeviceContext3& deviceContext,
    const float3& cameraPosition,
	const Light& light,
	const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayOriginTexture,
	const Texture2DSpecBind< TexBind::ShaderResource, float4 >& surfaceNormalTexture,
	/*const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,*/
    //const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > preShadowTexture,
	const std::vector< std::shared_ptr< const BlockActor > >& actors,
	const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& defaultAlphaTexture,
	const int outputTextureWidth, const int outputTextureHeight )
{
	if ( !m_compiled ) 
        throw std::exception( "RaytracingShadowsComputeShader::setParameters - Shader hasn't been compiled yet." );

    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > shadowMap;

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
        resources[ 2 ] = nullptr;//preShadowTexture ? preShadowTexture->getShaderResourceView() : nullptr;

        for ( int actorIdx = 0; actorIdx < actors.size() && passedActorsCount < s_maxActorCount; ++actorIdx ) 
        {
            const auto& actor = actors[ actorIdx ];
            if ( !actor || !actor->getModel() || !actor->getModel()->getMesh() )
                continue;

            const auto& mesh = *actor->getModel()->getMesh();

            const auto& alphaTexture = 
                !actor->getModel()->getAlphaTextures().empty() && actor->getModel()->getAlphaTextures()[ 0 ].getTexture()
                ? *actor->getModel()->getAlphaTextures()[ 0 ].getTexture() 
                : defaultAlphaTexture;

            const int resourceBaseIndex = lightRelatedResourceCount + passedActorsCount * meshRelatedResourceCount;
            
            resources[ lightRelatedResourceCount + actorIdx ]                       = mesh.getVertexBufferResource();
            resources[ lightRelatedResourceCount + 1 * s_maxActorCount + actorIdx ] = !mesh.getTexcoordBufferResources().empty() ? mesh.getTexcoordBufferResources().front() : nullptr;
            resources[ lightRelatedResourceCount + 2 * s_maxActorCount + actorIdx ] = mesh.getTriangleBufferResource();
            resources[ lightRelatedResourceCount + 3 * s_maxActorCount + actorIdx ] = mesh.getBvhTreeBufferNodesShaderResourceView().Get();
            resources[ lightRelatedResourceCount + 4 * s_maxActorCount + actorIdx ] = mesh.getBvhTreeBufferNodesExtentsShaderResourceView().Get();
            resources[ lightRelatedResourceCount + 5 * s_maxActorCount + actorIdx ] = alphaTexture.getShaderResourceView();

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

            const auto& alphaTexture = 
                !actor->getModel()->getAlphaTextures().empty() && actor->getModel()->getAlphaTextures()[ 0 ].getTexture()
                ? *actor->getModel()->getAlphaTextures()[ 0 ].getTexture() 
                : defaultAlphaTexture;

            const bool  isOpaque = alphaTexture.getWidth() * alphaTexture.getHeight() == 1 && alphaTexture.isInCpuMemory() && alphaTexture.getData()[ 0 ] == 255;

            const BoundingBox& boundingBox = actor->getModel()->getMesh()->getBoundingBox();

            dataPtr->localToWorldMatrix[ passedActorsCount ] = float44( actor->getPose() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
            dataPtr->worldToLocalMatrix[ passedActorsCount ] = float44( actor->getPose().getScaleOrientationTranslationInverse() ).getTranspose(); // Transpose from row-major to column-major to fit each column in one register.
            dataPtr->boundingBoxMin[ passedActorsCount ]     = float4( boundingBox.getMin(), 0.0f );
            dataPtr->boundingBoxMax[ passedActorsCount ]     = float4( boundingBox.getMax(), 0.0f );
            dataPtr->isOpaque[ passedActorsCount ]           = isOpaque ? float4::ONE : float4::ZERO;
            dataPtr->alphaMul[ passedActorsCount ]           = !actor->getModel()->getAlphaTextures().empty() ? actor->getModel()->getAlphaTextures()[ 0 ].getColorMultiplier() : float4::ONE;

            ++passedActorsCount;
        }

        dataPtr->actorCount = passedActorsCount;

        dataPtr->outputTextureSize    = float2( (float)outputTextureWidth, (float)outputTextureHeight );
        dataPtr->lightPosition        = light.getPosition();
        dataPtr->lightEmitterRadius   = light.getEmitterRadius();
        dataPtr->isPreShadowAvailable = false;//preShadowTexture ? 1 : 0;

        if ( shadowMap ) {
            const SpotLight& spotLight = static_cast<const SpotLight&>( light );

            dataPtr->shadowMapViewMatrix         = spotLight.getShadowMapViewMatrix().getTranspose();
            dataPtr->shadowMapProjectionMatrix   = spotLight.getShadowMapProjectionMatrix().getTranspose();
            dataPtr->lightConeMinDot             = cos( spotLight.getConeAngle() );
            dataPtr->lightDirection              = spotLight.getDirection();
        } else {
            dataPtr->shadowMapViewMatrix         = float44::IDENTITY;
            dataPtr->shadowMapProjectionMatrix   = float44::IDENTITY;
        }

        dataPtr->cameraPos = cameraPosition;

        dataPtr->shadowHardBlurRadiusStartThreshold = 
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold - 
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->shadowHardBlurRadiusEndThreshold = 
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold + 
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->shadowSoftBlurRadiusStartThreshold = 
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold - 
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->shadowSoftBlurRadiusEndThreshold = 
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold + 
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->shadowHardBlurRadiusTransitionWidth = settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusTransitionWidth;
        dataPtr->shadowSoftBlurRadiusTransitionWidth = settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusTransitionWidth;

        dataPtr->distToOccluderHardBlurRadiusStartThreshold =
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold - 
            settings().rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->distToOccluderHardBlurRadiusEndThreshold =
            settings().rendering.shadows.raytracing.layers.hardLayerBlurRadiusThreshold + 
            settings().rendering.shadows.raytracing.layers.distToOccluderHardLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->distToOccluderSoftBlurRadiusStartThreshold =
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold - 
            settings().rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth * 0.5f;

        dataPtr->distToOccluderSoftBlurRadiusEndThreshold =
            settings().rendering.shadows.raytracing.layers.softLayerBlurRadiusThreshold + 
            settings().rendering.shadows.raytracing.layers.distToOccluderSoftLayerBlurRadiusTransitionWidth * 0.5f;

		deviceContext.Unmap( m_constantInputBuffer.Get(), 0 );

		deviceContext.CSSetConstantBuffers( 0, 1, m_constantInputBuffer.GetAddressOf() );
	}

	{ // Set texture samplers.
        ID3D11SamplerState* samplerStates[] = { m_pointSamplerState.Get(), m_linearSamplerState.Get() };
		deviceContext.CSSetSamplers( 0, 2, samplerStates );
	}
}

void RaytracingShadowsComputeShader::unsetParameters( ID3D11DeviceContext3& deviceContext )
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
