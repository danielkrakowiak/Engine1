#include "MipmapRenderer.h"

#include "Direct3DRendererCore.h"

#include "GenerateMipmapMinValueComputeShader.h"
#include "GenerateMipmapVertexShader.h"
#include "ResampleTextureFragmentShader.h"
#include "GenerateMipmapMinValueFragmentShader.h"
#include "GenerateMipmapWithSampleRejectionFragmentShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

MipmapRenderer::MipmapRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_generateMipmapMinValueComputeShader( std::make_shared< GenerateMipmapMinValueComputeShader >() ),
    m_generateMipmapVertexShader( std::make_shared< GenerateMipmapVertexShader >() ),
	m_resampleTextureFragmentShader( std::make_shared< ResampleTextureFragmentShader >() ),
    m_generateMipmapMinValueFragmentShader( std::make_shared< GenerateMipmapMinValueFragmentShader >() ),
    m_generateMipmapWithSampleRejectionFragmentShader( std::make_shared< GenerateMipmapWithSampleRejectionFragmentShader >() )
{}

MipmapRenderer::~MipmapRenderer()
{}

void MipmapRenderer::initialize( ComPtr< ID3D11Device3 > device,
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

void MipmapRenderer::resampleTexture( 
	std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture, int destMipmapLevel,
	std::shared_ptr< Texture2D< float4 > > srcTexture, int srcMipmapLevel )
{
	if ( !m_initialized )
		throw std::exception( "MipmapRenderer::resampleTexture - renderer not initialized." );

	RenderTargets renderTargets;
	renderTargets.typeFloat4.push_back( destTexture );

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( *m_blendState.Get() );
	m_rendererCore.enableRenderingShaders( m_generateMipmapVertexShader, m_resampleTextureFragmentShader );
	m_rendererCore.setViewport( (float2)destTexture->getDimensions( destMipmapLevel ) );
	m_rendererCore.enableRenderTargets( renderTargets, RenderTargets(), destMipmapLevel );

	m_generateMipmapVertexShader->setParameters( *m_deviceContext.Get() );
	m_resampleTextureFragmentShader->setParameters( *m_deviceContext.Get(), *srcTexture, srcMipmapLevel );

	m_rendererCore.draw( m_rectangleMesh );

	m_resampleTextureFragmentShader->unsetParameters( *m_deviceContext.Get() );

	m_rendererCore.disableRenderTargets();
}

void MipmapRenderer::generateMipmaps( std::shared_ptr< RenderTargetTexture2D< float4 > > texture, int startSrcMipmapLevel, int generateMipmapCount )
{
	if ( !m_initialized )
		throw std::exception( "MipmapRenderer::generateMipmaps - renderer not initialized." );


	RenderTargets renderTargets;
	renderTargets.typeFloat4.push_back( texture );

	const int mipmapCount = texture->getMipMapCountOnGpu();

	if ( generateMipmapCount < 0 )
		generateMipmapCount = mipmapCount;

	const int maxSrcMipmapLevel = std::min( mipmapCount - 2, startSrcMipmapLevel + generateMipmapCount );

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
	m_rendererCore.enableBlendState( *m_blendState.Get() );
	m_rendererCore.enableRenderingShaders( m_generateMipmapVertexShader, m_resampleTextureFragmentShader );

	for ( int srcMipmapLevel = startSrcMipmapLevel; srcMipmapLevel <= maxSrcMipmapLevel; ++srcMipmapLevel )
	{
		const int destMipmapLevel = srcMipmapLevel + 1;

		m_rendererCore.setViewport( (float2)texture->getDimensions( destMipmapLevel ) );

		m_rendererCore.enableRenderTargets( renderTargets, RenderTargets(), destMipmapLevel );

		m_generateMipmapVertexShader->setParameters( *m_deviceContext.Get() );
		m_resampleTextureFragmentShader->setParameters( *m_deviceContext.Get(), *texture, srcMipmapLevel );

		m_rendererCore.draw( m_rectangleMesh );
	}

	m_resampleTextureFragmentShader->unsetParameters( *m_deviceContext.Get() );

	m_rendererCore.disableRenderTargets();
}

//void MipmapMinValueRenderer::generateMipmapsMinValue( std::shared_ptr< RenderTargetTexture2D< float > >& texture )
//{
//    m_rendererCore.disableRenderingPipeline();
//
//    std::vector< std::shared_ptr< Texture2D< float > > >         unorderedAccessTargetsF1;
//    std::vector< std::shared_ptr< Texture2D< float2 > > >        unorderedAccessTargetsF2;
//    std::vector< std::shared_ptr< Texture2D< float4 > > >        unorderedAccessTargetsF4;
//    std::vector< std::shared_ptr< Texture2D< unsigned char > > > unorderedAccessTargetsU1;
//    std::vector< std::shared_ptr< Texture2D< uchar4 > > >        unorderedAccessTargetsU4;
//
//    unorderedAccessTargetsF1.push_back( texture );
//
//    const int mipmapCount = texture->getMipMapCountOnGpu();
//
//    for ( int srcMipmapLevel = 0; srcMipmapLevel + 1 < mipmapCount; ++srcMipmapLevel )
//    {
//        const int destMipmapLevel = srcMipmapLevel + 1;
//
//        m_rendererCore.enableRenderTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
//                                                     unorderedAccessTargetsU1, unorderedAccessTargetsU4, destMipmapLevel );
//
//        m_generateMipmapMinValueComputeShader->setParameters( *m_deviceContext.Get(), *texture, srcMipmapLevel );
//
//        m_rendererCore.enableComputeShader( m_generateMipmapMinValueComputeShader );
//
//        uint3 groupCount( texture->getWidth( destMipmapLevel ) / 8, texture->getHeight( destMipmapLevel ) / 8, 1 );
//
//        m_rendererCore.compute( groupCount );
//    }
//
//    m_generateMipmapMinValueComputeShader->unsetParameters( *m_deviceContext.Get() );
//
//    m_rendererCore.disableComputePipeline();
//}

void MipmapRenderer::generateMipmapsMinValue( std::shared_ptr< RenderTargetTexture2D< float > >& texture )
{
    if ( !m_initialized ) 
        throw std::exception( "MipmapMinValueRenderer::generateMipmapsMinValue - renderer not initialized." );


	RenderTargets renderTargets;
    renderTargets.typeFloat.push_back( texture );

    const int mipmapCount = texture->getMipMapCountOnGpu();

    m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
    m_rendererCore.enableBlendState( *m_blendState.Get() );
    m_rendererCore.enableRenderingShaders( m_generateMipmapVertexShader, m_generateMipmapMinValueFragmentShader );

    for ( int srcMipmapLevel = 0; srcMipmapLevel + 1 < mipmapCount; ++srcMipmapLevel ) 
    {
        const int destMipmapLevel = srcMipmapLevel + 1;

        m_rendererCore.setViewport( (float2)texture->getDimensions( destMipmapLevel ) );

        m_rendererCore.enableRenderTargets( renderTargets, RenderTargets(), destMipmapLevel );

        m_generateMipmapVertexShader->setParameters( *m_deviceContext.Get() );
        m_generateMipmapMinValueFragmentShader->setParameters( *m_deviceContext.Get(), *texture, srcMipmapLevel );

        m_rendererCore.draw( m_rectangleMesh );
    }

    m_generateMipmapMinValueFragmentShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableRenderTargets();
}

void MipmapRenderer::generateMipmapsWithSampleRejection( const std::shared_ptr< RenderTargetTexture2D< float > >& texture, 
                                                         const float maxAcceptableValue, const int initialSrcMipmapLevel, int generateMipmapCount )
{
    if ( !m_initialized )
        throw std::exception( "MipmapRenderer::generateMipmapsWithSampleRejection - renderer not initialized." );

    const int mipmapCount = texture->getMipMapCountOnGpu();

    // Note: There is no point making the last mipmap a source, because there would be no destination mipmap.
    if ( initialSrcMipmapLevel < 0 || initialSrcMipmapLevel > mipmapCount - 2 )
        throw std::exception( "MipmapRenderer::generateMipmapsWithSampleRejection - incorrect value passed as \"initialSrcMipmapLevel\" argument (negative or too high)." );

    if ( generateMipmapCount <= 0 )
        generateMipmapCount = mipmapCount - 1 - initialSrcMipmapLevel;

    generateMipmapCount = std::min( generateMipmapCount, mipmapCount - 1 - initialSrcMipmapLevel );

	RenderTargets renderTargets;
    renderTargets.typeFloat.push_back( texture );

    m_rendererCore.enableRasterizerState( *m_rasterizerState.Get() );
    m_rendererCore.enableBlendState( *m_blendState.Get() );
    m_rendererCore.enableRenderingShaders( m_generateMipmapVertexShader, m_generateMipmapWithSampleRejectionFragmentShader );

    for ( int srcMipmapLevel = initialSrcMipmapLevel; srcMipmapLevel < initialSrcMipmapLevel + generateMipmapCount; ++srcMipmapLevel ) {
        const int destMipmapLevel = srcMipmapLevel + 1;

        m_rendererCore.setViewport( (float2)texture->getDimensions( destMipmapLevel ) );

        m_rendererCore.enableRenderTargets( renderTargets, RenderTargets(), destMipmapLevel );

        m_generateMipmapVertexShader->setParameters( *m_deviceContext.Get() );
        m_generateMipmapWithSampleRejectionFragmentShader->setParameters( 
            *m_deviceContext.Get(), 
            *texture, 
            srcMipmapLevel, 
            maxAcceptableValue 
        );

        m_rendererCore.draw( m_rectangleMesh );
    }

    m_generateMipmapWithSampleRejectionFragmentShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableRenderTargets();
}

ComPtr<ID3D11RasterizerState> MipmapRenderer::createRasterizerState( ID3D11Device3& device )
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
    if ( result < 0 ) throw std::exception( "MipmapMinValueRenderer::createRasterizerState - creation of rasterizer state failed" );

    return rasterizerState;
}

