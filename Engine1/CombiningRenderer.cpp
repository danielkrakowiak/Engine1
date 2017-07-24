#include "CombiningRenderer.h"

#include "Direct3DRendererCore.h"

#include "ShadingComputeShader.h"
#include "Camera.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombiningRenderer::CombiningRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_combiningVertexShader( std::make_shared< CombiningVertexShader >() ),
    m_combiningFragmentShader( std::make_shared< CombiningFragmentShader >() ),
    m_combiningFragmentShader2( std::make_shared< CombiningFragmentShader2 >() ),
    m_normalThreshold( 0.97f ),
    m_positionThreshold( 0.15f )
{}

CombiningRenderer::~CombiningRenderer()
{}

void CombiningRenderer::initialize( ComPtr< ID3D11Device3 > device, 
                                    ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

    m_rasterizerState = createRasterizerState( *device.Get() );
	m_blendState      = createBlendState( *device.Get() );

    loadAndCompileShaders( device );

    { // Load default rectangle mesh to GPU.
		m_rectangleMesh.loadCpuToGpu( *device.Get() );
	}

	m_initialized = true;
}

void CombiningRenderer::combine( 
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > destTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
    const float3 cameraPosition,
    const int contributionTextureFilledWidth, 
    const int contributionTextureFilledHeight,
    const int srcTextureFilledWidth, 
    const int srcTextureFilledHeight )
{
    if ( !m_initialized ) throw std::exception( "CombiningRenderer::combine - renderer not initialized." );

    m_rendererCore.setViewport( (float2)destTexture->getDimensions() );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >         renderTargetsF1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsF4.push_back( destTexture );

		m_rendererCore.enableRenderTargets( renderTargetsF1, renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_combiningVertexShader->setParameters( *m_deviceContext.Get() );
		m_combiningFragmentShader->setParameters( *m_deviceContext.Get(), srcTexture, contributionTermTexture, normalTexture, positionTexture, depthTexture, hitDistanceTexture,
                                                m_normalThreshold, m_positionThreshold, cameraPosition, 
                                                contributionTextureFilledWidth, contributionTextureFilledHeight,
                                                srcTextureFilledWidth, srcTextureFilledHeight );

		m_rendererCore.enableRenderingShaders( m_combiningVertexShader, m_combiningFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( *m_blendState.Get() );

	m_rendererCore.draw( m_rectangleMesh );

	m_combiningFragmentShader->unsetParameters( *m_deviceContext.Get() );
    
    m_rendererCore.disableRenderTargetViews();
}

void CombiningRenderer::combine( 
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > destTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitNormalTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitPositionTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  previousHitDistanceTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousRayOriginTexture,
    const int contributionTextureFilledWidth, 
    const int contributionTextureFilledHeight,
    const int srcTextureFilledWidth, 
    const int srcTextureFilledHeight )
{
    if ( !m_initialized ) throw std::exception( "CombiningRenderer::combine - renderer not initialized." );

    m_rendererCore.setViewport( (float2)destTexture->getDimensions() );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >         renderTargetsF1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsF4.push_back( destTexture );

		m_rendererCore.enableRenderTargets( renderTargetsF1, renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_combiningVertexShader->setParameters( *m_deviceContext.Get() );
		m_combiningFragmentShader2->setParameters( *m_deviceContext.Get(), srcTexture, contributionTermTexture, previousHitNormalTexture, previousHitPositionTexture, previousHitDistanceTexture, hitDistanceTexture,
                                                previousRayOriginTexture, m_normalThreshold, m_positionThreshold, 
                                                contributionTextureFilledWidth, contributionTextureFilledHeight,
                                                srcTextureFilledWidth, srcTextureFilledHeight );

		m_rendererCore.enableRenderingShaders( m_combiningVertexShader, m_combiningFragmentShader2 );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( *m_blendState.Get() );

	m_rendererCore.draw( m_rectangleMesh );

	m_combiningFragmentShader2->unsetParameters( *m_deviceContext.Get() );
    
    m_rendererCore.disableRenderTargetViews();
}

void CombiningRenderer::setNormalThreshold( float threshold )
{
    m_normalThreshold = fmax( 0.0f, fmin( 1.0f, threshold));
}

float CombiningRenderer::getNormalThreshold() const
{
    return m_normalThreshold;
}

void CombiningRenderer::setPositionThreshold( float threshold )
{
    m_positionThreshold = fmax( 0.0f, threshold);
}

float CombiningRenderer::getPositionThreshold() const
{
    return m_positionThreshold;
}

ComPtr<ID3D11RasterizerState> CombiningRenderer::createRasterizerState( ID3D11Device3& device )
{
	D3D11_RASTERIZER_DESC         rasterDesc;
	ComPtr<ID3D11RasterizerState> rasterizerState;

	ZeroMemory( &rasterDesc, sizeof( rasterDesc ) );

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode              = D3D11_CULL_NONE; // Culling disabled.
	rasterDesc.DepthBias             = 0;
	rasterDesc.DepthBiasClamp        = 0.0f;
	rasterDesc.DepthClipEnable       = false; // Depth test disabled.
	rasterDesc.FillMode              = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable     = false;
	rasterDesc.ScissorEnable         = false;
	rasterDesc.SlopeScaledDepthBias  = 0.0f;

	HRESULT result = device.CreateRasterizerState( &rasterDesc, rasterizerState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "CombiningRenderer::createRasterizerState - creation of rasterizer state failed" );

	return rasterizerState;
}

ComPtr<ID3D11BlendState> CombiningRenderer::createBlendState( ID3D11Device3& device )
{
	ComPtr<ID3D11BlendState> blendState;
	D3D11_BLEND_DESC         blendDesc;

	ZeroMemory( &blendDesc, sizeof( blendDesc ) );

	blendDesc.AlphaToCoverageEnable  = false;
	blendDesc.IndependentBlendEnable = false;
    // SRC * SRC_ALPHA + DEST * 1.0
 //   blendDesc.RenderTarget[ 0 ].BlendEnable           = true;
	//blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	//blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ONE;//D3D11_BLEND_INV_SRC_ALPHA;
	//blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	//blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;

	// Enable blending.
	blendDesc.RenderTarget[ 0 ].BlendEnable           = true;
	blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "CombiningRenderer::createBlendState - creation of blend state failed." );

	return blendState;
}

void CombiningRenderer::loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device )
{
    m_combiningVertexShader->loadAndInitialize( "Engine1/Shaders/CombiningShader/Combining_vs.cso", device );
    m_combiningFragmentShader->loadAndInitialize( "Engine1/Shaders/CombiningShader/Combining_ps.cso", device );
    m_combiningFragmentShader2->loadAndInitialize( "Engine1/Shaders/CombiningShader/Combining_ps2.cso", device );
}
