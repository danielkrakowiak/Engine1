#include "Direct3DFrameRenderer.h"

#include <exception>
#include <memory>
#include <string>

#include "Direct3DRendererCore.h"

#include "MathUtil.h"
#include "Direct3DUtil.h"

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DFrameRenderer::Direct3DFrameRenderer( Direct3DRendererCore& rendererCore, Profiler& profiler ) :
    m_rendererCore( rendererCore ),
    m_profiler( profiler ),
    m_initialized( false ),
    m_gpuMemory( 0 ),
    m_fullscreen( false ),
    m_screenWidth( 1024 ),
    m_screenHeight( 768 ),
    m_verticalSync( true ),
    m_textureVertexShader( std::make_shared<TextureVertexShader>() ),
    m_textureFragmentShader( std::make_shared<TextureFragmentShader>() ),
    m_textureAlphaFragmentShader( std::make_shared<TextureFragmentShader>() ),
    m_textVertexShader( std::make_shared<TextVertexShader>() ),
    m_textFragmentShader( std::make_shared<TextFragmentShader>() )
{
}

Direct3DFrameRenderer::~Direct3DFrameRenderer()
{
	if ( m_initialized ) {
		if ( m_swapChain ) {
			// set to windowed mode or when you release the swap chain it will throw an exception.
			m_swapChain->SetFullscreenState( false, nullptr );
		}
	}

	//reportLiveObjects( );
}

void Direct3DFrameRenderer::initialize( HWND windowHandle, int screenWidth, int screenHeight, bool fullscreen, bool verticalSync )
{
	this->m_fullscreen   = fullscreen;
	this->m_screenWidth  = screenWidth;
	this->m_screenHeight = screenHeight;
	this->m_verticalSync = verticalSync;

	int refreshRateNumerator = 0, refreshRateDenominator = 1;
	{
		ComPtr<IDXGIFactory> factory;
		ComPtr<IDXGIAdapter> adapter;
		HRESULT result;

		result = CreateDXGIFactory1( __uuidof( IDXGIFactory ), (void**)factory.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "Direct3DRenderer::initialize - creation of DXGIFactory failed" );

		result = factory->EnumAdapters( 0, adapter.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "Direct3DRenderer::initialize - enumarating adapters failed" );

		std::tie( refreshRateNumerator, refreshRateDenominator ) = getRefreshRateNumeratorDenominator( *adapter.Get(), (unsigned int)screenWidth, (unsigned int)screenHeight );
		m_gpuMemory = getGpuMemory( *adapter.Get() );
		m_gpuDescription = getGpuDescription( *adapter.Get() );
	}

	std::tie( m_swapChain, m_device, m_deviceContext ) = createDeviceAndSwapChain( windowHandle, fullscreen, verticalSync, screenWidth, screenHeight, refreshRateNumerator, refreshRateDenominator );

	{ // Initialize render target.
        ComPtr< ID3D11Texture2D > backbufferTexture = getBackbufferTexture( *m_swapChain.Get() );

        m_renderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget, uchar4 > >
            ( *m_device.Get(), backbufferTexture );
	}

	m_rasterizerState        = createRasterizerState( *m_device.Get() );
    m_blendStateNoBlending   = createBlendStateNoBlending( *m_device.Get() );
	m_blendStateWithBlending = createBlendStateWithBlending( *m_device.Get() );

	loadAndCompileShaders( m_device );

	{ // Load default rectangle mesh to GPU.
		rectangleMesh.loadCpuToGpu( *m_device.Get() );
	}

	m_initialized = true;
}

void Direct3DFrameRenderer::reportLiveObjects()
{
	// Print Direct3D objects which were not released yet.
	ComPtr<ID3D11Debug> deviceDebug;
	m_device.As( &deviceDebug );
	if ( deviceDebug ) {
		deviceDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
	}
}

