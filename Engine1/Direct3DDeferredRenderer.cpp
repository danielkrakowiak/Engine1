#include "Direct3DDeferredRenderer.h"

#include "Direct3DRendererCore.h"

#include "MathUtil.h"

#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"

#include "SkeletonPose.h"

//#include "RenderTargeTexture2D.h"
//#include "RenderTargetDepthTexture2D.h"

#include "Font.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DDeferredRenderer::Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_blockMeshVertexShader( std::make_shared<BlockMeshVertexShader>() ),
    m_blockMeshFragmentShader( std::make_shared<BlockMeshFragmentShader>() ),
    m_skeletonMeshVertexShader( std::make_shared<SkeletonMeshVertexShader>() ),
    m_skeletonMeshFragmentShader( std::make_shared<SkeletonMeshFragmentShader>() ),
    m_blockModelVertexShader( std::make_shared<BlockModelVertexShader>() ),
    m_blockModelFragmentShader( std::make_shared<BlockModelFragmentShader>() ),
    m_skeletonModelVertexShader( std::make_shared<SkeletonModelVertexShader>() ),
    m_skeletonModelFragmentShader( std::make_shared<SkeletonModelFragmentShader>() ),
    m_textVertexShader( std::make_shared<TextVertexShader>() ),
    m_textFragmentShader( std::make_shared<TextFragmentShader>() )
{}

Direct3DDeferredRenderer::~Direct3DDeferredRenderer()
{}

void Direct3DDeferredRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext )
{

	this->m_device = device;
	this->m_deviceContext = deviceContext;

	this->m_imageWidth = imageWidth;
	this->m_imageHeight = imageHeight;

	// Initialize rasterizer state.
	m_rasterizerState          = createRasterizerState( *device.Get() );
    m_wireframeRasterizerState = createWireframeRasterizerState( *device.Get() );
	// Initialize depth stencil state.
	m_depthStencilState = createDepthStencilState( *device.Get() );
	// Initialize blend states.
	m_blendStateForMeshRendering = createBlendStateForMeshRendering( *device.Get() );
	m_blendStateForTextRendering = createBlendStateForTextRendering( *device.Get() );

	createRenderTargets( imageWidth, imageHeight, *device.Get() );

	{ // Initialize projection matrices.
		const float fieldOfView = (float)MathUtil::pi / 4.0f;
		const float screenAspect = (float)imageWidth / (float)imageHeight;
		const float zNear = 0.1f;
		const float zFar = 1000.0f;

		m_perspectiveProjectionMatrix = MathUtil::perspectiveProjectionTransformation( fieldOfView, screenAspect, zNear, zFar );
		m_orthographicProjectionMatrix = MathUtil::orthographicProjectionTransformation( (float)imageWidth, (float)imageHeight, zNear, zFar );
	}

	loadAndCompileShaders( *device.Get() );

    createDefaultTextures( *device.Get() );

	m_initialized = true;
}

