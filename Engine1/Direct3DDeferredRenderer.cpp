#include "Direct3DDeferredRenderer.h"

#include "Direct3DRendererCore.h"

#include "MathUtil.h"

#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"

#include "SkeletonPose.h"

//#include "RenderTargetTexture2D.h"
//#include "RenderTargetDepthTexture2D.h"

#include "Font.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DDeferredRenderer::Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
blockMeshVertexShader(std::make_shared<BlockMeshVertexShader>()),
blockMeshFragmentShader(std::make_shared<BlockMeshFragmentShader>()),
skeletonMeshVertexShader(std::make_shared<SkeletonMeshVertexShader>()),
skeletonMeshFragmentShader(std::make_shared<SkeletonMeshFragmentShader>()),
blockModelVertexShader(std::make_shared<BlockModelVertexShader>()),
blockModelFragmentShader(std::make_shared<BlockModelFragmentShader>()),
skeletonModelVertexShader(std::make_shared<SkeletonModelVertexShader>()),
skeletonModelFragmentShader(std::make_shared<SkeletonModelFragmentShader>()),
textVertexShader(std::make_shared<TextVertexShader>()),
textFragmentShader(std::make_shared<TextFragmentShader>())
{}

Direct3DDeferredRenderer::~Direct3DDeferredRenderer()
{}

void Direct3DDeferredRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext )
{

	this->device = device;
	this->deviceContext = deviceContext;

	this->imageWidth = imageWidth;
	this->imageHeight = imageHeight;

	// Initialize rasterizer state.
	rasterizerState = createRasterizerState( *device.Get() );
	// Initialize depth stencil state.
	depthStencilState = createDepthStencilState( *device.Get() );
	// Initialize blend states.
	blendStateForMeshRendering = createBlendStateForMeshRendering( *device.Get() );
	blendStateForTextRendering = createBlendStateForTextRendering( *device.Get() );

	createRenderTargets( imageWidth, imageHeight, *device.Get() );

	{ // Initialize projection matrices.
		const float fieldOfView = (float)MathUtil::pi / 4.0f;
		const float screenAspect = (float)imageWidth / (float)imageHeight;
		const float zNear = 0.1f;
		const float zFar = 1000.0f;

		perspectiveProjectionMatrix = MathUtil::perspectiveProjectionTransformation( fieldOfView, screenAspect, zNear, zFar );
		orthographicProjectionMatrix = MathUtil::orthographicProjectionTransformation( (float)imageWidth, (float)imageHeight, zNear, zFar );
	}

	loadAndCompileShaders( *device.Get() );

    createDefaultTextures( *device.Get() );

	initialized = true;
}

void Direct3DDeferredRenderer::render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix )
{
	if ( !initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( normalRenderTarget );
        renderTargetsF4.push_back( positionRenderTarget );
        renderTargetsU1.push_back( metalnessRenderTarget );
        renderTargetsU1.push_back( roughnessRenderTarget );
        renderTargetsU1.push_back( indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( emissiveRenderTarget );
        renderTargetsU4.push_back( albedoRenderTarget );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, depthRenderTarget );
	}

	{ // Configure and set shaders.
		blockMeshVertexShader->setParameters( *deviceContext.Get(), worldMatrix, viewMatrix, perspectiveProjectionMatrix );

		rendererCore.enableRenderingShaders( blockMeshVertexShader, blockMeshFragmentShader );
	}

	rendererCore.enableRasterizerState( *rasterizerState.Get( ) );
	rendererCore.enableDepthStencilState( *depthStencilState.Get( ) );
	rendererCore.enableBlendState( *blendStateForMeshRendering.Get( ) );

	// Draw mesh.
	rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace )
{
	if ( !initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( normalRenderTarget );
        renderTargetsF4.push_back( positionRenderTarget );
        renderTargetsU1.push_back( metalnessRenderTarget );
        renderTargetsU1.push_back( roughnessRenderTarget );
        renderTargetsU1.push_back( indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( emissiveRenderTarget );
        renderTargetsU4.push_back( albedoRenderTarget );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, depthRenderTarget );
	}

	{ // Configure and set shaders.
		skeletonMeshVertexShader->setParameters( *deviceContext.Get( ), worldMatrix, viewMatrix, perspectiveProjectionMatrix, mesh, poseInSkeletonSpace );

		rendererCore.enableRenderingShaders( skeletonMeshVertexShader, skeletonMeshFragmentShader );
	}

	rendererCore.enableRasterizerState( *rasterizerState.Get( ) );
	rendererCore.enableDepthStencilState( *depthStencilState.Get( ) );
	rendererCore.enableBlendState( *blendStateForMeshRendering.Get( ) );

	// Draw mesh.
	rendererCore.draw( mesh );
}

