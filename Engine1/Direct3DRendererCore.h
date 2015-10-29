#pragma once

#include <vector>
#include <memory>

struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
class VertexShader;
class FragmentShader;
class RectangleMesh;
class BlockMesh;
class SkeletonMesh;
class FontCharacter;
class RenderTarget2D;
class RenderTargetDepth2D;

class Direct3DRendererCore {

	public:

	Direct3DRendererCore();
	~Direct3DRendererCore();

	void initialize( ID3D11DeviceContext& deviceContext );

	void enableRenderTargets( const std::vector< std::shared_ptr<RenderTarget2D> >& renderTargets, const std::shared_ptr<RenderTargetDepth2D> depthRenderTarget );
	void enableShaders( const VertexShader& vertexShader, const FragmentShader& fragmentShader );
	void enableRasterizerState( ID3D11RasterizerState& rasterizerState );
	void enableDepthStencilState( ID3D11DepthStencilState& depthStencilState );
	void enableBlendState( ID3D11BlendState& blendState );
	void enableDefaultBlendState();
	
	void draw( const RectangleMesh& mesh );
	void draw( const BlockMesh& mesh );
	void draw( const SkeletonMesh& mesh );
	void draw( const FontCharacter& character );

	private:

	ID3D11DeviceContext* deviceContext;

	std::weak_ptr<VertexShader>   currentVertexShader;
	std::weak_ptr<FragmentShader> currentFragmentShader;

	std::vector< std::weak_ptr<RenderTarget2D> > currentRenderTargets;
	std::weak_ptr<RenderTargetDepth2D>           currentDepthRenderTarget;

	ID3D11RasterizerState*   currentRasterizerState;
	ID3D11DepthStencilState* currentDepthStencilState;
	ID3D11BlendState*        currentBlendState;

	// Copying is not allowed.
	Direct3DRendererCore( const Direct3DRendererCore& ) = delete;
	Direct3DRendererCore& operator=( const Direct3DRendererCore& ) = delete;
};