void Direct3DDeferredRenderer::render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const bool wireframeMode )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( m_normalRenderTarget );
        renderTargetsF4.push_back( m_positionRenderTarget );
        renderTargetsU1.push_back( m_metalnessRenderTarget );
        renderTargetsU1.push_back( m_roughnessRenderTarget );
        renderTargetsU1.push_back( m_indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( m_emissiveRenderTarget );
        renderTargetsU4.push_back( m_albedoRenderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, m_depthRenderTarget );
	}

	{ // Configure and set shaders.
		m_blockMeshVertexShader->setParameters( *m_deviceContext.Get(), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix );

		m_rendererCore.enableRenderingShaders( m_blockMeshVertexShader, m_blockMeshFragmentShader );
	}

	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get( ) );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get( ) );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get( ) );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace, const bool wireframeMode )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( m_normalRenderTarget );
        renderTargetsF4.push_back( m_positionRenderTarget );
        renderTargetsU1.push_back( m_metalnessRenderTarget );
        renderTargetsU1.push_back( m_roughnessRenderTarget );
        renderTargetsU1.push_back( m_indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( m_emissiveRenderTarget );
        renderTargetsU4.push_back( m_albedoRenderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, m_depthRenderTarget );
	}

	{ // Configure and set shaders.
		m_skeletonMeshVertexShader->setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix, mesh, poseInSkeletonSpace );

		m_rendererCore.enableRenderingShaders( m_skeletonMeshVertexShader, m_skeletonMeshFragmentShader );
	}

	m_rendererCore.enableRasterizerState( wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( const BlockModel& model, const float43& worldMatrix, const float44& viewMatrix, const float4& extraEmissive, const bool wireframeMode )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( m_normalRenderTarget );
        renderTargetsF4.push_back( m_positionRenderTarget );
        renderTargetsU1.push_back( m_metalnessRenderTarget );
        renderTargetsU1.push_back( m_roughnessRenderTarget );
        renderTargetsU1.push_back( m_indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( m_emissiveRenderTarget );
        renderTargetsU4.push_back( m_albedoRenderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, m_depthRenderTarget );
	}

	{ // Configure and set shaders.
        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *m_defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissiveTexturesCount() > 0 ? *model.getEmissiveTexture( 0 ).getTexture() : *m_defaultEmissiveTexture;

	    const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getRefractiveIndexTexturesCount() > 0 ? *model.getRefractiveIndexTexture( 0 ).getTexture() : *m_defaultIndexOfRefractionTexture;

		m_blockModelVertexShader->setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix );
		m_blockModelFragmentShader->setParameters( *m_deviceContext.Get( ), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture, extraEmissive );

		m_rendererCore.enableRenderingShaders( m_blockModelVertexShader, m_blockModelFragmentShader );
	}

	m_rendererCore.enableRasterizerState( wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( const SkeletonModel& model, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace, const float4& extraEmissive, const bool wireframeMode )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );
	if ( !model.getMesh( ) ) throw std::exception( "Direct3DDeferredRenderer::render - model has no mesh." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( m_normalRenderTarget );
        renderTargetsF4.push_back( m_positionRenderTarget );
        renderTargetsU1.push_back( m_metalnessRenderTarget );
        renderTargetsU1.push_back( m_roughnessRenderTarget );
        renderTargetsU1.push_back( m_indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( m_emissiveRenderTarget );
        renderTargetsU4.push_back( m_albedoRenderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, m_depthRenderTarget );
	}

	{ // Configure and set shaders.
        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *m_defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissiveTexturesCount() > 0 ? *model.getEmissiveTexture( 0 ).getTexture() : *m_defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getRefractiveIndexTexturesCount() > 0 ? *model.getRefractiveIndexTexture( 0 ).getTexture() : *m_defaultIndexOfRefractionTexture;

		m_skeletonModelVertexShader->setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, m_perspectiveProjectionMatrix, *model.getMesh( ), poseInSkeletonSpace );
		m_skeletonModelFragmentShader->setParameters( *m_deviceContext.Get(), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture, extraEmissive );

		m_rendererCore.enableRenderingShaders( m_skeletonModelVertexShader, m_skeletonModelFragmentShader );
	}

	m_rendererCore.enableRasterizerState( wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( const std::string& text, Font& font, float2 position, float4 color )
{
	if ( !m_initialized ) throw std::exception( "Direct3DDeferredRenderer::renders - renderer not initialized." );

    color; // Unused.

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( m_normalRenderTarget );
        renderTargetsU4.push_back( m_albedoRenderTarget );

		m_rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, m_depthRenderTarget );
	}

	m_rendererCore.enableRenderingShaders( m_textVertexShader, m_textFragmentShader );
	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get( ) );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get( ) );
	m_rendererCore.enableBlendState( *m_blendStateForTextRendering.Get( ) );

	float43 worldMatrix;
	worldMatrix.identity();
	float44 viewMatrix;
	viewMatrix.identity();

	const char *charText = text.c_str(), *p;
	const FontCharacter* character = nullptr;
	float2 pos = position;

	for ( p = charText; *p; p++ ) {
		character = font.getCharacter( *p, *m_device.Get( ) );

		if ( character ) {
			if ( character->getCharcode() == '\n' ) {
				pos.y -= ( font.getLineHeight() / 64 );
				pos.x = position.x;
			} else {
				worldMatrix.setTranslation( float3( pos.x + character->getPos().x, pos.y + character->getPos().y, 0.0f ) );

				// Configure the shaders.
				m_textVertexShader->setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, m_orthographicProjectionMatrix );
				m_textFragmentShader->setParameters( *m_deviceContext.Get( ), character->getTextureResource( ) );

				// Draw the character.
				m_rendererCore.draw( *character );

				pos.x += character->getAdvance().x;
				pos.y += character->getAdvance().y;
			}
		}
	}
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > > Direct3DDeferredRenderer::getPositionRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getPositionRenderTarget - renderer not initialized." );

	return m_positionRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getEmissiveRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getEmissiveRenderTarget - renderer not initialized." );

	return m_emissiveRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getAlbedoRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getAlbedoRenderTarget - renderer not initialized." );

	return m_albedoRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getMetalnessRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getMetalnessRenderTarget - renderer not initialized." );

	return m_metalnessRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getRoughnessRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getRoughnessRenderTarget - renderer not initialized." );

	return m_roughnessRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > > Direct3DDeferredRenderer::getNormalRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getNormalRenderTarget - renderer not initialized." );

	return m_normalRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getIndexOfRefractionRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getIndexOfRefractionRenderTarget - renderer not initialized." );

	return m_indexOfRefractionRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getDepthRenderTarget()
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getRenderTarget - renderer not initialized." );

	return m_depthRenderTarget;
}

