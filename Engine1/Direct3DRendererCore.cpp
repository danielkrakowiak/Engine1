#include "Direct3DRendererCore.h"

#include "RectangleMesh.h"
#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "Font.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "ComputeShader.h"
#include "SkeletonMeshVertexShader.h"

#include "MathUtil.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DRendererCore::Direct3DRendererCore() :
m_deviceContext( nullptr ),
viewportDimensions( float2::ZERO ),
viewportTopLeft( float2::ZERO ),
viewportDepthMin( 0.0f ),
viewportDepthMax( 0.0f ),
m_currentRenderTargetViews(),
m_currentDepthRenderTargetView( nullptr ),
m_graphicsShaderEnabled( false ),
m_computeShaderEnabled( false ),
m_currentRasterizerState( nullptr ),
m_currentDepthStencilState( nullptr ),
m_currentBlendState( nullptr )
{
    createNullShaderInputs();
}


Direct3DRendererCore::~Direct3DRendererCore()
{}

void Direct3DRendererCore::initialize( ID3D11DeviceContext& deviceContext )
{
	this->m_deviceContext = &deviceContext;
}

void Direct3DRendererCore::disableRenderingPipeline()
{
    disableRenderingShaders();
    disableRenderTargetViews();
    enableDefaultBlendState();
    enableDefaultRasterizerState();
    enableDefaultDepthStencilState();
    disableShaderInputs();
}

void Direct3DRendererCore::disableComputePipeline()
{
    disableComputeShaders();
    disableUnorderedAccessViews();
}

void Direct3DRendererCore::enableRenderingShaders( std::shared_ptr<const VertexShader> vertexShader, std::shared_ptr<const FragmentShader> fragmentShader )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableRenderingShaders - renderer not initialized." );
    if ( !vertexShader ) throw std::exception( "Direct3DRendererCore::enableRenderingShaders - passed vertex shader is nullptr." );

    disableComputeShaders();

	// Check if currently set vertex shader is the same as the one to be enabled - do nothing then. 
	if ( m_currentVertexShader.expired() || !m_currentVertexShader.lock()->isSame( *vertexShader ) ) 
	{
		m_deviceContext->IASetInputLayout( &vertexShader->getInputLauout() );
		m_deviceContext->VSSetShader( &vertexShader->getShader(), nullptr, 0 );

        m_currentVertexShader = vertexShader;
	}

	// Check if currently set fragment shader is the same as the one to be enabled - do nothing then.
	if ( !m_currentFragmentShader.expired() && !fragmentShader )
	{
		m_deviceContext->PSSetShader( nullptr, nullptr, 0 );

		m_currentFragmentShader.reset();
	}
	else if ( m_currentFragmentShader.expired() || !m_currentFragmentShader.lock()->isSame( *fragmentShader ) ) 
	{
		m_deviceContext->PSSetShader( fragmentShader ? &fragmentShader->getShader() : nullptr, nullptr, 0 );

        m_currentFragmentShader = fragmentShader;
	}

    m_graphicsShaderEnabled = true;
}

void Direct3DRendererCore::enableComputeShader( std::shared_ptr<const ComputeShader> computeShader )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableComputeShader - renderer not initialized." );
    if ( !computeShader ) throw std::exception( "Direct3DRendererCore::enableComputeShader - passed shader is nullptr." );

    disableRenderingShaders();

    // Check if currently set compute shader is the same as the one to be enabled - do nothing then. 
    if ( m_currentComputeShader.expired() || !m_currentComputeShader.lock()->isSame( *computeShader ) ) {
        m_deviceContext->CSSetShader( &computeShader->getShader(), nullptr, 0 );

        m_currentComputeShader = computeShader;
    }

    m_computeShaderEnabled = true;
}

void Direct3DRendererCore::disableRenderingShaders()
{
    if ( m_graphicsShaderEnabled ) {
        m_deviceContext->IASetInputLayout( nullptr );
        m_deviceContext->VSSetShader( nullptr, nullptr, 0 );
        m_deviceContext->PSSetShader( nullptr, nullptr, 0 );
        
        m_currentVertexShader.reset();
        m_currentFragmentShader.reset();

        m_graphicsShaderEnabled = false;
    }
}

void Direct3DRendererCore::disableComputeShaders()
{
    if ( m_computeShaderEnabled ) {
        m_deviceContext->CSSetShader( nullptr, nullptr, 0 );
        m_currentComputeShader.reset();

        m_computeShaderEnabled = false;
    }
}

