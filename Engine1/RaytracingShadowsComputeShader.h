#pragma once

#include "ComputeShader.h"

#include <string>
#include <vector>
#include <memory>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
	class BlockActor;
	class Light;

	class RaytracingShadowsComputeShader : public ComputeShader
	{
		public:

        static const int s_maxActorCount = 1;

		RaytracingShadowsComputeShader();
		virtual ~RaytracingShadowsComputeShader();

		void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

		void setParameters( 
			ID3D11DeviceContext& deviceContext,
            const float3& cameraPos,
			const Light& light,
			const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayOriginTexture,
			const Texture2DSpecBind< TexBind::ShaderResource, float4 >& surfaceNormalTexture,
			/*const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,*/
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > preIlluminationTexture,
			const std::vector< std::shared_ptr< const BlockActor > >& actors,
			const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& defaultAlphaTexture,
			const int outputTextureWidth, const int outputTextureHeight );

		void unsetParameters( ID3D11DeviceContext& deviceContext );

		private:

		__declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
		struct ConstantBuffer
		{
            float44      localToWorldMatrix[ s_maxActorCount ]; // Transform from local to world space.
            float44      worldToLocalMatrix[ s_maxActorCount ]; // Transform from world to local space.
            float4       boundingBoxMin[ s_maxActorCount ]; // In local space (4th component is padding).
            float4       boundingBoxMax[ s_maxActorCount ]; // In local space (4th component is padding).
            float4       isOpaque[ s_maxActorCount ]; // 0 - semi transparent, else - fully opaque (2nd, 3rd, 4th components are padding).
            unsigned int actorCount;
            float3       pad1;
            float2       outputTextureSize;
            float2       pad2;
            float3       lightPosition;
            float        pad3;
            float        lightConeMinDot;
            float3       pad4;
            float3       lightDirection;
            float        pad5;
            float        lightEmitterRadius;
            float3       pad6;
            unsigned int isPreShadowAvailable; // 1 - available, 0 - not available.
            float3       pad7;
            float44      shadowMapViewMatrix;
            float44      shadowMapProjectionMatrix;
            float3       cameraPosition;
            float        pad8;
            float4       alphaMul[ s_maxActorCount ]; // (2nd, 3rd, 4th components are padding).
		};

        int m_resourceCount;

        Microsoft::WRL::ComPtr< ID3D11SamplerState > m_pointSamplerState;
		Microsoft::WRL::ComPtr< ID3D11SamplerState > m_linearSamplerState;

		// Copying is not allowed.
		RaytracingShadowsComputeShader( const RaytracingShadowsComputeShader& ) = delete;
		RaytracingShadowsComputeShader& operator=( const RaytracingShadowsComputeShader& ) = delete;
	};
}

