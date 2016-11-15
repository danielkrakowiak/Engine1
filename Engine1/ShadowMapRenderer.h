#pragma once

#include <vector>
#include <memory>
#include <wrl.h>

#include "BlockMeshVertexShader.h"
#include "SkeletonMeshVertexShader.h"

#include "uchar4.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
	class Direct3DRendererCore;
	class BlockMesh;
	class SkeletonMesh;
	class SkeletonPose;

	class ShadowMapRenderer
	{

		public:

		ShadowMapRenderer( Direct3DRendererCore& rendererCore );
		~ShadowMapRenderer();

		void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device,
			Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void setRenderTarget( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > renderTarget );
        void createAndSetRenderTarget( const int2 dimensions, ID3D11Device& device );

        void clearRenderTarget( float depth );
        void disableRenderTarget();

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > getRenderTarget();

		void render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const float44& perspectiveMatrix );
		void render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const float44& perspectiveMatrix, const SkeletonPose& poseInSkeletonSpace );

		private:

		Direct3DRendererCore& m_rendererCore;

		Microsoft::WRL::ComPtr<ID3D11Device>        m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

		bool m_initialized;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthStencilState( ID3D11Device& device );

		// Render targets.
		std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > > m_renderTarget;

		// Shaders.
		std::shared_ptr<BlockMeshVertexShader>    m_blockMeshVertexShader;
		std::shared_ptr<SkeletonMeshVertexShader> m_skeletonMeshVertexShader;

		void loadAndCompileShaders( ID3D11Device& device );

		// Copying is not allowed.
		ShadowMapRenderer( const ShadowMapRenderer& ) = delete;
		ShadowMapRenderer& operator=( const ShadowMapRenderer& ) = delete;
	};
}