void Direct3DRendererCore::setViewport( float2 dimensions, float2 topLeft, float depthMin, float depthMax )
{
    if ( !MathUtil::areEqual( dimensions, viewportDimensions ) || !MathUtil::areEqual( topLeft, viewportTopLeft ) || 
         !MathUtil::areEqual( depthMin, viewportDepthMin ) || !MathUtil::areEqual( depthMax, viewportDepthMax ) )
    {
        D3D11_VIEWPORT viewport;
        viewport.Width    = (float)dimensions.x;
        viewport.Height   = (float)dimensions.y;
        viewport.MinDepth = depthMin;
        viewport.MaxDepth = depthMax;
        viewport.TopLeftX = topLeft.x;
        viewport.TopLeftY = topLeft.y;

        m_deviceContext->RSSetViewports( 1, &viewport );

        viewportDimensions = dimensions;
        viewportTopLeft    = topLeft;
        viewportDepthMin   = depthMin;
        viewportDepthMax   = depthMax;
    }
}

void Direct3DRendererCore::enableRenderTargets( const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget, const int mipmapLevel )
{
	m_currentRenderTargetViews.clear();

	if ( depthRenderTarget )
		m_currentDepthRenderTargetView = depthRenderTarget->getDepthStencilView( mipmapLevel );

	// Enable render targets.
	m_deviceContext->OMSetRenderTargets( 0, 0, m_currentDepthRenderTargetView );
}

void Direct3DRendererCore::enableRenderTargets( const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, float > > depthRenderTarget, const int mipmapLevel )
{
    m_currentRenderTargetViews.clear();

    if ( depthRenderTarget )
        m_currentDepthRenderTargetView = depthRenderTarget->getDepthStencilView( mipmapLevel );

    // Enable render targets.
    m_deviceContext->OMSetRenderTargets( 0, 0, m_currentDepthRenderTargetView );

}

