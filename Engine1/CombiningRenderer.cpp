#include "CombiningRenderer.h"

#include "Direct3DRendererCore.h"

#include "ShadingComputeShader.h"
#include "Camera.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

CombiningRenderer::CombiningRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
combiningVertexShader( std::make_shared< CombiningVertexShader >() ),
combiningFragmentShader( std::make_shared< CombiningFragmentShader >() )
{}

CombiningRenderer::~CombiningRenderer()
{}

void CombiningRenderer::initialize( const int screenWidth, const int screenHeight, 
                                    ComPtr< ID3D11Device > device, 
                                    ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->device        = device;
	this->deviceContext = deviceContext;

    rasterizerState = createRasterizerState( *device.Get() );
	blendState      = createBlendState( *device.Get() );

    // TODO: Should be done through rendererCore every time before rendering.
	{ // Initialize viewport.
		D3D11_VIEWPORT viewport;
		viewport.Width    = (float)screenWidth;
		viewport.Height   = (float)screenHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		deviceContext->RSSetViewports( 1, &viewport );
	}

    loadAndCompileShaders( *device.Get() );

    { // Load default rectangle mesh to GPU.
		rectangleMesh.loadCpuToGpu( *device.Get() );
	}

	initialized = true;
}

void CombiningRenderer::combine( std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                                 const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > edgeDistanceTexture,
                                 const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                                 const std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >& srcTextureUpscaledMipmaps )
{
    if ( !initialized ) throw std::exception( "CombiningRenderer::combine - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsF4.push_back( destTexture );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		combiningVertexShader->setParameters( *deviceContext.Get() );
		combiningFragmentShader->setParameters( *deviceContext.Get(), edgeDistanceTexture, srcTexture, srcTextureUpscaledMipmaps );

		rendererCore.enableRenderingShaders( combiningVertexShader, combiningFragmentShader );
	}

	rendererCore.enableRasterizerState( *rasterizerState.Get() );
	rendererCore.enableBlendState( *blendState.Get() );

	rendererCore.draw( rectangleMesh );

	combiningFragmentShader->unsetParameters( *deviceContext.Get() );
}

ComPtr<ID3D11RasterizerState> CombiningRenderer::createRasterizerState( ID3D11Device& device )
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

ComPtr<ID3D11BlendState> CombiningRenderer::createBlendState( ID3D11Device& device )
{
	ComPtr<ID3D11BlendState> blendState;
	D3D11_BLEND_DESC         blendDesc;

	ZeroMemory( &blendDesc, sizeof( blendDesc ) );

	blendDesc.AlphaToCoverageEnable  = false;
	blendDesc.IndependentBlendEnable = false;

    blendDesc.RenderTarget[ 0 ].BlendEnable           = true;
	blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;

	// Enable blending.
	/*blendDesc.RenderTarget[ 0 ].BlendEnable           = true;
	blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;*/
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "CombiningRenderer::createBlendState - creation of blend state failed." );

	return blendState;
}

void CombiningRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    combiningVertexShader->compileFromFile( "../Engine1/Shaders/CombiningShader/vs.hlsl", device );
    combiningFragmentShader->compileFromFile( "../Engine1/Shaders/CombiningShader/ps.hlsl", device );
}