ComPtr<ID3D11BlendState> MipmapRenderer::createBlendState( ID3D11Device3& device )
{
    ComPtr<ID3D11BlendState> blendState;
    D3D11_BLEND_DESC         blendDesc;

    ZeroMemory( &blendDesc, sizeof( blendDesc ) );

    blendDesc.AlphaToCoverageEnable  = false;
    blendDesc.IndependentBlendEnable = false;

    // Disable blending.
    blendDesc.RenderTarget[ 0 ].BlendEnable           = false;
    blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

    HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
    if ( result < 0 ) 
        throw std::exception( "MipmapMinValueRenderer::createBlendState - creation of blend state failed." );

    return blendState;
}

void MipmapRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_generateMipmapMinValueComputeShader->loadAndInitialize( "Engine1/Shaders/GenerateMipmapShader/GenerateMipmapMinValue_cs.cso", device );
    m_generateMipmapVertexShader->loadAndInitialize( "Engine1/Shaders/GenerateMipmapShader/GenerateMipmap_vs.cso", device );
	m_resampleTextureFragmentShader->loadAndInitialize( "Engine1/Shaders/GenerateMipmapShader/GenerateMipmap_ps.cso", device );
    m_generateMipmapMinValueFragmentShader->loadAndInitialize( "Engine1/Shaders/GenerateMipmapShader/GenerateMipmapMinValue_ps.cso", device );
    m_generateMipmapWithSampleRejectionFragmentShader->loadAndInitialize( "Engine1/Shaders/GenerateMipmapShader/GenerateMipmapWithSampleRejection_ps.cso", device );
}