void Direct3DRendererCore::enableRenderTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > > >&  renderTargetsF1,
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >& renderTargetsF2,
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >& renderTargetsF4,
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > >& renderTargetsU1, 
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >& renderTargetsU4, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget,
                                                const int mipmapLevel )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - renderer not initialized." );
	if ( renderTargetsF2.size() + renderTargetsU4.size() > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - too many render targets passed. Number exceeds the supported maximum." );

	bool sameAsCurrent = true;

	{ // Check if render targets to be enabled are the same as the current ones.
		//if ( renderTargets.size() == currentRenderTargetViews.size() ) {
            for ( unsigned int i = 0; i < renderTargetsF1.size(); ++i ) {
                // Check each pair of render targets at corresponding indexes.
                if ( m_currentRenderTargetViews.size() <= i || renderTargetsF1.at( i )->getRenderTargetView( mipmapLevel ) != m_currentRenderTargetViews.at( i ) ) {
                    sameAsCurrent = false;
                    break;
                }
            }

            const unsigned int zero = (unsigned int)renderTargetsF1.size();
			for ( unsigned int i = 0; i < renderTargetsF2.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( m_currentRenderTargetViews.size() <= (zero + i) || renderTargetsF2.at( i )->getRenderTargetView( mipmapLevel ) != m_currentRenderTargetViews.at( zero + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int first = zero + (unsigned int)renderTargetsF2.size();
            for ( unsigned int i = 0; i < renderTargetsF4.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( m_currentRenderTargetViews.size() <= (first + i) || renderTargetsF4.at( i )->getRenderTargetView( mipmapLevel ) != m_currentRenderTargetViews.at( first + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int second = first + (unsigned int)renderTargetsF4.size();
            for ( unsigned int i = 0; i < renderTargetsU1.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( m_currentRenderTargetViews.size() <= (second + i) || renderTargetsU1.at( i )->getRenderTargetView( mipmapLevel ) != m_currentRenderTargetViews.at( second + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int third = second + (unsigned int)renderTargetsU1.size();
            for ( unsigned int i = 0; i < renderTargetsU4.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( m_currentRenderTargetViews.size() <= (second + i) || renderTargetsU4.at( i )->getRenderTargetView( mipmapLevel ) != m_currentRenderTargetViews.at( third + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}
		//} else {
		//	// Render targets are not the same as the current ones, because they have different count.
		//	sameAsCurrent = false;
		//}

		// Check if depth render target to be enabled is the same as the current one.
		if ( ( depthRenderTarget == nullptr ) != ( m_currentDepthRenderTargetView == nullptr ) 
             || ( depthRenderTarget && depthRenderTarget->getDepthStencilView( mipmapLevel ) == m_currentDepthRenderTargetView ) )
			sameAsCurrent = false;
	}

	// If render targets to be enabled are the same as the current ones - do nothing.
	if ( sameAsCurrent )
		return;

	// Collect and save render target views from passed render targets.
    m_currentRenderTargetViews.clear();
    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float > >& renderTarget : renderTargetsF1 )
		m_currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

	for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > >& renderTarget : renderTargetsF2 )
		m_currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > >& renderTarget : renderTargetsF4 )
		m_currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > >& renderTarget : renderTargetsU1 )
		m_currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > >& renderTarget : renderTargetsU4 )
		m_currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

	// Get and save depth render target view if passed.
	if ( depthRenderTarget )
		m_currentDepthRenderTargetView = depthRenderTarget->getDepthStencilView( mipmapLevel );

	// Enable render targets.
	m_deviceContext->OMSetRenderTargets( (unsigned int)m_currentRenderTargetViews.size(), m_currentRenderTargetViews.data(), m_currentDepthRenderTargetView );
}

void Direct3DRendererCore::enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > > unorderedAccessTargetsF1,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4,
                                                         const int mipmapLevel )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableUnorderedAccessTargets - renderer not initialized." );
    //#TODO: WARNING: For pixel shaders, UAVStartSlot param should be equal to the number of render-target views being bound.

    bool sameAsCurrent = true;

	{ // Check if UAV targets to be enabled are the same as the current ones.
		//if ( unorderedAccessTargets.size() == currentUnorderedAccessTargetViews.size() ) {
			for ( unsigned int i = 0; i < unorderedAccessTargetsF1.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( m_currentUnorderedAccessTargetViews.size() <= i 
                     || ( ( unorderedAccessTargetsF1.at( i ) == nullptr ) != ( m_currentUnorderedAccessTargetViews.at( i ) == nullptr) ) 
                     || unorderedAccessTargetsF1.at( i )->getUnorderedAccessView( mipmapLevel ) != m_currentUnorderedAccessTargetViews.at( i ) ) 
                {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int first = (unsigned int)unorderedAccessTargetsF1.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsF2.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( m_currentUnorderedAccessTargetViews.size() <= (first + i) 
                     || ((unorderedAccessTargetsF2.at( i ) == nullptr) != (m_currentUnorderedAccessTargetViews.at( first + i ) == nullptr))
                     || unorderedAccessTargetsF2.at( i )->getUnorderedAccessView( mipmapLevel ) != m_currentUnorderedAccessTargetViews.at( first + i ) ) 
                {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int second = first + (unsigned int)unorderedAccessTargetsF2.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsF4.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( m_currentUnorderedAccessTargetViews.size() <= (second + i) 
                     || ((unorderedAccessTargetsF2.at( i ) == nullptr) != (m_currentUnorderedAccessTargetViews.at( second + i ) == nullptr))
                     || unorderedAccessTargetsF4.at( i )->getUnorderedAccessView( mipmapLevel ) != m_currentUnorderedAccessTargetViews.at( second + i ) ) 
                {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int third = second + (unsigned int)unorderedAccessTargetsF4.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsU1.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( m_currentUnorderedAccessTargetViews.size() <= (third + i) 
                     || ((unorderedAccessTargetsF2.at( i ) == nullptr) != (m_currentUnorderedAccessTargetViews.at( third + i ) == nullptr))
                     || unorderedAccessTargetsU1.at( i )->getUnorderedAccessView( mipmapLevel ) != m_currentUnorderedAccessTargetViews.at( third + i ) ) 
                {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int fourth = third + (unsigned int)unorderedAccessTargetsU1.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsU4.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( m_currentUnorderedAccessTargetViews.size() <= (third + i) 
                     || ((unorderedAccessTargetsF2.at( i ) == nullptr) != (m_currentUnorderedAccessTargetViews.at( fourth + i ) == nullptr))
                     || unorderedAccessTargetsU4.at( i )->getUnorderedAccessView( mipmapLevel ) != m_currentUnorderedAccessTargetViews.at( fourth + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}
		//} else {
		//	// UAV targets are not the same as the current ones, because they have different count.
		//	sameAsCurrent = false;
		//}
	}

	// If UAV targets to be enabled are the same as the current ones - do nothing.
	if ( sameAsCurrent )
		return;

	// Collect and save UAV target views from passed UAV targets.
    m_currentUnorderedAccessTargetViews.clear();
	for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > >& unorderedAccessTarget : unorderedAccessTargetsF1 )
		m_currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget ? unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) : nullptr );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > >& unorderedAccessTarget : unorderedAccessTargetsF2 )
		m_currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget ? unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) : nullptr );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > >& unorderedAccessTarget : unorderedAccessTargetsF4 )
		m_currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget ? unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) : nullptr );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > >& unorderedAccessTarget : unorderedAccessTargetsU1 )
		m_currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget ? unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) : nullptr );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > >& unorderedAccessTarget : unorderedAccessTargetsU4 )
		m_currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget ? unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) : nullptr );

	// Enable UAV targets.
    m_deviceContext->CSSetUnorderedAccessViews( 0, (unsigned int)m_currentUnorderedAccessTargetViews.size(), m_currentUnorderedAccessTargetViews.data(), nullptr );
}