ComPtr<ID3D11RasterizerState> Direct3DDeferredRenderer::createRasterizerState( ID3D11Device& device )
{
	D3D11_RASTERIZER_DESC         rasterDesc;
	ComPtr<ID3D11RasterizerState> rasterizerState;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode              = D3D11_CULL_BACK;
	rasterDesc.DepthBias             = 0;
	rasterDesc.DepthBiasClamp        = 0.0f;
	rasterDesc.DepthClipEnable       = true;
	rasterDesc.FillMode              = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable     = false;
	rasterDesc.ScissorEnable         = false;
	rasterDesc.SlopeScaledDepthBias  = 0.0f;

	HRESULT result = device.CreateRasterizerState( &rasterDesc, rasterizerState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createRasterizerState - creation of rasterizer state failed" );

	return rasterizerState;
}

ComPtr<ID3D11RasterizerState> Direct3DDeferredRenderer::createWireframeRasterizerState( ID3D11Device& device )
{
    D3D11_RASTERIZER_DESC         rasterDesc;
    ComPtr<ID3D11RasterizerState> rasterizerState;

    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode              = D3D11_CULL_NONE;
    rasterDesc.DepthBias             = 0;
    rasterDesc.DepthBiasClamp        = 0.0f;
    rasterDesc.DepthClipEnable       = true;
    rasterDesc.FillMode              = D3D11_FILL_WIREFRAME;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable     = false;
    rasterDesc.ScissorEnable         = false;
    rasterDesc.SlopeScaledDepthBias  = 0.0f;

    HRESULT result = device.CreateRasterizerState( &rasterDesc, rasterizerState.ReleaseAndGetAddressOf() );
    if ( result < 0 ) throw std::exception( "Direct3DRenderer::createWireframeRasterizerState - creation of rasterizer state failed" );

    return rasterizerState;
}

ComPtr<ID3D11DepthStencilState> Direct3DDeferredRenderer::createDepthStencilState( ID3D11Device& device )
{
	D3D11_DEPTH_STENCIL_DESC        depthStencilDesc;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	depthStencilDesc.DepthEnable      = true;
	depthStencilDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc        = D3D11_COMPARISON_LESS;

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
	if ( result < 0 ) throw std::exception( "Direct3DDeferredRenderer::createDepthStencilState - creation of depth/stencil state failed." );

	return depthStencilState;
}

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForMeshRendering( ID3D11Device& device )
{
	ComPtr<ID3D11BlendState> blendState;
	D3D11_BLEND_DESC         blendDesc;

	ZeroMemory( &blendDesc, sizeof( blendDesc ) );

	blendDesc.AlphaToCoverageEnable  = false;
	blendDesc.IndependentBlendEnable = false; // Use same blend settings for each render target (as for render target 0).

	// Disable blending for all render targets.
	const int maxRenderTargetCount = 8;
	for ( int i = 0; i < maxRenderTargetCount; ++i ) {
		blendDesc.RenderTarget[ i ].BlendEnable           = false;
		blendDesc.RenderTarget[ i ].SrcBlend              = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ i ].DestBlend             = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[ i ].BlendOp               = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[ i ].SrcBlendAlpha         = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[ i ].DestBlendAlpha        = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[ i ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[ i ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createBlendStateForMeshRendering - creation of blend state failed." );

	return blendState;
}

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForTextRendering( ID3D11Device& device )
{
	ComPtr<ID3D11BlendState> blendState;
	D3D11_BLEND_DESC         blendDesc;

	ZeroMemory( &blendDesc, sizeof( blendDesc ) );

	blendDesc.AlphaToCoverageEnable  = false;
	blendDesc.IndependentBlendEnable = true; // Use different blend settings for each render target.

    // Disable blending for normal (vector) render target.
	blendDesc.RenderTarget[ 0 ].BlendEnable           = false;
	blendDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	// Enable blending for albedo render target.
	blendDesc.RenderTarget[ 1 ].BlendEnable           = true;
	blendDesc.RenderTarget[ 1 ].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[ 1 ].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[ 1 ].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 1 ].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[ 1 ].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[ 1 ].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[ 1 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; //D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createBlendStateForTextRendering - creation of blend state failed." );

	return blendState;
}

void Direct3DDeferredRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
	// Create render targets.
    m_positionRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_emissiveRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_albedoRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_metalnessRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_roughnessRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_normalRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    m_indexOfRefractionRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

	// Create depth render target.
	m_depthRenderTarget = std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS );
}

void Direct3DDeferredRenderer::clearRenderTargets( float4 color, float depth )
{
    m_positionRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
    m_emissiveRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
	m_albedoRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
    m_metalnessRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
    m_roughnessRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
    m_normalRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );
    m_indexOfRefractionRenderTarget->clearRenderTargetView( *m_deviceContext.Get( ), color );

	m_depthRenderTarget->clearDepthStencilView( *m_deviceContext.Get( ), true, depth, true, 0 );
}

