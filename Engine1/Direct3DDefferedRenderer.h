#pragma once

#include <vector>
#include <memory>
#include <wrl.h>

#include "BlockMeshVertexShader.h"
#include "BlockMeshFragmentShader.h"
#include "SkeletonMeshVertexShader.h"
#include "SkeletonMeshFragmentShader.h"
#include "BlockModelVertexShader.h"
#include "BlockModelFragmentShader.h"
#include "SkeletonModelVertexShader.h"
#include "SkeletonModelFragmentShader.h"
#include "TextVertexShader.h"
#include "TextFragmentShader.h"

class Direct3DRendererCore;

class RenderTargetTexture2D;
class RenderTargetDepthTexture2D;

class BlockMesh;
class SkeletonMesh;
class BlockModel;
class SkeletonModel;
class SkeletonPose;
class Font;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

class Direct3DDefferedRenderer {

	public:

	enum class RenderTargetType : char
	{
		ALBEDO = 0,
		NORMAL = 1,
		DEBUG_VERTEX_INDEX = 2
	};

	Direct3DDefferedRenderer( Direct3DRendererCore& rendererCore );
	~Direct3DDefferedRenderer();

	void initialize( int imageWidth, int imageHeight, ID3D11Device& device, ID3D11DeviceContext& deviceContext );

	void clearRenderTargets( float4 color, float depth );

	void render( const BlockMesh& mesh,      const float43& worldMatrix, const float44& viewMatrix );
	void render( const SkeletonMesh& mesh,   const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
	void render( const BlockModel& model,    const float43& worldMatrix, const float44& viewMatrix );
	void render( const SkeletonModel& model, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
	void render( const std::string& text, Font& font, float2 position, float4 color );

	std::shared_ptr<RenderTargetTexture2D>      getRenderTarget( RenderTargetType type );
	std::shared_ptr<RenderTargetDepthTexture2D> getDepthRenderTarget();

	private:

	Direct3DRendererCore& rendererCore;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

	bool initialized;

	// Rasterizer states.
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStateForMeshRendering;
	Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStateForTextRendering;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createRasterizerState( ID3D11Device& device );
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthStencilState( ID3D11Device& device );
	Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForMeshRendering( ID3D11Device& device );
	Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTextRendering( ID3D11Device& device );

	// Render targets.
	const int RENDER_TARGETS_COUNT = 3;

	int imageWidth, imageHeight;

	std::vector< std::shared_ptr<RenderTargetTexture2D> > renderTargets;
	std::shared_ptr<RenderTargetDepthTexture2D>           depthRenderTarget;

	void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

	// Projection matrices.
	float44 perspectiveProjectionMatrix;
	float44 orthographicProjectionMatrix;

	// Shaders.
	BlockMeshVertexShader         blockMeshVertexShader;
	BlockMeshFragmentShader       blockMeshFragmentShader;
	SkeletonMeshVertexShader      skeletonMeshVertexShader;
	SkeletonMeshFragmentShader    skeletonMeshFragmentShader;
	BlockModelVertexShader	      blockModelVertexShader;
	BlockModelFragmentShader	  blockModelFragmentShader;
	SkeletonModelVertexShader     skeletonModelVertexShader;
	SkeletonModelFragmentShader   skeletonModelFragmentShader;
	TextVertexShader              textVertexShader;
	TextFragmentShader            textFragmentShader;

	void loadAndCompileShaders( ID3D11Device& device );

	// Copying is not allowed.
	Direct3DDefferedRenderer( const Direct3DDefferedRenderer& ) = delete;
	Direct3DDefferedRenderer& operator=( const Direct3DDefferedRenderer& ) = delete;
};

