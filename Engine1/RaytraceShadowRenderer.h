#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
	class Direct3DRendererCore;
	class BlockActor;
	class RaytracingShadowsComputeShader;
	class Light;

	class RaytraceShadowRenderer
	{
	public:

		RaytraceShadowRenderer(Direct3DRendererCore& rendererCore);
		~RaytraceShadowRenderer();

		void initialize(
			int imageWidth, 
			int imageHeight, 
			Microsoft::WRL::ComPtr< ID3D11Device > device,
			Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext
		);

		void generateAndTraceShadowRays(
            const float3& cameraPos,
			const std::shared_ptr< Light > light,
			const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
			const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > surfaceNormalTexture,
			const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > preIlluminationTexture,
            std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > minIlluminationBlurRadiusTexture,
            std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > maxIlluminationBlurRadiusTexture,
			const std::vector< std::shared_ptr< const BlockActor > >& actors
		);

		std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > getHardIlluminationTexture();
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > getSoftIlluminationTexture();

	private:

		Direct3DRendererCore& m_rendererCore;

		Microsoft::WRL::ComPtr<ID3D11Device>        m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

		bool m_initialized;

		// Render targets.
		int m_imageWidth, m_imageHeight;

		std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > m_hardIlluminationTexture;
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > m_softIlluminationTexture;

		void createComputeTargets(int imageWidth, int imageHeight, ID3D11Device& device);

		// Shaders.
		std::shared_ptr< RaytracingShadowsComputeShader > m_raytracingShadowsComputeShader;

		void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

		// Default textures.
		std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > m_defaultAlphaTexture;

		void createDefaultTextures( ID3D11Device& device );

		// Copying is not allowed.
		RaytraceShadowRenderer(const RaytraceShadowRenderer&) = delete;
		RaytraceShadowRenderer& operator=(const RaytraceShadowRenderer&) = delete;
	};
}

