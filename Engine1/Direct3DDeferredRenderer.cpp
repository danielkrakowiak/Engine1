#include "Direct3DDeferredRenderer.h"

#include "Direct3DRendererCore.h"

#include "MathUtil.h"

#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"

#include "SkeletonPose.h"

#include "Font.h"
#include "Settings.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DDeferredRenderer::Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false )
{}

Direct3DDeferredRenderer::~Direct3DDeferredRenderer()
{}

void Direct3DDeferredRenderer::initialize( 
    ComPtr< ID3D11Device3 > device, 
    ComPtr< ID3D11DeviceContext3 > deviceContext )
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
    const Settings& renderSettings, 
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
        renderSettings.fieldOfView,
        renderSettings.imageDimensions.x / renderSettings.imageDimensions.y,
        renderSettings.zNear,
        renderSettings.zFar
    );

    m_rendererCore.setViewport( renderSettings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	{ // Configure and set shaders.
        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ZERO;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 0.0f;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = !model.getAlphaTextures().empty() && model.getAlphaTextures()[ 0 ].getTexture()
            ? *model.getAlphaTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.alpha;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() && model.getEmissiveTextures()[ 0 ].getTexture()
            ? *model.getEmissiveTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.emissive;

	    const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() && model.getAlbedoTextures()[ 0 ].getTexture()
            ? *model.getAlbedoTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.albedo;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() && model.getNormalTextures()[ 0 ].getTexture()
            ? *model.getNormalTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.normal;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() && model.getMetalnessTextures()[ 0 ].getTexture()
            ? *model.getMetalnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.metalness;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() && model.getRoughnessTextures()[ 0 ].getTexture()
            ? *model.getRoughnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.roughness;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() && model.getRefractiveIndexTextures()[ 0 ].getTexture()
            ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.refractiveIndex;

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

	m_rendererCore.enableRasterizerState( renderSettings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get() );
	m_rendererCore.enableBlendState( *m_blendStateForMeshRendering.Get() );

	// Draw mesh.
	m_rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( 
    const RenderTargets& renderTargets, 
    const Settings& renderSettings, 
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
        renderSettings.fieldOfView,
        renderSettings.imageDimensions.x / renderSettings.imageDimensions.y,
        renderSettings.zNear,
        renderSettings.zFar
    );

    m_rendererCore.setViewport( renderSettings.imageDimensions );

    // Enable render targets.
    enableRenderTargets( renderTargets );

	{ // Configure and set shaders.
        const float  alphaMul             = !model.getAlphaTextures().empty()           ? model.getAlphaTextures()[ 0 ].getColorMultiplier().x           : 1.0f;
        const float3 emissiveMul          = !model.getEmissiveTextures().empty()        ? model.getEmissiveTextures()[ 0 ].getColorMultiplier()          : float3::ZERO;
        const float3 albedoMul            = !model.getAlbedoTextures().empty()          ? model.getAlbedoTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float3 normalMul            = !model.getNormalTextures().empty()          ? model.getNormalTextures()[ 0 ].getColorMultiplier()            : float3::ONE;
        const float  metalnessMul         = !model.getMetalnessTextures().empty()       ? model.getMetalnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  roughnessMul         = !model.getRoughnessTextures().empty()       ? model.getRoughnessTextures()[ 0 ].getColorMultiplier().x       : 0.0f;
        const float  indexOfRefractionMul = !model.getRefractiveIndexTextures().empty() ? model.getRefractiveIndexTextures()[ 0 ].getColorMultiplier().x : 0.0f;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = !model.getAlphaTextures().empty() && model.getAlphaTextures()[ 0 ].getTexture()
            ? *model.getAlphaTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.alpha;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = !model.getEmissiveTextures().empty() && model.getEmissiveTextures()[ 0 ].getTexture()
            ? *model.getEmissiveTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.emissive;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = !model.getAlbedoTextures().empty() && model.getAlbedoTextures()[ 0 ].getTexture()
            ? *model.getAlbedoTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.albedo;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = !model.getNormalTextures().empty() && model.getNormalTextures()[ 0 ].getTexture()
            ? *model.getNormalTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.normal;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = !model.getMetalnessTextures().empty() && model.getMetalnessTextures()[ 0 ].getTexture()
            ? *model.getMetalnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.metalness;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = !model.getRoughnessTextures().empty() && model.getRoughnessTextures()[ 0 ].getTexture()
            ? *model.getRoughnessTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.roughness;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = !model.getRefractiveIndexTextures().empty() && model.getRefractiveIndexTextures()[ 0 ].getTexture()
            ? *model.getRefractiveIndexTextures()[ 0 ].getTexture() 
            : *settings().textures.defaults.refractiveIndex;

		m_skeletonModelVertexShader.setParameters( *m_deviceContext.Get( ), worldMatrix, viewMatrix, projectionMatrix, *model.getMesh( ), poseInSkeletonSpace );
		m_skeletonModelFragmentShader.setParameters( *m_deviceContext.Get(), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture, extraEmissive );

		m_rendererCore.enableRenderingShaders( m_skeletonModelVertexShader, m_skeletonModelFragmentShader );
	}

	m_rendererCore.enableRasterizerState( renderSettings.wireframeMode ? *m_wireframeRasterizerState.Get() : *m_rasterizerState.Get() );
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

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >         renderTargetsF1;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float3 > > >        renderTargetsF3;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( renderTargets.normal );
        renderTargetsU4.push_back( renderTargets.albedo );

		m_rendererCore.enableRenderTargets( renderTargetsF1, renderTargetsF2, renderTargetsF3, renderTargetsF4, renderTargetsU1, renderTargetsU4, renderTargets.depth );
	}

	m_rendererCore.enableRenderingShaders( m_textVertexShader, m_textFragmentShader );
	m_rendererCore.enableRasterizerState( *m_rasterizerState.Get( ) );
	m_rendererCore.enableDepthStencilState( *m_depthStencilState.Get( ) );
	m_rendererCore.enableBlendState( *m_blendStateForTextRendering.Get( ) );

	float43 worldMatrix;
	worldMatrix.identity();
	float44 viewMatrix;
	viewMatrix.identity();

    const std::string colorTag         = "<color %f,%f,%f>";
    const std::string colorTagStart    = "<color ";
    const std::string colorClosingTag  = "<color/>";
    float4            colorTagValue    = float4::ONE;
    bool              useColorTagValue = false;

	const char* charText = text.c_str(), *p;
    const char* charTextEnd = charText + text.size();
	const FontCharacter* character = nullptr;
	float2 pos = position;

	for ( p = charText; *p; p++ ) 
    {
        // Look for color tag.
        if ((size_t)(charTextEnd - p) > colorTag.size() 
            && strncmp( p, colorTagStart.c_str(), colorTagStart.size() ) == 0)
        {
            p += colorTagStart.size();

            // Parse color tag.
            useColorTagValue = ( sscanf_s( p, "%f,%f,%f", &colorTagValue.x, &colorTagValue.y, &colorTagValue.z) == 3 );

            // Skip the rest of color tag.
            while ( *p != '>' && *p && *(p + 1) ) {
                ++p;
            }

            continue;
        }

        // Look for color closing tag.
        if ((size_t)(charTextEnd - p) > colorClosingTag.size() 
            && strncmp( p, colorClosingTag.c_str(), colorClosingTag.size() ) == 0) 
        {
            useColorTagValue = false;
            p += (colorClosingTag.size() - 1);

            continue;
        }

        character = font.getCharacter( *p, *m_device.Get( ) );

		if ( character ) {
			if ( character->getCharcode() == '\n' ) {
				pos.y -= ( font.getLineHeight() / 64 );
				pos.x = position.x;
			} else {
				worldMatrix.setTranslation( float3( pos.x + character->getPos().x, pos.y + character->getPos().y, 0.0f ) );

				// Configure the shaders.
				m_textVertexShader.setParameters(
                    *m_deviceContext.Get( ), 
                    worldMatrix, 
                    viewMatrix, 
                    projectionMatrix 
                );
				
                m_textFragmentShader.setParameters( 
                    *m_deviceContext.Get( ), 
                    character->getTextureResource( ), 
                    (useColorTagValue ? colorTagValue : color)
                );

				// Draw the character.
				m_rendererCore.draw( *character );

				pos.x += character->getAdvance().x;
				pos.y += character->getAdvance().y;
			}
		}
	}

    m_textFragmentShader.unsetParameters( *m_deviceContext.Get() );
}

