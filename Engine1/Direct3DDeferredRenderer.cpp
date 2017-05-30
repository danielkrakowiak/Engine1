#include "Direct3DDeferredRenderer.h"

#include "Direct3DRendererCore.h"

#include "MathUtil.h"

#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"

#include "SkeletonPose.h"

#include "Font.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DDeferredRenderer::Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false )
{}

Direct3DDeferredRenderer::~Direct3DDeferredRenderer()
{}

void Direct3DDeferredRenderer::initialize( 
    ComPtr< ID3D11Device > device, 
    ComPtr< ID3D11DeviceContext > deviceContext )
{
	this->m_device = device;
	this->m_deviceContext = deviceContext;

	m_rasterizerState                       = createRasterizerState( *device.Get() );
    m_wireframeRasterizerState              = createWireframeRasterizerState( *device.Get() );
	m_depthStencilState                     = createDepthStencilState( *device.Get() );
	m_blendStateForMeshRendering            = createBlendStateForMeshRendering( *device.Get() );
    m_blendStateForTransparentMeshRendering = createBlendStateForTransparentMeshRendering( *device.Get() );
	m_blendStateForTextRendering            = createBlendStateForTextRendering( *device.Get() );

	loadAndCompileShaders( device );

    createDefaultTextures( *device.Get() );

	m_initialized = true;
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const BlockMesh& mesh, 
    const float43& worldMatrix, 
    const float44& viewMatrix )
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation( 
        settings.fieldOfView, 
        settings.imageDimensions.x / settings.imageDimensions.y, 
        settings.zNear, 
        settings.zFar 
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	// Configure and set shaders.
    m_blockMeshVertexShader.setParameters(
        *m_deviceContext.Get(),
        worldMatrix,
        viewMatrix,
        projectionMatrix
    );

    m_rendererCore.enableRenderingShaders(
        m_blockMeshVertexShader, m_blockMeshFragmentShader
    );

	m_rendererCore.enableRasterizerState( 
        settings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() 
    );

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::renderEmissive( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const BlockMesh& mesh, 
    const float43& worldMatrix, 
    const float44& viewMatrix )
{
    if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::renderEmissive - renderer not initialized." );

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation(
        settings.fieldOfView,
        settings.imageDimensions.x / settings.imageDimensions.y,
        settings.zNear, 
        settings.zFar
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

    // Configure and set shaders.
    m_blockMeshVertexShader.setParameters( 
        *m_deviceContext.Get(), worldMatrix, viewMatrix, projectionMatrix 
    );

    m_rendererCore.enableRenderingShaders( m_blockMeshVertexShader, m_blockMeshEmissiveFragmentShader );

    m_rendererCore.enableRasterizerState( 
        settings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() 
    );

    m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
    m_rendererCore.enableBlendState( *m_blendStateForTransparentMeshRendering.Get() );

    // Draw mesh.
    m_rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const SkeletonMesh& mesh, 
    const float43& worldMatrix, 
    const float44& viewMatrix, 
    const SkeletonPose& poseInSkeletonSpace )
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation(
        settings.fieldOfView,
        settings.imageDimensions.x / settings.imageDimensions.y,
        settings.zNear,
        settings.zFar
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	// Configure and set shaders.
    m_skeletonMeshVertexShader.setParameters(
        *m_deviceContext.Get(),
        worldMatrix,
        viewMatrix,
        projectionMatrix,
        mesh,
        poseInSkeletonSpace
    );

    m_rendererCore.enableRenderingShaders( m_skeletonMeshVertexShader, m_skeletonMeshFragmentShader );

	m_rendererCore.enableRasterizerState( 
        settings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() 
    );

	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const BlockModel& model, 
    const float43& worldMatrix, 
    const float44& viewMatrix, 
    const float4& extraEmissive )
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

    if ( !model.getMesh( ) ) 
        throw std::exception( "Direct3DDeferredRenderer::render - model has no mesh." );

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation(
        settings.fieldOfView,
        settings.imageDimensions.x / settings.imageDimensions.y,
        settings.zNear,
        settings.zFar
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	{ // Configure and set shaders.
        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ONE;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 1.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 1.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 1.0f;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = !model.getAlphaTextures().empty() ? *model.getAlphaTextures()[ 0 ].getTexture() : *m_defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() ? *model.getEmissiveTextures()[ 0 ].getTexture() : *m_defaultEmissiveTexture;

	    const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() ? *model.getAlbedoTextures()[ 0 ].getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() ? *model.getNormalTextures()[ 0 ].getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() ? *model.getMetalnessTextures()[ 0 ].getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() ? *model.getRoughnessTextures()[ 0 ].getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() : *m_defaultIndexOfRefractionTexture;

		m_blockModelVertexShader.setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, projectionMatrix );

		m_blockModelFragmentShader.setParameters( 
            *m_deviceContext.Get( ), 
            alphaTexture, alphaMul,
            emissiveTexture, emissiveMul,
            albedoTexture, albedoMul,
            normalTexture, normalMul,
            metalnessTexture, metalnessMul,
            roughnessTexture, roughnessMul,
            indexOfRefractionTexture, indexOfRefractionMul,
            extraEmissive 
        );

		m_rendererCore.enableRenderingShaders( m_blockModelVertexShader, m_blockModelFragmentShader );
	}

	m_rendererCore.enableRasterizerState( settings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const SkeletonModel& model, 
    const float43& worldMatrix, 
    const float44& viewMatrix, 
    const SkeletonPose& poseInSkeletonSpace, 
    const float4& extraEmissive )
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	if ( !model.getMesh( ) ) 
        throw std::exception( "Direct3DDeferredRenderer::render - model has no mesh." );

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation(
        settings.fieldOfView,
        settings.imageDimensions.x / settings.imageDimensions.y,
        settings.zNear,
        settings.zFar
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	{ // Configure and set shaders.
        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ONE;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 1.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 1.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 1.0f;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = !model.getAlphaTextures().empty() ? *model.getAlphaTextures()[ 0 ].getTexture() : *m_defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() ? *model.getEmissiveTextures()[ 0 ].getTexture() : *m_defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() ? *model.getAlbedoTextures()[ 0 ].getTexture() : *m_defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() ? *model.getNormalTextures()[ 0 ].getTexture() : *m_defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() ? *model.getMetalnessTextures()[ 0 ].getTexture() : *m_defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() ? *model.getRoughnessTextures()[ 0 ].getTexture() : *m_defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() : *m_defaultIndexOfRefractionTexture;

		m_skeletonModelVertexShader.setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, projectionMatrix, *model.getMesh( ), poseInSkeletonSpace );
		m_skeletonModelFragmentShader.setParameters( *m_deviceContext.Get(), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture, extraEmissive );

		m_rendererCore.enableRenderingShaders( m_skeletonModelVertexShader, m_skeletonModelFragmentShader );
	}

	m_rendererCore.enableRasterizerState( settings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& settings, 
    const std::string& text, 
    Font& font, 
    float2 position, 
    float4 color )
{
	if ( !m_initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::renders - renderer not initialized." );

    const float44 projectionMatrix = MathUtil::orthographicProjectionTransformation( 
        settings.imageDimensions.x, 
        settings.imageDimensions.y, 
        settings.zNear, 
        settings.zFar 
    );

    m_rendererCore.setViewport( settings.imageDimensions );

    color; // Unused.

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >         renderTargetsF1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( renderTargets.normal );
        renderTargetsU4.push_back( renderTargets.albedo );

		m_rendererCore.enableRenderTargets( renderTargetsF1, renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, renderTargets.depth );
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
				m_textVertexShader.setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, projectionMatrix );
				m_textFragmentShader.setParameters( *m_deviceContext.Get( ), character->getTextureResource( ) );

				// Draw the character.
				m_rendererCore.draw( *character );

				pos.x += character->getAdvance().x;
				pos.y += character->getAdvance().y;
			}
		}
	}
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
	for ( int i = 0; i < maxRenderTargetCount; ++i ) 
    {
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

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForTransparentMeshRendering( ID3D11Device& device )
{
    ComPtr<ID3D11BlendState> blendState;
    D3D11_BLEND_DESC         blendDesc;

    ZeroMemory( &blendDesc, sizeof( blendDesc ) );

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false; // Use same blend settings for each render target (as for render target 0).

    // Enable blending for all render targets.
    const int maxRenderTargetCount = 8;
    for ( int i = 0; i < maxRenderTargetCount; ++i ) 
    {
        blendDesc.RenderTarget[ i ].BlendEnable           = true;
        blendDesc.RenderTarget[ i ].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[ i ].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
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

void Direct3DDeferredRenderer::disableRenderTargets()
{
    m_rendererCore.disableRenderTargetViews();
}

void Direct3DDeferredRenderer::enableRenderTargets( const RenderTargets& renderTargets )
{
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >         renderTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

    renderTargetsF4.push_back( renderTargets.normal );
    renderTargetsF4.push_back( renderTargets.position );
    renderTargetsU1.push_back( renderTargets.metalness );
    renderTargetsU1.push_back( renderTargets.roughness );
    renderTargetsU1.push_back( renderTargets.refractiveIndex );
    renderTargetsU4.push_back( renderTargets.emissive );
    renderTargetsU4.push_back( renderTargets.albedo );

    m_rendererCore.enableRenderTargets(
        renderTargetsF1,
        renderTargetsF2,
        renderTargetsF4,
        renderTargetsU1,
        renderTargetsU4,
        renderTargets.depth
    );
}

void Direct3DDeferredRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
	m_blockMeshVertexShader.loadAndInitialize( "Shaders/BlockMeshShader/BlockMesh_vs.cso", device );
    m_blockMeshFragmentShader.loadAndInitialize( "Shaders/BlockMeshShader/BlockMesh_ps.cso", device );
    m_blockMeshEmissiveFragmentShader.loadAndInitialize( "Shaders/BlockMeshShader/BlockMesh_ps_emissive.cso", device );

	m_skeletonMeshVertexShader.loadAndInitialize( "Shaders/SkeletonMeshShader/SkeletonMesh_vs.cso", device );
	m_skeletonMeshFragmentShader.loadAndInitialize( "Shaders/SkeletonMeshShader/SkeletonMesh_ps.cso", device );

	m_blockModelVertexShader.loadAndInitialize( "Shaders/BlockModelShader/BlockModel_vs.cso", device );
	m_blockModelFragmentShader.loadAndInitialize( "Shaders/BlockModelShader/BlockModel_ps.cso", device );

	m_skeletonModelVertexShader.loadAndInitialize( "Shaders/SkeletonModelShader/SkeletonModel_vs.cso", device );
	m_skeletonModelFragmentShader.loadAndInitialize( "Shaders/SkeletonModelShader/SkeletonModel_ps.cso", device );

	m_textVertexShader.loadAndInitialize( "Shaders/TextShader/Text_vs.cso", device );
	m_textFragmentShader.loadAndInitialize( "Shaders/TextShader/Text_ps.cso", device );
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
