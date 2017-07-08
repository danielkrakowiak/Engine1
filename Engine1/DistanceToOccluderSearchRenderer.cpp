#include "DistanceToOccluderSearchRenderer.h"

#include "Direct3DRendererCore.h"

#include "DistanceToOccluderSearchComputeShader.h"
#include "Camera.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

DistanceToOccluderSearchRenderer::DistanceToOccluderSearchRenderer( Direct3DRendererCore& rendererCore ) :
    m_rendererCore( rendererCore ),
    m_initialized( false ),
    m_imageWidth( 0 ),
    m_imageHeight( 0 ),
    m_distanceToOccluderSearchComputeShader( std::make_shared< DistanceToOccluderSearchComputeShader >() )
{}

DistanceToOccluderSearchRenderer::~DistanceToOccluderSearchRenderer()
{}

void DistanceToOccluderSearchRenderer::initialize( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device,
                 ComPtr< ID3D11DeviceContext > deviceContext )
{
    this->m_device = device;
    this->m_deviceContext = deviceContext;

    this->m_imageWidth = imageWidth;
    this->m_imageHeight = imageHeight;

    loadAndCompileShaders( device );

    m_initialized = true;
}

void DistanceToOccluderSearchRenderer::performDistanceToOccluderSearch( const Camera& camera,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > distanceToOccluder,
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > finalDistanceToOccluderRenderTarget,
                                      const Light& light )
{
    m_rendererCore.disableRenderingPipeline();

    m_distanceToOccluderSearchComputeShader->setParameters( 
        *m_deviceContext.Get(), 
        camera.getPosition(), 
        positionTexture,
        normalTexture, 
        distanceToOccluder, 
        light 
    );

    m_rendererCore.enableComputeShader( m_distanceToOccluderSearchComputeShader );

    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >         unorderedAccessTargetsF1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > >        unorderedAccessTargetsF2;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > >        unorderedAccessTargetsF4;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1;
    std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > >        unorderedAccessTargetsU4;

    unorderedAccessTargetsF1.push_back( finalDistanceToOccluderRenderTarget );

    m_rendererCore.enableUnorderedAccessTargets( 
        unorderedAccessTargetsF1, 
        unorderedAccessTargetsF2, 
        unorderedAccessTargetsF4, 
        unorderedAccessTargetsU1, 
        unorderedAccessTargetsU4 
    );

    const int imageWidth = positionTexture->getWidth();
    const int imageHeight = positionTexture->getHeight();

    uint3 groupCount( imageWidth / 16, imageHeight / 16, 1 );

    m_rendererCore.compute( groupCount );

    m_distanceToOccluderSearchComputeShader->unsetParameters( *m_deviceContext.Get() );

    m_rendererCore.disableComputePipeline();
}

void DistanceToOccluderSearchRenderer::loadAndCompileShaders( ComPtr< ID3D11Device >& device )
{
    m_distanceToOccluderSearchComputeShader->loadAndInitialize( "Engine1/Shaders/DistanceToOccluderSearchShader/DistanceToOccluderSearch_cs.cso", device );
}