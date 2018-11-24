#include "ToneMappingRenderer.h"

#include "Direct3DRendererCore.h"

#include "ToneMappingComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ToneMappingRenderer::ToneMappingRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_toneMappingComputeShader( std::make_shared< ToneMappingComputeShader >() )
{}

ToneMappingRenderer::~ToneMappingRenderer()
{}

void ToneMappingRenderer::initialize( ComPtr< ID3D11Device3 > device,
                                              ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void ToneMappingRenderer::performToneMapping( std::shared_ptr< Texture2D< float4 > > srcTexture,
                                              std::shared_ptr< Texture2D< uchar4 > > dstTexture,
                                              const float exposure )
{
    m_rendererCore.disableRenderingPipeline();

    m_toneMappingComputeShader->setParameters( *m_deviceContext.Get(), *srcTexture, exposure );

    m_rendererCore.enableComputeShader( m_toneMappingComputeShader );

    std::vector< std::shared_ptr< Texture2D< float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2D< float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2D< float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< Texture2D< float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2D< unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2D< uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsU4.push_back( dstTexture );

    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, 
        unorderedAccessTargetsF2, 
        unorderedAccessTargetsF3,
        unorderedAccessTargetsF4, 
        unorderedAccessTargetsU1, 
        unorderedAccessTargetsU4 
    );

    const int imageWidth = dstTexture->getWidth();
    const int imageHeight = dstTexture->getHeight();

    uint3 groupCount(
        imageWidth / 16,
        imageHeight / 16,
        1
    );

    m_rendererCore.compute( groupCount );

    m_toneMappingComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void ToneMappingRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_toneMappingComputeShader->loadAndInitialize( "Engine1/Shaders/ToneMappingShader/ToneMapping_cs.cso", device );
}



