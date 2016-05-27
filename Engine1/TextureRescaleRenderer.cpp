#include "TextureRescaleRenderer.h"

#include "Direct3DRendererCore.h"

#include "TextureRescaleComputeShader.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

TextureRescaleRenderer::TextureRescaleRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
textureRescaleComputeShader( std::make_shared< TextureRescaleComputeShader >() )
{}

TextureRescaleRenderer::~TextureRescaleRenderer()
{}

void TextureRescaleRenderer::initialize( ComPtr< ID3D11Device > device, 
                                         ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->device        = device;
	this->deviceContext = deviceContext;

    loadAndCompileShaders( *device.Get() );

	initialized = true;
}

void TextureRescaleRenderer::rescaleTexture( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                                             const unsigned char srcMipmapLevel,
                                             const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 > > destTexture )
{
    rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;
    
    unorderedAccessTargetsF4.push_back( destTexture );
    rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

    textureRescaleComputeShader->setParameters( *deviceContext.Get(), *srcTexture, destTexture->getWidth(), destTexture->getHeight(), srcMipmapLevel );

    rendererCore.enableComputeShader( textureRescaleComputeShader );

    uint3 groupCount( destTexture->getWidth() / 8, destTexture->getHeight() / 8, 1 );

    rendererCore.compute( groupCount );

    textureRescaleComputeShader->unsetParameters( *deviceContext.Get() );

    rendererCore.disableComputePipeline();
}

void TextureRescaleRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    textureRescaleComputeShader->compileFromFile( "../Engine1/Shaders/TextureRescaleShader/cs.hlsl", device );
}
