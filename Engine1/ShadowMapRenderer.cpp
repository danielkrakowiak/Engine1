#include "ShadowMapRenderer.h"

#include "DX11RendererCore.h"

#include "MathUtil.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ShadowMapRenderer::ShadowMapRenderer( DX11RendererCore& rendererCore ) :
m_rendererCore( rendererCore ),
m_initialized( false ),
m_blockMeshVertexShader( std::make_shared<BlockMeshVertexShader>() ),
m_skeletonMeshVertexShader( std::make_shared<SkeletonMeshVertexShader>() )
{
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device,
	Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext )
{
	this->m_device = device;
	this->m_deviceContext = deviceContext;

	// Initialize depth stencil state.
	m_depthStencilState = createDepthStencilState( *device.Get() );

	loadAndCompileShaders( device );

	m_initialized = true;
}

void ShadowMapRenderer::clearRenderTarget( float depth )
{
	m_renderTarget->clearDepthStencilView( *m_deviceContext.Get(), true, depth, false, 0 );
}

void ShadowMapRenderer::disableRenderTarget()
{
	m_rendererCore.disableRenderTargets();

    m_renderTarget.reset();
}

void ShadowMapRenderer::render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const float44& perspectiveMatrix )
{
	if ( !m_initialized ) throw std::exception( "ShadowMapRenderer::render - renderer not initialized." );

    m_rendererCore.setViewport( (float2)m_renderTarget->getDimensions() );

	// Enable render targets.
	RenderTargets renderTargets;
	renderTargets.depth = m_renderTarget;
	m_rendererCore.enableRenderTargets( renderTargets, RenderTargets() );

	{ // Configure and set shaders.
		m_blockMeshVertexShader->setParameters( *m_deviceContext.Get(), worldMatrix, viewMatrix, perspectiveMatrix );

		m_rendererCore.enableRenderingShaders( m_blockMeshVertexShader, nullptr );
	}

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void ShadowMapRenderer::render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const float44& perspectiveMatrix, const SkeletonPose& poseInSkeletonSpace )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

    m_rendererCore.setViewport( (float2)m_renderTarget->getDimensions() );

	// Enable render targets.
	RenderTargets renderTargets;
	renderTargets.depth = m_renderTarget;
	m_rendererCore.enableRenderTargets( renderTargets, RenderTargets() );

	{ // Configure and set shaders.
		m_skeletonMeshVertexShader->setParameters( *m_deviceContext.Get(), worldMatrix, viewMatrix, perspectiveMatrix, mesh, poseInSkeletonSpace );

		m_rendererCore.enableRenderingShaders( m_skeletonMeshVertexShader, nullptr );
	}

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ShadowMapRenderer::createDepthStencilState( ID3D11Device3& device )
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

void ShadowMapRenderer::setRenderTarget( std::shared_ptr< DepthTexture2D< float > > renderTarget )
{
    m_renderTarget = renderTarget;
}

void ShadowMapRenderer::createAndSetRenderTarget( const int2 dimensions, ID3D11Device3& device )
{
	// Create depth render target.
	m_renderTarget = std::make_shared< DepthTexture2D< float > >
		( device, dimensions.x, dimensions.y, false, true, false, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT );
}

void ShadowMapRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
	m_blockMeshVertexShader->loadAndInitialize( "Engine1/Shaders/BlockMeshShader/BlockMesh_vs.cso", device );
	m_skeletonMeshVertexShader->loadAndInitialize( "Engine1/Shaders/SkeletonMeshShader/SkeletonMesh_vs.cso", device );
}

std::shared_ptr< DepthTexture2D< float > > ShadowMapRenderer::getRenderTarget()
{
    return m_renderTarget;
}