void Direct3DDeferredRenderer::render( const BlockModel& model, const float43& worldMatrix, const float44& viewMatrix )
{
	if ( !initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( normalRenderTarget );
        renderTargetsF4.push_back( positionRenderTarget );
        renderTargetsU1.push_back( metalnessRenderTarget );
        renderTargetsU1.push_back( roughnessRenderTarget );
        renderTargetsU1.push_back( indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( emissiveRenderTarget );
        renderTargetsU4.push_back( albedoRenderTarget );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, depthRenderTarget );
	}

	{ // Configure and set shaders.
        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *defaultEmissiveTexture;

	    const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *defaultIndexOfRefractionTexture;

		blockModelVertexShader->setParameters( *deviceContext.Get( ), worldMatrix, viewMatrix, perspectiveProjectionMatrix );
		blockModelFragmentShader->setParameters( *deviceContext.Get( ), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

		rendererCore.enableRenderingShaders( blockModelVertexShader, blockModelFragmentShader );
	}

	rendererCore.enableRasterizerState( *rasterizerState.Get( ) );
	rendererCore.enableDepthStencilState( *depthStencilState.Get( ) );
	rendererCore.enableBlendState( *blendStateForMeshRendering.Get( ) );

	// Draw mesh.
	rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( const SkeletonModel& model, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace )
{
	if ( !initialized ) throw std::exception( "Direct3DDeferredRenderer::render - renderer not initialized." );
	if ( !model.getMesh( ) ) throw std::exception( "Direct3DDeferredRenderer::render - model has no mesh." );

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( normalRenderTarget );
        renderTargetsF4.push_back( positionRenderTarget );
        renderTargetsU1.push_back( metalnessRenderTarget );
        renderTargetsU1.push_back( roughnessRenderTarget );
        renderTargetsU1.push_back( indexOfRefractionRenderTarget );
        renderTargetsU4.push_back( emissiveRenderTarget );
        renderTargetsU4.push_back( albedoRenderTarget );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, depthRenderTarget );
	}

	{ // Configure and set shaders.
        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture 
            = model.getAlphaTexturesCount() > 0 ? *model.getAlphaTexture( 0 ).getTexture() : *defaultAlphaTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture 
            = model.getEmissionTexturesCount() > 0 ? *model.getEmissionTexture( 0 ).getTexture() : *defaultEmissiveTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture 
            = model.getAlbedoTexturesCount() > 0 ? *model.getAlbedoTexture( 0 ).getTexture() : *defaultAlbedoTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture 
            = model.getNormalTexturesCount() > 0 ? *model.getNormalTexture( 0 ).getTexture() : *defaultNormalTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture 
            = model.getMetalnessTexturesCount() > 0 ? *model.getMetalnessTexture( 0 ).getTexture() : *defaultMetalnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture 
            = model.getRoughnessTexturesCount() > 0 ? *model.getRoughnessTexture( 0 ).getTexture() : *defaultRoughnessTexture;

        const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture 
            = model.getIndexOfRefractionTexturesCount() > 0 ? *model.getIndexOfRefractionTexture( 0 ).getTexture() : *defaultIndexOfRefractionTexture;

		skeletonModelVertexShader->setParameters( *deviceContext.Get( ), worldMatrix, viewMatrix, perspectiveProjectionMatrix, *model.getMesh( ), poseInSkeletonSpace );
		skeletonModelFragmentShader->setParameters( *deviceContext.Get(), alphaTexture, emissiveTexture, albedoTexture, normalTexture, metalnessTexture, roughnessTexture, indexOfRefractionTexture );

		rendererCore.enableRenderingShaders( skeletonModelVertexShader, skeletonModelFragmentShader );
	}

	rendererCore.enableRasterizerState( *rasterizerState.Get() );
	rendererCore.enableDepthStencilState( *depthStencilState.Get() );
	rendererCore.enableBlendState( *blendStateForMeshRendering.Get() );

	// Draw mesh.
	rendererCore.draw( *model.getMesh().get() );
}

void Direct3DDeferredRenderer::render( const std::string& text, Font& font, float2 position, float4 color )
{
	if ( !initialized ) throw std::exception( "Direct3DDeferredRenderer::renders - renderer not initialized." );

    color; // Unused.

	{ // Enable render targets.
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >        renderTargetsF2;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >        renderTargetsF4;
        std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > > renderTargetsU1;
		std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >        renderTargetsU4;

        renderTargetsF4.push_back( normalRenderTarget );
        renderTargetsU4.push_back( albedoRenderTarget );

		rendererCore.enableRenderTargets( renderTargetsF2, renderTargetsF4, renderTargetsU1, renderTargetsU4, depthRenderTarget );
	}

	rendererCore.enableRenderingShaders( textVertexShader, textFragmentShader );
	rendererCore.enableRasterizerState( *rasterizerState.Get( ) );
	rendererCore.enableDepthStencilState( *depthStencilState.Get( ) );
	rendererCore.enableBlendState( *blendStateForTextRendering.Get( ) );

	float43 worldMatrix;
	worldMatrix.identity();
	float44 viewMatrix;
	viewMatrix.identity();

	const char *charText = text.c_str(), *p;
	const FontCharacter* character = nullptr;
	float2 pos = position;

	for ( p = charText; *p; p++ ) {
		character = font.getCharacter( *p, *device.Get( ) );

		if ( character ) {
			if ( character->getCharcode() == '\n' ) {
				pos.y -= character->getSize().y;
				pos.x = position.x;
			} else {
				worldMatrix.setTranslation( float3( pos.x + character->getPos().x, pos.y + character->getPos().y, 0.0f ) );

				// Configure the shaders.
				textVertexShader->setParameters( *deviceContext.Get( ), worldMatrix, viewMatrix, orthographicProjectionMatrix );
				textFragmentShader->setParameters( *deviceContext.Get( ), character->getTextureResource( ) );

				// Draw the character.
				rendererCore.draw( *character );

				pos.x += character->getAdvance().x;
				pos.y += character->getAdvance().y;
			}
		}
	}
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > > Direct3DDeferredRenderer::getPositionRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getPositionRenderTarget - renderer not initialized." );

	return positionRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getEmissiveRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getEmissiveRenderTarget - renderer not initialized." );

	return emissiveRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getAlbedoRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getAlbedoRenderTarget - renderer not initialized." );

	return albedoRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getMetalnessRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getMetalnessRenderTarget - renderer not initialized." );

	return metalnessRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getRoughnessRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getRoughnessRenderTarget - renderer not initialized." );

	return roughnessRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > > Direct3DDeferredRenderer::getNormalRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getNormalRenderTarget - renderer not initialized." );

	return normalRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > Direct3DDeferredRenderer::getIndexOfRefractionRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getIndexOfRefractionRenderTarget - renderer not initialized." );

	return indexOfRefractionRenderTarget;
}

