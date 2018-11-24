#include "BlurShadowPatternRenderer.h"

#include "Direct3DRendererCore.h"

#include "BlurShadowPatternComputeShader.h"
#include "Camera.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

BlurShadowPatternRenderer::BlurShadowPatternRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_blurShadowPatternComputeShader( std::make_shared< BlurShadowPatternComputeShader >() ),
    m_blurShadowPatternHorizontalComputeShader( std::make_shared< BlurShadowPatternComputeShader >() ),
    m_blurShadowPatternVerticalComputeShader( std::make_shared< BlurShadowPatternComputeShader >() )
{}

BlurShadowPatternRenderer::~BlurShadowPatternRenderer()
{}

void BlurShadowPatternRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device,
                                  ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void BlurShadowPatternRenderer::blurShadowPattern( 
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

    m_blurShadowPatternComputeShader->setParameters( 
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

    m_rendererCore.enableComputeShader( m_blurShadowPatternComputeShader );

    std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >        unorderedAccessTargetsU4;
    
    unorderedAccessTargetsU1.push_back( shadowRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, 
        unorderedAccessTargetsF2, 
        unorderedAccessTargetsF3, 
        unorderedAccessTargetsF4, 
        unorderedAccessTargetsU1, 
        unorderedAccessTargetsU4 
    );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_blurShadowPatternComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void BlurShadowPatternRenderer::blurShadowPatternHorzVert( 
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

    std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >        unorderedAccessTargetsU4;

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    { // Horizontal blurring pass.
        unorderedAccessTargetsU1.push_back( shadowTemporaryRenderTarget );
        m_rendererCore.enableUnorderedAccessTargets( 
            unorderedAccessTargetsF1, 
            unorderedAccessTargetsF2, 
            unorderedAccessTargetsF3,
            unorderedAccessTargetsF4, 
            unorderedAccessTargetsU1, 
            unorderedAccessTargetsU4 
        );

        m_blurShadowPatternHorizontalComputeShader->setParameters( 
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

        m_rendererCore.enableComputeShader( m_blurShadowPatternHorizontalComputeShader );

        m_rendererCore.compute( groupCount );

        m_blurShadowPatternHorizontalComputeShader->unsetParameters( *m_deviceContext.Get() );
    }

    { // Vertical blurring pass.
        unorderedAccessTargetsU1.clear();
        unorderedAccessTargetsU1.push_back( shadowRenderTarget );
        m_rendererCore.enableUnorderedAccessTargets( 
            unorderedAccessTargetsF1, 
            unorderedAccessTargetsF2,
            unorderedAccessTargetsF3,
            unorderedAccessTargetsF4, 
            unorderedAccessTargetsU1, 
            unorderedAccessTargetsU4 
        );

        m_blurShadowPatternVerticalComputeShader->setParameters( 
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

        m_rendererCore.enableComputeShader( m_blurShadowPatternVerticalComputeShader );

        m_rendererCore.compute( groupCount );

        m_blurShadowPatternVerticalComputeShader->unsetParameters( *m_deviceContext.Get() );
    }

    m_rendererCore.disableComputePipeline();
}

void BlurShadowPatternRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_blurShadowPatternComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowPatternShader/BlurShadowPattern_cs.cso", device );
    m_blurShadowPatternHorizontalComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowPatternShader/BlurShadowPatternHorizontal_cs.cso", device );
    m_blurShadowPatternVerticalComputeShader->loadAndInitialize( "Engine1/Shaders/BlurShadowPatternShader/BlurShadowPatternVertical_cs.cso", device );
}


