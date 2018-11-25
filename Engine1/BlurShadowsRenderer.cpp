#include "BlurShadowsRenderer.h"

#include "Direct3DRendererCore.h"

#include "BlurShadowsComputeShader.h"
#include "Camera.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

BlurShadowsRenderer::BlurShadowsRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_blurShadowsComputeShader( std::make_shared< BlurShadowsComputeShader >() ),
    m_blurShadowsHorizontalComputeShader( std::make_shared< BlurShadowsComputeShader >() ),
    m_blurShadowsVerticalComputeShader( std::make_shared< BlurShadowsComputeShader >() )
{}

BlurShadowsRenderer::~BlurShadowsRenderer()
{}

void BlurShadowsRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device,
                                  ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void BlurShadowsRenderer::blurShadows( 
    const Camera& camera,
    const float positionThreshold,
    const float normalThreshold,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
    const std::shared_ptr< Texture2D< float > > distanceToOccluder,
    const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
    const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowRenderTarget,
    const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

    m_blurShadowsComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        positionThreshold,
        normalThreshold,
        positionTexture, 
        normalTexture, 
        shadowTexture, 
        distanceToOccluder, 
        finalDistanceToOccluder, 
        light 
    );

    m_rendererCore.enableComputeShader( m_blurShadowsComputeShader );

	RenderTargets unorderedAccessTargets;
    
    unorderedAccessTargets.typeUchar.push_back( shadowRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_blurShadowsComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void BlurShadowsRenderer::blurShadowsHorzVert( 
    const Camera& camera,
    const float positionThreshold,
    const float normalThreshold,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< unsigned char > > shadowTexture,
    const std::shared_ptr< Texture2D< float > > distanceToOccluder,
    const std::shared_ptr< Texture2D< float > > finalDistanceToOccluder,
    const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowRenderTarget,
    const std::shared_ptr< RenderTargetTexture2D< unsigned char > > shadowTemporaryRenderTarget,
    const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

	RenderTargets unorderedAccessTargets;

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    { // Horizontal blurring pass.
        unorderedAccessTargets.typeUchar.push_back( shadowTemporaryRenderTarget );

		m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

        m_blurShadowsHorizontalComputeShader->setParameters( 
            *m_deviceContext.Get(), 
            camera.getPosition(), 
            positionThreshold,
            normalThreshold,
            positionTexture,
            normalTexture, 
            shadowTexture, 
            distanceToOccluder, 
            finalDistanceToOccluder, 
            light 
        );

        m_rendererCore.enableComputeShader( m_blurShadowsHorizontalComputeShader );

        m_rendererCore.compute( groupCount );

        m_blurShadowsHorizontalComputeShader->unsetParameters( *m_deviceContext.Get() );
    }

    { // Vertical blurring pass.
		unorderedAccessTargets.typeUchar.clear();
        unorderedAccessTargets.typeUchar.push_back( shadowRenderTarget );

        m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

        m_blurShadowsVerticalComputeShader->setParameters( 
            *m_deviceContext.Get(), 
            camera.getPosition(), 
            positionThreshold,
            normalThreshold,
            positionTexture,
            normalTexture, 
            shadowTemporaryRenderTarget, 
            distanceToOccluder, 
            finalDistanceToOccluder, 
            light 
        );

        m_rendererCore.enableComputeShader( m_blurShadowsVerticalComputeShader );

        m_rendererCore.compute( groupCount );

        m_blurShadowsVerticalComputeShader->unsetParameters( *m_deviceContext.Get() );
    }

    m_rendererCore.disableComputePipeline();
}

void BlurShadowsRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_blurShadowsComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowsShader/BlurShadows_cs.cso", device );
    m_blurShadowsHorizontalComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowsShader/BlurShadowsHorizontal_cs.cso", device );
    m_blurShadowsVerticalComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowsShader/BlurShadowsVertical_cs.cso", device );
}