ComPtr<ID3D11RasterizerState> Direct3DDeferredRenderer::createRasterizerState( ID3D11Device3& device )
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

ComPtr<ID3D11RasterizerState> Direct3DDeferredRenderer::createWireframeRasterizerState( ID3D11Device3& device )
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

ComPtr<ID3D11DepthStencilState> Direct3DDeferredRenderer::createDepthStencilState( ID3D11Device3& device )
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

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForMeshRendering( ID3D11Device3& device )
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

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForTransparentMeshRendering( ID3D11Device3& device )
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

ComPtr<ID3D11BlendState> Direct3DDeferredRenderer::createBlendStateForTextRendering( ID3D11Device3& device )
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
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float3 > > >        renderTargetsF3;
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
        renderTargetsF3,
        renderTargetsF4,
        renderTargetsU1,
        renderTargetsU4,
        renderTargets.depth
    );
}

void Direct3DDeferredRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
	m_blockMeshVertexShader.loadAndInitialize( "Engine1/Shaders/BlockMeshShader/BlockMesh_vs.cso", device );
    m_blockMeshFragmentShader.loadAndInitialize( "Engine1/Shaders/BlockMeshShader/BlockMesh_ps.cso", device );
    m_blockMeshEmissiveFragmentShader.loadAndInitialize( "Engine1/Shaders/BlockMeshShader/BlockMesh_ps_emissive.cso", device );

	m_skeletonMeshVertexShader.loadAndInitialize( "Engine1/Shaders/SkeletonMeshShader/SkeletonMesh_vs.cso", device );
	m_skeletonMeshFragmentShader.loadAndInitialize( "Engine1/Shaders/SkeletonMeshShader/SkeletonMesh_ps.cso", device );

	m_blockModelVertexShader.loadAndInitialize( "Engine1/Shaders/BlockModelShader/BlockModel_vs.cso", device );
	m_blockModelFragmentShader.loadAndInitialize( "Engine1/Shaders/BlockModelShader/BlockModel_ps.cso", device );

	m_skeletonModelVertexShader.loadAndInitialize( "Engine1/Shaders/SkeletonModelShader/SkeletonModel_vs.cso", device );
	m_skeletonModelFragmentShader.loadAndInitialize( "Engine1/Shaders/SkeletonModelShader/SkeletonModel_ps.cso", device );

	m_textVertexShader.loadAndInitialize( "Engine1/Shaders/TextShader/Text_vs.cso", device );
	m_textFragmentShader.loadAndInitialize( "Engine1/Shaders/TextShader/Text_ps.cso", device );
}