void Direct3DRendererCore::enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4,
                                                         const int mipmapLevel )
{
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >  emptyF1;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > emptyF2;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > emptyU1;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > emptyU4;

    enableUnorderedAccessTargets( emptyF1, emptyF2, unorderedAccessTargetsF4, emptyU1, emptyU4, mipmapLevel );
}

void Direct3DRendererCore::enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4,
                                                         const int mipmapLevel )
{
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > >  emptyF1;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > emptyF2;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > emptyF4;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > emptyU1;
    const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > emptyU4;

    enableUnorderedAccessTargets( emptyF1, emptyF2, emptyF4, emptyU1, unorderedAccessTargetsU4, mipmapLevel );
}

void Direct3DRendererCore::disableRenderTargetViews()
{
    if ( !m_currentRenderTargetViews.empty() || m_currentDepthRenderTargetView )
    {
        std::vector< ID3D11RenderTargetView* > emptyTargets;
        emptyTargets.resize( m_currentRenderTargetViews.size() );
        for (unsigned int i = 0; i < m_currentRenderTargetViews.size(); ++i)
            emptyTargets[ i ] = nullptr;

        ID3D11DepthStencilView* emptyDepthTarget = nullptr;

        m_deviceContext->OMSetRenderTargets( (unsigned int)m_currentRenderTargetViews.size(), emptyTargets.data(), emptyDepthTarget );

        m_currentRenderTargetViews.clear();
        m_currentDepthRenderTargetView = nullptr;
    }
}

void Direct3DRendererCore::disableUnorderedAccessViews()
{
    if ( !m_currentUnorderedAccessTargetViews.empty() )
    {
        std::vector< ID3D11UnorderedAccessView* > emptyTargets;
        emptyTargets.resize( m_currentUnorderedAccessTargetViews.size() );
        for (unsigned int i = 0; i < m_currentUnorderedAccessTargetViews.size(); ++i)
            emptyTargets[ i ] = nullptr;

        m_deviceContext->CSSetUnorderedAccessViews( 0, (unsigned int)m_currentUnorderedAccessTargetViews.size(), emptyTargets.data(), nullptr );
        //deviceContext->OMSetRenderTargetsAndUnorderedAccessViews( D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, 0, nullptr, nullptr );

        m_currentUnorderedAccessTargetViews.clear();
    }
}

void Direct3DRendererCore::enableRasterizerState( ID3D11RasterizerState& rasterizerState )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableRasterizerState - renderer not initialized." );

	// Change rasterizer state if new state is different than the current one.
	if ( &rasterizerState != m_currentRasterizerState ) {
		m_deviceContext->RSSetState( &rasterizerState );

		m_currentRasterizerState = &rasterizerState;
	}
}

void Direct3DRendererCore::enableDepthStencilState( ID3D11DepthStencilState& depthStencilState )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDepthStencilState - renderer not initialized." );

	// Change depth stencil state if new state is different than the current one.
	if ( &depthStencilState != m_currentDepthStencilState ) {
		m_deviceContext->OMSetDepthStencilState( &depthStencilState, 0 );

		m_currentDepthStencilState = &depthStencilState;
	}
}

