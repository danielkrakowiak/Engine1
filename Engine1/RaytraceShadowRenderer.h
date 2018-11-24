#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2DTypes.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
	class Direct3DRendererCore;
	class BlockActor;
	class RaytracingShadowsComputeShader;
	class Light;
    class Camera;

	class RaytraceShadowRenderer
	{
	public:

		RaytraceShadowRenderer(Direct3DRendererCore& rendererCore);
		~RaytraceShadowRenderer();

		void initialize(
			int imageWidth, 
			int imageHeight, 
			Microsoft::WRL::ComPtr< ID3D11Device3 > device,
			Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext
		);

		void generateAndTraceShadowRays(
            const Camera& camera,
            const std::shared_ptr< Light > light,
	        const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
	        const std::shared_ptr< Texture2D< float4 > > surfaceNormalTexture,
	        const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture,
            //const std::shared_ptr< Texture2D< unsigned char > > preIlluminationTexture,]
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > hardShadowRenderTarget,
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > mediumShadowRenderTarget,
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > softShadowRenderTarget,
            std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderHardShadowRenderTarget,
            std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderMediumShadowRenderTarget,
            std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderSoftShadowRenderTarget,
	        const std::vector< std::shared_ptr< BlockActor > >& actors
		);

	private:

		Direct3DRendererCore& m_rendererCore;

		Microsoft::WRL::ComPtr<ID3D11Device3>        m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_deviceContext;

		bool m_initialized;

		// Render targets.
		int m_imageWidth, m_imageHeight;

		// Shaders.
		std::shared_ptr< RaytracingShadowsComputeShader > m_raytracingShadowsComputeShader;

		void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

		// Default textures.
		std::shared_ptr< ImmutableTexture2D< unsigned char > > m_defaultAlphaTexture;

		void createDefaultTextures( ID3D11Device3& device );

		// Copying is not allowed.
		RaytraceShadowRenderer(const RaytraceShadowRenderer&) = delete;
		RaytraceShadowRenderer& operator=(const RaytraceShadowRenderer&) = delete;
	};
}

