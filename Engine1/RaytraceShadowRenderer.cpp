#include "RaytraceShadowRenderer.h"

#include <d3d11_3.h>

#include "DX11RendererCore.h"
#include "RaytracingShadowsComputeShader.h"
#include "uint3.h"
#include "MathUtil.h"
#include "BlockModel.h"
#include "BlockActor.h"
#include "Camera.h"
#include "Texture2Dtypes.h"

#include "Settings.h"

using namespace Engine1;
using Microsoft::WRL::ComPtr;

RaytraceShadowRenderer::RaytraceShadowRenderer(DX11RendererCore& rendererCore) :
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
	Microsoft::WRL::ComPtr< ID3D11Device3 > device,
	Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext )
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
	const Camera& camera,
    const std::shared_ptr< Light > light,
	const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
	const std::shared_ptr< Texture2D< float4 > > surfaceNormalTexture,
	const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture,
    //const std::shared_ptr< Texture2D< unsigned char > > preIlluminationTexture,]
    std::shared_ptr< RenderTargetTexture2D< unsigned char > > hardShadowRenderTarget,
    std::shared_ptr< RenderTargetTexture2D< unsigned char > > mediumShadowRenderTarget,
    std::shared_ptr< RenderTargetTexture2D< unsigned char > > softShadowRenderTarget,
    std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderHardShadowRenderTarget,
    std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderMediumShadowRenderTarget,
    std::shared_ptr< RenderTargetTexture2D< float > >         distanceToOccluderSoftShadowRenderTarget,
	const std::vector< std::shared_ptr< BlockActor > >& actors
)
{
	m_rendererCore.disableRenderingPipeline();

	m_rendererCore.enableComputeShader( m_raytracingShadowsComputeShader );

	// Don't have to clear, because illumination should be initialized with pre-illumination.
	//m_hardIlluminationTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 255, 255, 255 ) );
    //m_softIlluminationTexture->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4( 255, 255, 255, 255 ) );

    hardShadowRenderTarget->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO );
    mediumShadowRenderTarget->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO );
    softShadowRenderTarget->clearUnorderedAccessViewUint( *m_deviceContext.Get(), uint4::ZERO );
    distanceToOccluderHardShadowRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 1000.0f ) );
    distanceToOccluderMediumShadowRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 1000.0f ) );
    distanceToOccluderSoftShadowRenderTarget->clearUnorderedAccessViewFloat( *m_deviceContext.Get(), float4( 1000.0f ) );

    // Copy pre-illumination texture as initial illumination. 
    // This is to avoid copying values inside the shader in each pass.
    ///*m_rendererCore.copyTexture(
    //    *m_illuminationTexture,
    //    *preIlluminationTexture
    //);*/

	RenderTargets unorderedAccessTargets;

    unorderedAccessTargets.typeFloat.push_back( distanceToOccluderHardShadowRenderTarget );
    unorderedAccessTargets.typeFloat.push_back( distanceToOccluderMediumShadowRenderTarget );
    unorderedAccessTargets.typeFloat.push_back( distanceToOccluderSoftShadowRenderTarget );
	unorderedAccessTargets.typeUchar.push_back( hardShadowRenderTarget );
	unorderedAccessTargets.typeUchar.push_back( mediumShadowRenderTarget );
	unorderedAccessTargets.typeUchar.push_back( softShadowRenderTarget );

	m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

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
            camera.getPosition(),
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
	m_rendererCore.disableRenderTargets();

	m_raytracingShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

	m_rendererCore.disableComputePipeline();
}

void RaytraceShadowRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
	m_raytracingShadowsComputeShader->loadAndInitialize( "Engine1/Shaders/RaytracingShadowsShader/RaytracingShadows_cs.cso", device );
}

void RaytraceShadowRenderer::createDefaultTextures( ID3D11Device3& device )
{
	std::vector< unsigned char > dataAlpha = { 255 };

	m_defaultAlphaTexture = std::make_shared< ImmutableTexture2D< unsigned char > >
		( device, dataAlpha, 1, 1, true, true, false, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
}