std::tuple<int, int> Direct3DFrameRenderer::getRefreshRateNumeratorDenominator( IDXGIAdapter& adapter, unsigned int screenWidth, unsigned int screenHeight )
{
	HRESULT             result;
	ComPtr<IDXGIOutput> adapterOutput;
	unsigned int        numModes       = 0;
	int                 numerator      = 0;
	int                 denominator    = 1;

	// Enumerate the primary adapter output (monitor).
	result = adapter.EnumOutputs( 0, adapterOutput.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::getRefreshRateNumeratorDenominator - enumarating outputs failed" );

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::getRefreshRateNumeratorDenominator - getting display modes failed" );

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	std::unique_ptr<DXGI_MODE_DESC[ ]> displayModeList( new DXGI_MODE_DESC[ numModes ] );

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.get() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::getRefreshRateNumeratorDenominator - enumarating outputs failed" );

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	bool displayModeFound = false;
	for ( unsigned int i = 0; i < numModes; i++ ) {
		if ( displayModeList[ i ].Width == screenWidth ) {
			if ( displayModeList[ i ].Height == screenHeight ) {
				numerator   = displayModeList[ i ].RefreshRate.Numerator;
				denominator = displayModeList[ i ].RefreshRate.Denominator;

				displayModeFound = true;
				break;
			}
		}
	}

	if ( !displayModeFound ) throw std::exception( "Direct3DRenderer::getRefreshRateNumeratorDenominator - needed display mode is not available" );

	return std::make_tuple( numerator, denominator );
}

std::string Direct3DFrameRenderer::getGpuDescription( IDXGIAdapter& adapter )
{
	DXGI_ADAPTER_DESC adapterDesc;

	HRESULT result = adapter.GetDesc( &adapterDesc );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::getGpuDescription - getting adapter descriptor failed" );

	const int maxStringLength = 128;
	char      description[ maxStringLength ];
	size_t    stringLength = 0;

	int error = wcstombs_s( &stringLength, description, maxStringLength, adapterDesc.Description, maxStringLength );
	if ( error != 0 ) throw std::exception( "Direct3DRenderer::getGpuDescription - getting adapter description string failed" );

	return std::string( description );
}

size_t Direct3DFrameRenderer::getGpuMemory( IDXGIAdapter& adapter )
{
	DXGI_ADAPTER_DESC adapterDesc;

	HRESULT result = adapter.GetDesc( &adapterDesc );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::getGpuDescription - getting adapter descriptor failed" );

	return adapterDesc.DedicatedVideoMemory;
}

std::tuple< ComPtr<IDXGISwapChain>, ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext> >  Direct3DFrameRenderer::createDeviceAndSwapChain( HWND windowHandle, bool fullscreen, bool verticalSync, unsigned int screenWidth, unsigned int screenHeight, int refreshRateNumerator, int refreshRateDenominator )
{
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11Device> basicDevice;
	ComPtr<ID3D11DeviceContext> basicDeviceContext;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if ( verticalSync ) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = refreshRateNumerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = refreshRateDenominator;
	} else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = windowHandle;
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	if ( fullscreen ) {
		swapChainDesc.Windowed = false;
	} else {
		swapChainDesc.Windowed = true;
	}

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swapChainDesc.Flags = 0;

	//////////////////////////////////////////////////////

	// Enable debug layer if in debug mode.
	unsigned int flags = 0;
#if defined(DEBUG_DIRECT3D) /*|| defined(_DEBUG)*/
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	HRESULT result = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &featureLevel, 1,
													D3D11_SDK_VERSION, &swapChainDesc, swapChain.ReleaseAndGetAddressOf(),
													basicDevice.ReleaseAndGetAddressOf(), nullptr, basicDeviceContext.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createDeviceAndSwapChain - creation of device or swap chain failed" );

	ComPtr< ID3D11Device3 >        device;
	ComPtr< ID3D11DeviceContext3 > deviceContext;

    result = basicDevice.As( &device );

    if ( result < 0 ) 
        throw std::exception( "Direct3DRenderer::createDeviceAndSwapChain - creation of DirectX 11.3 device failed" );

    (void)basicDeviceContext.As( &deviceContext );

    return std::make_tuple( swapChain, device, deviceContext );

    //////////////////////////////////////////////////////////////
    // Check features support
    //////////////////////////////////////////////////////////////

    //D3D11_FEATURE_DATA_D3D11_OPTIONS2 featureData;
    //ZeroMemory( &featureData, sizeof( featureData ) );
    //HRESULT hr = basicDevice->CheckFeatureSupport( D3D11_FEATURE_D3D11_OPTIONS2, &featureData, sizeof( featureData ) );
    //if ( SUCCEEDED( hr ) ) {
    //    // TypedUAVLoadAdditionalFormats contains a Boolean that tells you whether the feature is supported or not
    //    if ( featureData.TypedUAVLoadAdditionalFormats ) {
    //        // Can assume “all-or-nothing” subset is supported (e.g. R32G32B32A32_FLOAT)
    //        // Can not assume other formats are supported, so we check:
    //        D3D11_FEATURE_DATA_FORMAT_SUPPORT2 FormatSupport;
    //        ZeroMemory( &FormatSupport, sizeof( FormatSupport ) );
    //        FormatSupport.InFormat = DXGI_FORMAT_R32G32_FLOAT;
    //        hr = basicDevice->CheckFeatureSupport( D3D11_FEATURE_FORMAT_SUPPORT2, &FormatSupport, sizeof( FormatSupport ) );
    //        if ( SUCCEEDED( hr ) && ( FormatSupport.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD ) != 0 ) {
    //            // DXGI_FORMAT_R32G32_FLOAT supports UAV Typed Load!
    //        }
    //    }
    //}

    /////////////////////////////////////////////////////////////

    //return std::make_tuple( swapChain, basicDevice, basicDeviceContext );
}

ComPtr< ID3D11Texture2D > Direct3DFrameRenderer::getBackbufferTexture( IDXGISwapChain& swapChain )
{
	ComPtr< ID3D11Texture2D > backBufferPtr;

	// Get the pointer to the back buffer.
	HRESULT result = swapChain.GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)backBufferPtr.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createRenderTargetView - getting pointer to the backbuffer failed" );

	return backBufferPtr;
}

