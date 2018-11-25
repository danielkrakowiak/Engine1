#include "HitDistanceSearchRenderer.h"

#include <algorithm>

#include "Direct3DRendererCore.h"
#include "HitDistanceSearchComputeShader.h"
#include "Camera.h"



using namespace Engine1;

using Microsoft::WRL::ComPtr;

HitDistanceSearchRenderer::HitDistanceSearchRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_hitDistanceSearchComputeShader( std::make_shared< HitDistanceSearchComputeShader >() )
{}

HitDistanceSearchRenderer::~HitDistanceSearchRenderer()
{}

void HitDistanceSearchRenderer::initialize( 
    ComPtr< ID3D11Device3 > device,
    ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    m_device        = device;
    m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void HitDistanceSearchRenderer::performHitDistanceSearch( 
    const Camera& camera,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< float > > hitDistance,
    std::shared_ptr< RenderTargetTexture2D< float > > blurredHitDistanceRenderTarget )
{
    const int2 renderTargetDimensions = blurredHitDistanceRenderTarget->getDimensions();

    m_rendererCore.disableRenderingPipeline();

    m_hitDistanceSearchComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        renderTargetDimensions,
        positionTexture, 
        normalTexture, 
        hitDistance 
    );

    m_rendererCore.enableComputeShader( m_hitDistanceSearchComputeShader );

	RenderTargets unorderedAccessTargets;

	unorderedAccessTargets.typeFloat.push_back( blurredHitDistanceRenderTarget );

	m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    uint3 groupCount( 
        (int)ceil( (float)renderTargetDimensions.x / 16.0f ), 
        (int)ceil( (float)renderTargetDimensions.y / 16.0f ), 
        1 
    );

    m_rendererCore.compute( groupCount );

    m_hitDistanceSearchComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void HitDistanceSearchRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_hitDistanceSearchComputeShader->loadAndInitialize( "Engine1/Shaders/HitDistanceSearchShader/HitDistanceSearch_cs.cso", device );
}