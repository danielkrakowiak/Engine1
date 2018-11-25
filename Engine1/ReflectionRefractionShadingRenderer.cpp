#include "ReflectionRefractionShadingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ReflectionShadingComputeShader.h"
#include "ReflectionShadingComputeShader2.h"
#include "RefractionShadingComputeShader.h"
#include "RefractionShadingComputeShader2.h"
#include "Camera.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

const int ReflectionRefractionShadingRenderer::maxRenderTargetCount = 10;

ReflectionRefractionShadingRenderer::ReflectionRefractionShadingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_reflectionShadingComputeShader( std::make_shared< ReflectionShadingComputeShader >() ),
    m_reflectionShadingComputeShader2( std::make_shared< ReflectionShadingComputeShader2 >() ),
    m_refractionShadingComputeShader( std::make_shared< RefractionShadingComputeShader >() ),
    m_refractionShadingComputeShader2( std::make_shared< RefractionShadingComputeShader2 >() )
{}

ReflectionRefractionShadingRenderer::~ReflectionRefractionShadingRenderer()
{}

void ReflectionRefractionShadingRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device, 
                                          ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

	this->m_imageWidth  = imageWidth;
	this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

	m_initialized = true;
}

void ReflectionRefractionShadingRenderer::performFirstReflectionShading( 
    const Camera& camera,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
    const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
    const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
    const std::shared_ptr< RenderTargetTexture2D< uchar4 > > contributionRoughnessRenderTarget )
{
    m_rendererCore.disableRenderingPipeline();

    m_reflectionShadingComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        positionTexture, 
        normalTexture, 
        albedoTexture, 
        metalnessTexture, 
        roughnessTexture 
    );

    m_rendererCore.enableComputeShader( m_reflectionShadingComputeShader );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeUchar4.push_back( contributionRoughnessRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_reflectionShadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performReflectionShading( 
    const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
    const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
    const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
    const std::shared_ptr< Texture2D< uchar4 > > prevContributionRoughnessRenderTarget,
    const std::shared_ptr< RenderTargetTexture2D< uchar4 > > contributionRoughnessRenderTarget )
{
    m_rendererCore.disableRenderingPipeline();

    m_reflectionShadingComputeShader2->setParameters( 
        *m_deviceContext.Get(), 
        rayOriginTexture, 
        positionTexture, 
        normalTexture, 
        albedoTexture, 
        metalnessTexture, 
        roughnessTexture, 
        prevContributionRoughnessRenderTarget 
    );

    m_rendererCore.enableComputeShader( m_reflectionShadingComputeShader2 );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeUchar4.push_back( contributionRoughnessRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_reflectionShadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performFirstRefractionShading( 
    const Camera& camera,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
    const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
    const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
    const std::shared_ptr< RenderTargetTexture2D< uchar4 > > contributionRoughnessRenderTarget )
{
    m_rendererCore.disableRenderingPipeline();

    m_refractionShadingComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        positionTexture, 
        normalTexture, 
        albedoTexture, 
        metalnessTexture, 
        roughnessTexture, 
        nullptr 
    );

    m_rendererCore.enableComputeShader( m_refractionShadingComputeShader );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeUchar4.push_back( contributionRoughnessRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_refractionShadingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::performRefractionShading( 
    const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
    const std::shared_ptr< Texture2D< float4 > > positionTexture,
    const std::shared_ptr< Texture2D< float4 > > normalTexture,
    const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
    const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
    const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
    const std::shared_ptr< Texture2D< uchar4 > > prevContributionRoughnessRenderTarget,
    const std::shared_ptr< RenderTargetTexture2D< uchar4 > > contributionRoughnessRenderTarget )
{
    m_rendererCore.disableRenderingPipeline();

    m_refractionShadingComputeShader2->setParameters( 
        *m_deviceContext.Get(), 
        rayOriginTexture, 
        positionTexture, 
        normalTexture, 
        albedoTexture, 
        metalnessTexture, 
        roughnessTexture, 
        prevContributionRoughnessRenderTarget 
    );

    m_rendererCore.enableComputeShader( m_refractionShadingComputeShader2 );

	RenderTargets unorderedAccessTargets;
    unorderedAccessTargets.typeUchar4.push_back( contributionRoughnessRenderTarget );

    m_rendererCore.enableRenderTargets( RenderTargets(), unorderedAccessTargets );

    const int imageWidth  = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_refractionShadingComputeShader2->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ReflectionRefractionShadingRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_reflectionShadingComputeShader->loadAndInitialize( "Engine1/Shaders/ReflectionShadingShader/ReflectionShading_cs.cso", device );
    m_reflectionShadingComputeShader2->loadAndInitialize( "Engine1/Shaders/ReflectionShadingShader/ReflectionShading_cs2.cso", device );
    m_refractionShadingComputeShader->loadAndInitialize( "Engine1/Shaders/RefractionShadingShader/RefractionShading_cs.cso", device );
    m_refractionShadingComputeShader2->loadAndInitialize( "Engine1/Shaders/RefractionShadingShader/RefractionShading_cs2.cso", device );
}