ComPtr<ID3D11RasterizerState> Direct3DFrameRenderer::createRasterizerState( ID3D11Device& device )
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
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createRasterizerState - creation of rasterizer state failed" );

	return rasterizerState;
}

ComPtr<ID3D11BlendState> Direct3DFrameRenderer::createBlendStateNoBlending( ID3D11Device& device )
{
	ComPtr<ID3D11BlendState> blendState;
	D3D11_BLEND_DESC         blendDesc;

	ZeroMemory( &blendDesc, sizeof( blendDesc ) );

	blendDesc.AlphaToCoverageEnable  = false;
	blendDesc.IndependentBlendEnable = false;

	// Enable blending.
	blendDesc.RenderTarget[ 0 ].BlendEnable           = false;
	blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DFrameRenderer::createBlendStateNoBlending - creation of blend state failed." );

	return blendState;
}

ComPtr<ID3D11BlendState> Direct3DFrameRenderer::createBlendStateWithBlending( ID3D11Device& device )
{
    ComPtr<ID3D11BlendState> blendState;
    D3D11_BLEND_DESC         blendDesc;

    ZeroMemory( &blendDesc, sizeof( blendDesc ) );

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    // Enable blending.
    blendDesc.RenderTarget[ 0 ].BlendEnable = true;//false;
    blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;//D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;//D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;//D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;//D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

    HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
    if ( result < 0 ) throw std::exception( "Direct3DFrameRenderer::createBlendStateWithBlending - creation of blend state failed." );

    return blendState;
}

void Direct3DFrameRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
	m_textureVertexShader->loadAndInitialize( "Shaders/TextureShader/Texture_vs.cso", device );
	m_textureFragmentShader->loadAndInitialize( "Shaders/TextureShader/Texture_ps.cso", device );
    m_textureAlphaFragmentShader->loadAndInitialize( "Shaders/TextureShader/Texture_ps2.cso", device );

	m_textVertexShader->loadAndInitialize( "Shaders/TextShader/Text_vs.cso", device );
	m_textFragmentShader->loadAndInitialize( "Shaders/TextShader/Text_ps.cso", device );
}

void Direct3DFrameRenderer::renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, unsigned char>& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

	float relativeWidth  = ( width / (float)m_screenWidth );
	float relativeHeight = ( height / (float)m_screenHeight );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, uchar4>& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

    float relativeWidth  = ( width / (float)m_screenWidth );
    float relativeHeight = ( height / (float)m_screenHeight );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::renderTexture( const Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

    float relativeWidth  = ( width / (float)m_screenWidth );
    float relativeHeight = ( height / (float)m_screenHeight );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::renderTexture( const Texture2DSpecBind< TexBind::ShaderResource, float2 >& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

    float relativeWidth  = ( width / (float)m_screenWidth );
    float relativeHeight = ( height / (float)m_screenHeight );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::renderTexture( const Texture2DSpecBind< TexBind::ShaderResource, float >& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

    float relativeWidth  = ( width / (float)m_screenWidth );
    float relativeHeight = ( height / (float)m_screenHeight );


	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::renderTextureAlpha( const Texture2DSpecBind<TexBind::ShaderResource, uchar4>& texture, float posX, float posY, float width, float height, bool blend )
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::renderTexture - renderer not initialized." );

    m_rendererCore.setViewport( float2( (float)m_screenWidth, (float)m_screenHeight ) );

    float relativeWidth  = ( width / (float)m_screenWidth );
    float relativeHeight = ( height / (float)m_screenHeight );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;
		renderTargetsU4.push_back( m_renderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, nullptr );
	}

	{ // Configure and enable shaders.
		m_textureVertexShader->setParameters( *m_deviceContext.Get(), posX, posY, relativeWidth, relativeHeight );
		m_textureAlphaFragmentShader->setParameters( *m_deviceContext.Get(), texture );

		m_rendererCore.enableRenderingShaders( m_textureVertexShader, m_textureAlphaFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( blend ? *m_blendStateWithBlending.Get() : *m_blendStateNoBlending.Get() );

	m_rendererCore.draw( rectangleMesh );

	m_textureAlphaFragmentShader->unsetParameters( *m_deviceContext.Get() );
}

void Direct3DFrameRenderer::displayFrame()
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::displayFrame - renderer not initialized." );

	HRESULT result = 0;
	if ( m_verticalSync ) {
		result = m_swapChain->Present( 1, 0 );
	} else {
		result = m_swapChain->Present( 0, 0 );
	}

	if ( result < 0 ) throw std::exception( "Direct3DRenderer::displayFrame - swapping frame failed" );

}

ComPtr< ID3D11Device > Direct3DFrameRenderer::getDevice()
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::getDevice - renderer not initialized." );

	return m_device;
}

ComPtr< ID3D11DeviceContext > Direct3DFrameRenderer::getDeviceContext()
{
	if ( !m_initialized ) throw std::exception( "Direct3DFrameRenderer::getDeviceContext - renderer not initialized." );

	return m_deviceContext;
}
