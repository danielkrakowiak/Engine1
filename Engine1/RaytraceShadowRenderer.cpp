#include "RaytraceShadowRenderer.h"

#include <d3d11.h>

#include "Direct3DRendererCore.h"
#include "RaytracingShadowsComputeShader.h"
#include "uint3.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"

using namespace Engine1;
using Microsoft::WRL::ComPtr;

RaytraceShadowRenderer::RaytraceShadowRenderer(Direct3DRendererCore& rendererCore) :
	m_rendererCore( rendererCore ),
	m_initialized( false ),
	m_imageWidth( 0 ),
	m_imageHeight( 0 ),
	m_raytracingShadowsComputeShader( std::make_shared< RaytracingShadowsComputeShader >() )
{
}

RaytraceShadowRenderer::~RaytraceShadowRenderer()
{
}

void RaytraceShadowRenderer::initialize(
	int imageWidth,
	int imageHeight,
	Microsoft::WRL::ComPtr< ID3D11Device > device,
	Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext )
{
	m_device        = device;
	m_deviceContext = deviceContext;

	m_imageWidth  = imageWidth;
	m_imageHeight = imageHeight;

	loadAndCompileShaders( device );

	createDefaultTextures( *device.Get() );

	m_initialized = true;
}

void RaytraceShadowRenderer::generateAndTraceShadowRays(
	const std::shared_ptr< Light > light,
	const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
	const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > surfaceNormalTexture,
	const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
    //const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > preIlluminationTexture,]
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > hardShadowRenderTarget,
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > softShadowRenderTarget,
    std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > >         distanceToOccluderRenderTarget,
	const std::vector< std::shared_ptr< BlockActor > >& actors
)
{
	m_rendererCore.disableRenderingPipeline();

	m_rendererCore.enableComputeShader( m_raytracingShadowsComputeShader );

	// Don't have to clear, because illumination should be initialized with pre-illumination.
	//m_hardIlluminationTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 255, 255, 255 ) );
    //m_softIlluminationTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 255, 255, 255 ) );

    hardShadowRenderTarget->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO );
    softShadowRenderTarget->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO );
    distanceToOccluderRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 1000.0f ) );

    // Copy pre-illumination texture as initial illumination. 
    // This is to avoid copying values inside the shader in each pass.
    ///*m_rendererCore.copyTexture(
    //    *m_illuminationTexture,
    //    *preIlluminationTexture
    //);*/

	std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
	std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
	std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
	std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
	std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( distanceToOccluderRenderTarget );
	unorderedAccessTargetsU1.push_back( hardShadowRenderTarget );
    unorderedAccessTargetsU1.push_back( softShadowRenderTarget );

	m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, 
        unorderedAccessTargetsF2, 
        unorderedAccessTargetsF4, 
        unorderedAccessTargetsU1, 
        unorderedAccessTargetsU4 
    );

	const int imageWidth  = m_imageWidth; //rayOriginTexture->getWidth();
	const int imageHeight = m_imageHeight; //rayOriginTexture->getHeight();

	uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    std::vector< std::shared_ptr< const BlockActor > > actorsToPass;
    int actorIdx = 0;

	while ( actorIdx < actors.size() )
    {
        //#TODO: Should we only pass actors which cast shadows?
        // Gather a few actors to pass to the shader. 
        actorsToPass.clear();
        while ( actorsToPass.size() < RaytracingShadowsComputeShader::s_maxActorCount && actorIdx < actors.size() )
        {
            if ( actors[ actorIdx ]->isCastingShadows() )
                actorsToPass.push_back( actors[ actorIdx ] );

            ++actorIdx;
        }

		m_raytracingShadowsComputeShader->setParameters( 
			*m_deviceContext.Get(), 
            *light, 
            *rayOriginTexture, 
            *surfaceNormalTexture, 
            /*preIlluminationTexture, */
            actorsToPass, 
            *m_defaultAlphaTexture, 
            imageWidth, 
            imageHeight 
		);

		m_rendererCore.compute( groupCount );
	}

	// Unbind resources to avoid binding the same resource on input and output.
	m_rendererCore.disableUnorderedAccessViews();

	m_raytracingShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

	m_rendererCore.disableComputePipeline();
}

void RaytraceShadowRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
	m_raytracingShadowsComputeShader->loadAndInitialize( "Shaders/RaytracingShadowsShader/RaytracingShadows_cs.cso", device );
}

void RaytraceShadowRenderer::createDefaultTextures( ID3D11Device& device )
{
	std::vector< unsigned char > dataAlpha = { 255 };

	m_defaultAlphaTexture = std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > >
		( device, dataAlpha, 1, 1, true, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
}