std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, uchar4 > > Direct3DDeferredRenderer::getDepthRenderTarget()
{
	if ( !initialized ) 
        throw std::exception( "Direct3DDeferredRenderer::getRenderTarget - renderer not initialized." );

	return depthRenderTarget;
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
	blendDesc.RenderTarget[ 1 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE; // Don't write alpha.

	HRESULT result = device.CreateBlendState( &blendDesc, blendState.ReleaseAndGetAddressOf() );
	if ( result < 0 ) throw std::exception( "Direct3DRenderer::createBlendStateForTextRendering - creation of blend state failed." );

	return blendState;
}

void Direct3DDeferredRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
	// Create render targets.
    positionRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    emissiveRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    albedoRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    metalnessRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    roughnessRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    normalRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT );

    indexOfRefractionRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

	// Create depth render target.
	depthRenderTarget = std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >
        ( device, imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS );
}

void Direct3DDeferredRenderer::clearRenderTargets( float4 color, float depth )
{
    positionRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
    emissiveRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
	albedoRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
    metalnessRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
    roughnessRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
    normalRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );
    indexOfRefractionRenderTarget->clearRenderTargetView( *deviceContext.Get( ), color );

	depthRenderTarget->clearDepthStencilView( *deviceContext.Get( ), true, depth, true, 0 );
}

void Direct3DDeferredRenderer::disableRenderTargets()
{
    rendererCore.disableRenderTargetViews();
}

void Direct3DDeferredRenderer::loadAndCompileShaders( ID3D11Device& device )
{
	blockMeshVertexShader->compileFromFile( "Shaders/BlockMeshShader/vs.hlsl", device );
	blockMeshFragmentShader->compileFromFile( "Shaders/BlockMeshShader/ps.hlsl", device );

	skeletonMeshVertexShader->compileFromFile( "Shaders/SkeletonMeshShader/vs.hlsl", device );
	skeletonMeshFragmentShader->compileFromFile( "Shaders/SkeletonMeshShader/ps.hlsl", device );

	blockModelVertexShader->compileFromFile( "Shaders/BlockModelShader/vs.hlsl", device );
	blockModelFragmentShader->compileFromFile( "Shaders/BlockModelShader/ps.hlsl", device );

	skeletonModelVertexShader->compileFromFile( "Shaders/SkeletonModelShader/vs.hlsl", device );
	skeletonModelFragmentShader->compileFromFile( "Shaders/SkeletonModelShader/ps.hlsl", device );

	textVertexShader->compileFromFile( "Shaders/TextShader/vs.hlsl", device );
	textFragmentShader->compileFromFile( "Shaders/TextShader/ps.hlsl", device );
}

void Direct3DDeferredRenderer::createDefaultTextures( ID3D11Device& device )
{
    std::vector< unsigned char > dataAlpha             = { 255 };
    std::vector< unsigned char > dataMetalness         = { 180 };
    std::vector< unsigned char > dataRoughness         = { 150 };
    std::vector< unsigned char > dataIndexOfRefraction = { 120 };
    std::vector< uchar4 >        dataEmissive          = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataAlbedo            = { uchar4( 0, 0, 0, 255 ) };
    std::vector< uchar4 >        dataNormal            = { uchar4( 128, 128, 255, 0 ) };

    defaultAlphaTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataAlpha, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultMetalnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataMetalness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultRoughnessTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataRoughness, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultIndexOfRefractionTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
        ( device, dataIndexOfRefraction, 1, 1, false, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );

    defaultEmissiveTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataEmissive, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    defaultAlbedoTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataAlbedo, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );

    defaultNormalTexture = std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( device, dataNormal, 1, 1, false, true, false, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
}