void Direct3DDeferredRenderer::disableRenderTargets()
{
    m_rendererCore.disableRenderTargetViews();
}

void Direct3DDeferredRenderer::loadAndCompileShaders( ID3D11Device& device )
{
	m_blockMeshVertexShader->compileFromFile( "Shaders/BlockMeshShader/vs.hlsl", device );
	m_blockMeshFragmentShader->compileFromFile( "Shaders/BlockMeshShader/ps.hlsl", device );

	m_skeletonMeshVertexShader->compileFromFile( "Shaders/SkeletonMeshShader/vs.hlsl", device );
	m_skeletonMeshFragmentShader->compileFromFile( "Shaders/SkeletonMeshShader/ps.hlsl", device );

	m_blockModelVertexShader->compileFromFile( "Shaders/BlockModelShader/vs.hlsl", device );
	m_blockModelFragmentShader->compileFromFile( "Shaders/BlockModelShader/ps.hlsl", device );

	m_skeletonModelVertexShader->compileFromFile( "Shaders/SkeletonModelShader/vs.hlsl", device );
	m_skeletonModelFragmentShader->compileFromFile( "Shaders/SkeletonModelShader/ps.hlsl", device );

	m_textVertexShader->compileFromFile( "Shaders/TextShader/vs.hlsl", device );
	m_textFragmentShader->compileFromFile( "Shaders/TextShader/ps.hlsl", device );
}

void Direct3DDeferredRenderer::createDefaultTextures( ID3D11Device& device )
{
    std::vector< unsigned char > dataAlpha             = { 255 };
    std::vector< unsigned char > dataMetalness         = { 0 };
    std::vector< unsigned char > dataRoughness         = { 0 };
    std::vector< unsigned char > dataIndexOfRefraction = { 0 };
    std::vector< uchar4 >        dataEmissive          = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataAlbedo            = { uchar4( 128, 128, 128, 255 ) };
    std::vector< uchar4 >        dataNormal            = { uchar4( 128, 128, 255, 255 ) };

    m_defaultAlphaTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultMetalnessTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultRoughnessTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultIndexOfRefractionTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    m_defaultEmissiveTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_defaultAlbedoTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    m_defaultNormalTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
}
