#include "ShadowMapRenderer.h"

#include "Direct3DRendererCore.h"

#include "MathUtil.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadowMapRenderer::ShadowMapRenderer( Direct3DRendererCore& rendererCore ) :
m_rendererCore( rendererCore ),
m_initialized( false ),
m_imageWidth( 0 ),
m_imageHeight( 0 ),
m_blockMeshVertexShader( std::make_shared<BlockMeshVertexShader>() ),
m_skeletonMeshVertexShader( std::make_shared<SkeletonMeshVertexShader>() )
{
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device,
	Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext )
{
	this->m_device = device;
	this->m_deviceContext = deviceContext;

	this->m_imageWidth = imageWidth;
	this->m_imageHeight = imageHeight;

	// Initialize depth stencil state.
	m_depthStencilState = createDepthStencilState( *device.Get() );

	createRenderTarget( imageWidth, imageHeight, *device.Get() );

	{ // Initialize projection matrices.
		const float fieldOfView = (float)MathUtil::pi / 4.0f;
		const float screenAspect = (float)imageWidth / (float)imageHeight;
		const float zNear = 0.1f;
		const float zFar = 1000.0f;

		m_perspectiveProjectionMatrix = MathUtil::perspectiveProjectionTransformation( fieldOfView, screenAspect, zNear, zFar );
		m_orthographicProjectionMatrix = MathUtil::orthographicProjectionTransformation( (float)imageWidth, (float)imageHeight, zNear, zFar );
	}

	loadAndCompileShaders( *device.Get() );

	m_initialized = true;
}

void ShadowMapRenderer::clearRenderTarget( float depth )
{
	m_depthRenderTarget->clearDepthStencilView( *m_deviceContext.Get(), true, depth, true, 0 );
}

void ShadowMapRenderer::disableRenderTarget()
{
	m_rendererCore.disableRenderTargetViews();
}

void ShadowMapRenderer::render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix )
{
	if ( !m_initialized ) throw std::exception( "ShadowMapRenderer::render - renderer not initialized." );

	// Enable render targets.
	m_rendererCore.enableRenderTargets( m_depthRenderTarget );

	{ // Configure and set shaders.
		m_blockMeshVertexShader->setParameters( *m_deviceContext.Get(), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix );

		m_rendererCore.enableRenderingShaders( m_blockMeshVertexShader, nullptr );
	}

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void ShadowMapRenderer::render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	// Enable render targets.
	m_rendererCore.enableRenderTargets( m_depthRenderTarget );

	{ // Configure and set shaders.
		m_skeletonMeshVertexShader->setParameters( *m_deviceContext.Get(), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix, mesh, poseInSkeletonSpace );

		m_rendererCore.enableRenderingShaders( m_skeletonMeshVertexShader, nullptr );
	}

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ShadowMapRenderer::createDepthStencilState( ID3D11Device& device )
{
	D3D11_DEPTH_STENCIL_DESC        depthStencilDesc;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	depthStencilDesc.DepthEnable    = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable    = false;
	depthStencilDesc.StencilReadMask  = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

	HRESULT result = device.CreateDepthStencilState( &depthStencilDesc, depthStencilState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "ShadowMapRenderer::createDepthStencilState - creation of depth/stencil state failed." );

	return depthStencilState;
}

void ShadowMapRenderer::createRenderTarget( int imageWidth, int imageHeight, ID3D11Device& device )
{
	// Create depth render target.
	m_depthRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >
		( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT );
}

void ShadowMapRenderer::loadAndCompileShaders( ID3D11Device& device )
{
	m_blockMeshVertexShader->compileFromFile( "Shaders/BlockMeshShader/vs.hlsl", device );
	m_skeletonMeshVertexShader->compileFromFile( "Shaders/SkeletonMeshShader/vs.hlsl", device );
}
