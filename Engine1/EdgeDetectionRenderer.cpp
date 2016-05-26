#include "EdgeDetectionRenderer.h"

#include "Direct3DRendererCore.h"

#include "EdgeDetectionComputeShader.h"
#include "EdgeDistanceComputeShader.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

EdgeDetectionRenderer::EdgeDetectionRenderer( Direct3DRendererCore& rendererCore ) :
rendererCore( rendererCore ),
initialized( false ),
imageWidth( 0 ),
imageHeight( 0 ),
edgeDetectionComputeShader( std::make_shared< EdgeDetectionComputeShader >() ),
edgeDistanceComputeShader( std::make_shared< EdgeDistanceComputeShader >() )
{}

EdgeDetectionRenderer::~EdgeDetectionRenderer()
{}

void EdgeDetectionRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device, 
                                          ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->device        = device;
	this->deviceContext = deviceContext;

	this->imageWidth  = imageWidth;
	this->imageHeight = imageHeight;

    createRenderTargets( imageWidth, imageHeight, *device.Get() );

    loadAndCompileShaders( *device.Get() );

	initialized = true;
}

void EdgeDetectionRenderer::performEdgeDetection( const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                                  const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture )
{
    // For test - may not be necessary.
    valueRenderTargetSrc->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 255, 0, 0, 0 ) );
    valueRenderTargetDest->clearUnorderedAccessViewUint( *deviceContext.Get(), uint4( 255, 0, 0, 0 ) );

    rendererCore.disableRenderingPipeline();

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;
    
    { // Mark edges with value of 0.
        unorderedAccessTargetsU1.push_back( valueRenderTargetDest );
        rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                   unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

        edgeDetectionComputeShader->setParameters( *deviceContext.Get(), *positionTexture, *normalTexture );

        rendererCore.enableComputeShader( edgeDetectionComputeShader );

        uint3 groupCount( imageWidth / 32, imageHeight / 32, 1 );

        rendererCore.compute( groupCount );

        edgeDetectionComputeShader->unsetParameters( *deviceContext.Get() );
    }

    { // Calculate distance to nearest edge for each pixel - in 255 passes (because max dist is 255).
         rendererCore.enableComputeShader( edgeDistanceComputeShader );

         uint3 groupCount( imageWidth / 8, imageHeight / 8, 1 );

         for ( int i = 0; i < 255; ++i ) 
         {
             // Unset parameters to avoid binding the same resource twice.
             edgeDistanceComputeShader->unsetParameters( *deviceContext.Get() );

             // Swap source and destination texture.
             swapSrcDestRenderTargets();

             // Enable new destination render target.
             unorderedAccessTargetsU1.clear();
             unorderedAccessTargetsU1.push_back( valueRenderTargetDest );
             rendererCore.enableUnorderedAccessTargets( unorderedAccessTargetsF1, unorderedAccessTargetsF2, unorderedAccessTargetsF4,
                                                        unorderedAccessTargetsU1, unorderedAccessTargetsU4 );

             edgeDistanceComputeShader->setParameters( *deviceContext.Get(), *valueRenderTargetSrc );

             rendererCore.compute( groupCount );
         }
    }

    rendererCore.disableComputePipeline();
}

std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > EdgeDetectionRenderer::getValueRenderTarget()
{
    return valueRenderTargetDest;
}

void EdgeDetectionRenderer::swapSrcDestRenderTargets()
{
    auto valueRenderTargetTemp = valueRenderTargetSrc;
    valueRenderTargetSrc       = valueRenderTargetDest;
    valueRenderTargetDest      = valueRenderTargetTemp;
}

void EdgeDetectionRenderer::createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device )
{
    valueRenderTargetDest = valueRenderTarget0 = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT );

    valueRenderTargetSrc = valueRenderTarget1 = std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >
        ( device, imageWidth, imageHeight, false, true, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT );
}

void EdgeDetectionRenderer::loadAndCompileShaders( ID3D11Device& device )
{
    edgeDetectionComputeShader->compileFromFile( "../Engine1/Shaders/EdgeDetectionShader/cs.hlsl", device );
    edgeDistanceComputeShader->compileFromFile( "../Engine1/Shaders/EdgeDistanceShader/cs.hlsl", device );
}