void Direct3DRendererCore::enableBlendState( ID3D11BlendState& blendState )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableBlendState - renderer not initialized." );

	// Change blend state if new state is different than the current one.
	if ( &blendState != m_currentBlendState ) {

		float blendFactor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState( &blendState, blendFactor, sampleMask );

		m_currentBlendState = &blendState;
	}
}

void Direct3DRendererCore::enableDefaultRasterizerState()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultRasterizerState - renderer not initialized." );

    if ( m_currentRasterizerState ) {
        m_deviceContext->RSSetState( nullptr );

        m_currentRasterizerState = nullptr;
    }
}

void Direct3DRendererCore::enableDefaultDepthStencilState()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultDepthStencilState - renderer not initialized." );

    if ( m_currentDepthStencilState ) {
        m_deviceContext->OMSetDepthStencilState( nullptr, 0 );

        m_currentDepthStencilState = nullptr;
    }
}

void Direct3DRendererCore::enableDefaultBlendState()
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultBlendState - renderer not initialized." );

	if ( m_currentBlendState ) {

		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState( nullptr, nullptr, sampleMask );

		m_currentBlendState = nullptr;
	}
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const RectangleMesh& mesh )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );
	if ( !mesh.isInGpuMemory() ) throw std::exception( "Direct3DRenderer::drawRectangleMesh - mesh hasn't been loaded to GPU yet" );

	{ // set mesh buffers
		if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() && mesh.getTexcoordBuffer() > 0 ) {
			const unsigned int bufferCount = 3;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ), sizeof( float2 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer(), mesh.getTexcoordBuffer() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() ) {

			const unsigned int bufferCount = 2;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() ) {

			const unsigned int bufferCount = 1;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		}
	}

	// draw mesh
	m_deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const BlockMesh& mesh )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );
	if ( !mesh.isInGpuMemory() ) throw std::exception( "Direct3DRenderer::drawBlockMesh - mesh hasn't been loaded to GPU yet" );

	{ // set mesh buffers
		if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() && mesh.getTangentBuffer() && mesh.getTexcoordBuffers().size() > 0 && mesh.getTexcoordBuffers().front() ) {
			const unsigned int bufferCount = 4;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ), sizeof( float3 ), sizeof( float2 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0, 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer(), mesh.getTangentBuffer(), mesh.getTexcoordBuffers().front() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() ) {

			const unsigned int bufferCount = 2;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() ) {

			const unsigned int bufferCount = 1;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer() };

			m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		}
	}

	// draw mesh
	m_deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const SkeletonMesh& mesh )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );

	//TODO: move this tests to some method? To which class?
	if ( mesh.getVertexBones().empty() )   throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh doesn't have vertex-bone assignemnts." );
	if ( mesh.getVertexWeights().empty() ) throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh doesn't have vertex weights." );
	if ( !mesh.isInGpuMemory() )           throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh is not in GPU memory." );

	const bool hasNormals   = mesh.getNormalBuffer() != nullptr;
    const bool hasTangents  = mesh.getTangentBuffer() != nullptr;
	const bool hasTexcoords = !mesh.getTexcoordBuffers().empty() && mesh.getTexcoordBuffers().front() != nullptr;

	unsigned int bufferCount = 3;      // vertices + vertex-bones + vertex-weights
	if ( hasNormals ) ++bufferCount;   // normals
    if ( hasTangents ) ++bufferCount;  // tangents
	if ( hasTexcoords ) ++bufferCount; // texcoords

	const unsigned int vertexStride        = sizeof( float3 );
	const unsigned int vertexBonesStride   = static_cast<unsigned int>( mesh.getBonesPerVertexCount() ) * sizeof( unsigned char );
	const unsigned int vertexWeightsStride = static_cast<unsigned int>( mesh.getBonesPerVertexCount() ) * sizeof( float );
	const unsigned int normalStride        = sizeof( float3 );
    const unsigned int tangentStride       = sizeof( float3 );
	const unsigned int texcoordStride      = sizeof( float2 );

	const unsigned int vertexOffset        = 0;
	const unsigned int vertexBonesOffset   = 0;
	const unsigned int vertexWeightsOffset = 0;
	const unsigned int normalOffset        = 0;
    const unsigned int tangentOffset       = 0;
	const unsigned int texcoordOffset      = 0;

	std::vector<unsigned int> strides;
	strides.push_back( vertexStride );
	strides.push_back( vertexBonesStride );
	strides.push_back( vertexWeightsStride );
	if ( hasNormals )   strides.push_back( normalStride );
    if ( hasTangents )  strides.push_back( tangentStride );
	if ( hasTexcoords ) strides.push_back( texcoordStride );

	std::vector<unsigned int> offsets;
	offsets.push_back( vertexOffset );
	offsets.push_back( vertexBonesOffset );
	offsets.push_back( vertexWeightsOffset );
	if ( hasNormals )   offsets.push_back( normalOffset );
    if ( hasTangents )  offsets.push_back( tangentOffset );
	if ( hasTexcoords ) offsets.push_back( texcoordOffset );

	std::vector<ID3D11Buffer*> buffers;
	buffers.push_back( mesh.getVertexBuffer() );
	buffers.push_back( mesh.getVertexBonesBuffer() );
	buffers.push_back( mesh.getVertexWeightsBuffer() );
	if ( hasNormals )   buffers.push_back( mesh.getNormalBuffer() );
    if ( hasTangents )  buffers.push_back( mesh.getTangentBuffer() );
	if ( hasTexcoords ) buffers.push_back( mesh.getTexcoordBuffers().front() );

	m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers.data(), strides.data(), offsets.data() );
	m_deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Draw the mesh.
	m_deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const FontCharacter& character )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );

	const unsigned int bufferCount = 2;
	unsigned int  strides[ bufferCount ] = { sizeof( float3 ), sizeof( float2 ) };
	unsigned int  offsets[ bufferCount ] = { 0, 0 };
	ID3D11Buffer* buffers[ bufferCount ] = { character.getVertexBuffer(), FontCharacter::getDefaultTexcoordsBuffer() };

	m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
	m_deviceContext->IASetIndexBuffer( FontCharacter::getDefaultTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Draw the mesh.
	const unsigned int triangleCount = 2;
	m_deviceContext->DrawIndexed( triangleCount * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::compute( uint3 threadCount )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::compute - renderer not initialized." );

    m_deviceContext->Dispatch( threadCount.x, threadCount.y, threadCount.z );
}

void Direct3DRendererCore::disableShaderInputs()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::disableShaderInputs - renderer not initialized." );

    m_deviceContext->IASetVertexBuffers( 0, (unsigned int)m_nullVertexBuffers.size(), m_nullVertexBuffers.data(), m_nullVertexBuffersStrideOffset.data(), m_nullVertexBuffersStrideOffset.data() );
    m_deviceContext->IASetIndexBuffer( nullptr, DXGI_FORMAT_UNKNOWN, 0 );
    m_deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_UNDEFINED );

    m_deviceContext->PSSetShaderResources( 0, (unsigned int)m_nullResources.size(), m_nullResources.data() );
    m_deviceContext->PSSetSamplers( 0, (unsigned int)m_nullSamplers.size(), m_nullSamplers.data() );
}

