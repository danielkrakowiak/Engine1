#include "TextureRescaleRenderer.h"

#include "Direct3DRendererCore.h"

#include "TextureRescaleComputeShader.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureRescaleRenderer::TextureRescaleRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_textureRescaleComputeShader( std::make_shared< TextureRescaleComputeShader >() )
{}

TextureRescaleRenderer::~TextureRescaleRenderer()
{}

void TextureRescaleRenderer::initialize( ComPtr< ID3D11Device3 > device, 
                                         ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void TextureRescaleRenderer::rescaleTexture( const std::shared_ptr< Texture2D< float4 > > srcTexture,
                                             const unsigned char srcMipmapLevel,
                                             const std::shared_ptr< Texture2D< float4 > > destTexture,
                                             const unsigned char destMipmapLevel )
{
    m_rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2D< float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2D< float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2D< float3 > > >        unorderedAccessTargetsF3;
    std::vector< std::shared_ptr< Texture2D< float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2D< unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2D< uchar4 > > >        unorderedAccessTargetsU4;
    
    unorderedAccessTargetsF4.push_back( destTexture );
    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF3,
        unorderedAccessTargetsF4, unorderedAccessTargetsU1, unorderedAccessTargetsU4, 
        destMipmapLevel 
    );

    m_textureRescaleComputeShader->setParameters( *m_deviceContext.Get(), *srcTexture, destTexture->getWidth(), destTexture->getHeight(), srcMipmapLevel );

    m_rendererCore.enableComputeShader( m_textureRescaleComputeShader );

    uint3 groupCount( destTexture->getWidth() / 8, destTexture->getHeight() / 8, 1 );

    m_rendererCore.compute( groupCount );

    m_textureRescaleComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void TextureRescaleRenderer::loadAndCompileShaders( ComPtr< ID3D11Device3 >& device )
{
    m_textureRescaleComputeShader->loadAndInitialize( "Engine1/Shaders/TextureRescaleShader/TextureRescale_cs.cso", device );
}