void Direct3DRendererCore::createNullShaderInputs()
{
    m_nullVertexBuffers.resize( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT  );
    m_nullVertexBuffersStrideOffset.resize( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
    for ( int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ; ++i ) {
        m_nullVertexBuffers[ i ] = nullptr;
        m_nullVertexBuffersStrideOffset[ i ] = 0;
    }

    m_nullResources.resize( D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT );
    for ( int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i )
        m_nullResources[ i ] = nullptr;

    m_nullSamplers.resize( D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT );
    for ( int i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i )
        m_nullSamplers[ i ] = nullptr;
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                                        const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    m_deviceContext->CopyResource( destTexture->getTextureResource().Get(), srcTexture->getTextureResource().Get() );
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > srcTexture )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    m_deviceContext->CopyResource( destTexture->getTextureResource().Get(), srcTexture->getTextureResource().Get() );
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > srcTexture )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    m_deviceContext->CopyResource( destTexture->getTextureResource().Get(), srcTexture->getTextureResource().Get() );
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture, const int destMipmap,
                  const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture, const int srcMipmap )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    D3D11_BOX sourceRregion;
    sourceRregion.left   = 0;
    sourceRregion.right  = srcTexture->getWidth();
    sourceRregion.top    = 0;
    sourceRregion.bottom = srcTexture->getHeight();
    sourceRregion.front  = 0;
    sourceRregion.back   = 1;

    m_deviceContext->CopySubresourceRegion( destTexture->getTextureResource().Get(), (UINT)destMipmap, 0, 0, 0, srcTexture->getTextureResource().Get(), srcMipmap, &sourceRregion );
}