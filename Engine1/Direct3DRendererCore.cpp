#include "Direct3DRendererCore.h"

#include "RectangleMesh.h"
#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "Font.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "ComputeShader.h"
#include "SkeletonMeshVertexShader.h"
//#include "RenderTargetTexture2D.h"
//#include "RenderTargetDepthTexture2D.h"
//#include "ComputeTargetTexture2D.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DRendererCore::Direct3DRendererCore() :
deviceContext( nullptr ),
currentRenderTargetViews(),
vertexOrFragmentShaderEnabled( false ),
computeShaderEnabled( false ),
currentRasterizerState( nullptr ),
currentDepthStencilState( nullptr ),
currentBlendState( nullptr )
{
    createNullShaderInputs();
}


Direct3DRendererCore::~Direct3DRendererCore()
{}

void Direct3DRendererCore::initialize( ID3D11DeviceContext& deviceContext )
{
	this->deviceContext = &deviceContext;
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
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableRenderingShaders - renderer not initialized." );
    if ( !vertexShader || !fragmentShader ) throw std::exception( "Direct3DRendererCore::enableRenderingShaders - passed shaders are nullptrs." );

    disableComputeShaders();

	// Check if currently set vertex shader is the same as the one to be enabled - do nothing then. 
	if ( currentVertexShader.expired() || !currentVertexShader.lock()->isSame( *vertexShader ) ) 
	{
		deviceContext->IASetInputLayout( &vertexShader->getInputLauout() );
		deviceContext->VSSetShader( &vertexShader->getShader(), nullptr, 0 );

        currentVertexShader = vertexShader;
	}

	// Check if currently set fragment shader is the same as the one to be enabled - do nothing then. 
	if ( currentFragmentShader.expired() || !currentFragmentShader.lock()->isSame( *fragmentShader ) ) 
	{
		deviceContext->PSSetShader( &fragmentShader->getShader(), nullptr, 0 );

        currentFragmentShader = fragmentShader;
	}

    vertexOrFragmentShaderEnabled = true;
}

void Direct3DRendererCore::enableComputeShader( std::shared_ptr<const ComputeShader> computeShader )
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableComputeShader - renderer not initialized." );
    if ( !computeShader ) throw std::exception( "Direct3DRendererCore::enableComputeShader - passed shader is nullptr." );

    disableRenderingShaders();

    // Check if currently set compute shader is the same as the one to be enabled - do nothing then. 
    if ( currentComputeShader.expired() || !currentComputeShader.lock()->isSame( *computeShader ) ) {
        deviceContext->CSSetShader( &computeShader->getShader(), nullptr, 0 );

        currentComputeShader = computeShader;
    }

    computeShaderEnabled = true;
}

void Direct3DRendererCore::disableRenderingShaders()
{
    if ( vertexOrFragmentShaderEnabled ) {
        deviceContext->IASetInputLayout( nullptr );
        deviceContext->VSSetShader( nullptr, nullptr, 0 );
        deviceContext->PSSetShader( nullptr, nullptr, 0 );
        
        currentVertexShader.reset();
        currentFragmentShader.reset();

        vertexOrFragmentShaderEnabled = false;
    }
}

void Direct3DRendererCore::disableComputeShaders()
{
    if ( computeShaderEnabled ) {
        deviceContext->CSSetShader( nullptr, nullptr, 0 );
        currentComputeShader.reset();

        computeShaderEnabled = false;
    }
}

void Direct3DRendererCore::enableRenderTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >& renderTargetsF2,
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >& renderTargetsF4,
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > >& renderTargetsU1, 
                                                const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >& renderTargetsU4, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget,
                                                const int mipmapLevel )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - renderer not initialized." );
	if ( renderTargetsF2.size() + renderTargetsU4.size() > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - too many render targets passed. Number exceeds the supported maximum." );

	bool sameAsCurrent = true;

	{ // Check if render targets to be enabled are the same as the current ones.
		//if ( renderTargets.size() == currentRenderTargetViews.size() ) {
			for ( unsigned int i = 0; i < renderTargetsF2.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( currentRenderTargetViews.size() <= i || renderTargetsF2.at( i )->getRenderTargetView( mipmapLevel ) != currentRenderTargetViews.at( i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int first = (unsigned int)renderTargetsF2.size();
            for ( unsigned int i = 0; i < renderTargetsF4.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( currentRenderTargetViews.size() <= (first + i) || renderTargetsF4.at( i )->getRenderTargetView( mipmapLevel ) != currentRenderTargetViews.at( first + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int second = first + (unsigned int)renderTargetsF4.size();
            for ( unsigned int i = 0; i < renderTargetsU1.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( currentRenderTargetViews.size() <= (second + i) || renderTargetsU1.at( i )->getRenderTargetView( mipmapLevel ) != currentRenderTargetViews.at( second + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int third = second + (unsigned int)renderTargetsU1.size();
            for ( unsigned int i = 0; i < renderTargetsU4.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( currentRenderTargetViews.size() <= (second + i) || renderTargetsU4.at( i )->getRenderTargetView( mipmapLevel ) != currentRenderTargetViews.at( third + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}
		//} else {
		//	// Render targets are not the same as the current ones, because they have different count.
		//	sameAsCurrent = false;
		//}

		// Check if depth render target to be enabled is the same as the current one.
		if ( ( depthRenderTarget == nullptr ) != ( currentDepthRenderTargetView == nullptr ) 
             || ( depthRenderTarget && depthRenderTarget->getDepthStencilView( mipmapLevel ) == currentDepthRenderTargetView ) )
			sameAsCurrent = false;
	}

	// If render targets to be enabled are the same as the current ones - do nothing.
	if ( sameAsCurrent )
		return;

	// Collect and save render target views from passed render targets.
    currentRenderTargetViews.clear();
	for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > >& renderTarget : renderTargetsF2 )
		currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > >& renderTarget : renderTargetsF4 )
		currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > >& renderTarget : renderTargetsU1 )
		currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > >& renderTarget : renderTargetsU4 )
		currentRenderTargetViews.push_back( renderTarget->getRenderTargetView( mipmapLevel ) );

	// Get and save depth render target view if passed.
	if ( depthRenderTarget )
		currentDepthRenderTargetView = depthRenderTarget->getDepthStencilView( mipmapLevel );

	// Enable render targets.
	deviceContext->OMSetRenderTargets( (unsigned int)currentRenderTargetViews.size(), currentRenderTargetViews.data(), currentDepthRenderTargetView );
}

void Direct3DRendererCore::enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > > unorderedAccessTargetsF1,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1,
                                                         const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4,
                                                         const int mipmapLevel )
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableUnorderedAccessTargets - renderer not initialized." );
    //#TODO: WARNING: For pixel shaders, UAVStartSlot param should be equal to the number of render-target views being bound.

    bool sameAsCurrent = true;

	{ // Check if UAV targets to be enabled are the same as the current ones.
		//if ( unorderedAccessTargets.size() == currentUnorderedAccessTargetViews.size() ) {
			for ( unsigned int i = 0; i < unorderedAccessTargetsF1.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( currentUnorderedAccessTargetViews.size() <= i || unorderedAccessTargetsF1.at( i )->getUnorderedAccessView( mipmapLevel ) != currentUnorderedAccessTargetViews.at( i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int first = (unsigned int)unorderedAccessTargetsF1.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsF2.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( currentUnorderedAccessTargetViews.size() <= (first + i) || unorderedAccessTargetsF2.at( i )->getUnorderedAccessView( mipmapLevel ) != currentUnorderedAccessTargetViews.at( first + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int second = first + (unsigned int)unorderedAccessTargetsF2.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsF4.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( currentUnorderedAccessTargetViews.size() <= (second + i) || unorderedAccessTargetsF4.at( i )->getUnorderedAccessView( mipmapLevel ) != currentUnorderedAccessTargetViews.at( second + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int third = second + (unsigned int)unorderedAccessTargetsF4.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsU1.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( currentUnorderedAccessTargetViews.size() <= (third + i) || unorderedAccessTargetsU1.at( i )->getUnorderedAccessView( mipmapLevel ) != currentUnorderedAccessTargetViews.at( third + i ) ) {
					sameAsCurrent = false;
					break;
				}
			}

            const unsigned int fourth = third + (unsigned int)unorderedAccessTargetsU1.size();
            for ( unsigned int i = 0; i < unorderedAccessTargetsU4.size(); ++i ) {
				// Check each pair of UAV targets at corresponding indexes.
				if ( currentUnorderedAccessTargetViews.size() <= (third + i) || unorderedAccessTargetsU4.at( i )->getUnorderedAccessView( mipmapLevel ) != currentUnorderedAccessTargetViews.at( fourth + i ) ) {
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
    currentUnorderedAccessTargetViews.clear();
	for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > >& unorderedAccessTarget : unorderedAccessTargetsF1 )
		currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > >& unorderedAccessTarget : unorderedAccessTargetsF2 )
		currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > >& unorderedAccessTarget : unorderedAccessTargetsF4 )
		currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > >& unorderedAccessTarget : unorderedAccessTargetsU1 )
		currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) );

    for ( const std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > >& unorderedAccessTarget : unorderedAccessTargetsU4 )
		currentUnorderedAccessTargetViews.push_back( unorderedAccessTarget->getUnorderedAccessView( mipmapLevel ) );

	// Enable UAV targets.
    deviceContext->CSSetUnorderedAccessViews( 0, (unsigned int)currentUnorderedAccessTargetViews.size(), currentUnorderedAccessTargetViews.data(), nullptr );
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

void Direct3DRendererCore::disableRenderTargetViews()
{
    if ( !currentRenderTargetViews.empty() || currentDepthRenderTargetView )
    {
        std::vector< ID3D11RenderTargetView* > emptyTargets;
        emptyTargets.resize( currentRenderTargetViews.size() );
        for (unsigned int i = 0; i < currentRenderTargetViews.size(); ++i)
            emptyTargets[ i ] = nullptr;

        ID3D11DepthStencilView* emptyDepthTarget = nullptr;

        deviceContext->OMSetRenderTargets( (unsigned int)currentRenderTargetViews.size(), emptyTargets.data(), emptyDepthTarget );

        currentRenderTargetViews.clear();
        currentDepthRenderTargetView = nullptr;
    }
}

void Direct3DRendererCore::disableUnorderedAccessViews()
{
    if ( !currentUnorderedAccessTargetViews.empty() )
    {
        std::vector< ID3D11UnorderedAccessView* > emptyTargets;
        emptyTargets.resize( currentUnorderedAccessTargetViews.size() );
        for (unsigned int i = 0; i < currentUnorderedAccessTargetViews.size(); ++i)
            emptyTargets[ i ] = nullptr;

        deviceContext->CSSetUnorderedAccessViews( 0, (unsigned int)currentUnorderedAccessTargetViews.size(), emptyTargets.data(), nullptr );
        //deviceContext->OMSetRenderTargetsAndUnorderedAccessViews( D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, 0, nullptr, nullptr );

        currentUnorderedAccessTargetViews.clear();
    }
}

void Direct3DRendererCore::enableRasterizerState( ID3D11RasterizerState& rasterizerState )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableRasterizerState - renderer not initialized." );

	// Change rasterizer state if new state is different than the current one.
	if ( &rasterizerState != currentRasterizerState ) {
		deviceContext->RSSetState( &rasterizerState );

		currentRasterizerState = &rasterizerState;
	}
}

void Direct3DRendererCore::enableDepthStencilState( ID3D11DepthStencilState& depthStencilState )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableDepthStencilState - renderer not initialized." );

	// Change depth stencil state if new state is different than the current one.
	if ( &depthStencilState != currentDepthStencilState ) {
		deviceContext->OMSetDepthStencilState( &depthStencilState, 0 );

		currentDepthStencilState = &depthStencilState;
	}
}

void Direct3DRendererCore::enableBlendState( ID3D11BlendState& blendState )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableBlendState - renderer not initialized." );

	// Change blend state if new state is different than the current one.
	if ( &blendState != currentBlendState ) {

		float blendFactor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;
		deviceContext->OMSetBlendState( &blendState, blendFactor, sampleMask );

		currentBlendState = &blendState;
	}
}

void Direct3DRendererCore::enableDefaultRasterizerState()
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultRasterizerState - renderer not initialized." );

    if ( currentRasterizerState ) {
        deviceContext->RSSetState( nullptr );

        currentRasterizerState = nullptr;
    }
}

void Direct3DRendererCore::enableDefaultDepthStencilState()
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultDepthStencilState - renderer not initialized." );

    if ( currentDepthStencilState ) {
        deviceContext->OMSetDepthStencilState( nullptr, 0 );

        currentDepthStencilState = nullptr;
    }
}

void Direct3DRendererCore::enableDefaultBlendState()
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultBlendState - renderer not initialized." );

	if ( currentBlendState ) {

		UINT sampleMask = 0xffffffff;
		deviceContext->OMSetBlendState( nullptr, nullptr, sampleMask );

		currentBlendState = nullptr;
	}
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const RectangleMesh& mesh )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );
	if ( !mesh.isInGpuMemory() ) throw std::exception( "Direct3DRenderer::drawRectangleMesh - mesh hasn't been loaded to GPU yet" );

	{ // set mesh buffers
		if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() && mesh.getTexcoordBuffer() > 0 ) {
			const unsigned int bufferCount = 3;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ), sizeof( float2 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer(), mesh.getTexcoordBuffer() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() ) {

			const unsigned int bufferCount = 2;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() ) {

			const unsigned int bufferCount = 1;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		}
	}

	// draw mesh
	deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const BlockMesh& mesh )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );
	if ( !mesh.isInGpuMemory() ) throw std::exception( "Direct3DRenderer::drawBlockMesh - mesh hasn't been loaded to GPU yet" );

	{ // set mesh buffers
		if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() && mesh.getTexcoordBuffers().size() > 0 && mesh.getTexcoordBuffers().front() ) {
			const unsigned int bufferCount = 3;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ), sizeof( float2 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer(), mesh.getTexcoordBuffers().front() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() && mesh.getNormalBuffer() ) {

			const unsigned int bufferCount = 2;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ), sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0, 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer(), mesh.getNormalBuffer() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		} else if ( mesh.getVertexBuffer() ) {

			const unsigned int bufferCount = 1;
			unsigned int strides[ bufferCount ] = { sizeof( float3 ) };
			unsigned int offsets[ bufferCount ] = { 0 };
			ID3D11Buffer* buffers[ bufferCount ] = { mesh.getVertexBuffer() };

			deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
			deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		}
	}

	// draw mesh
	deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const SkeletonMesh& mesh )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );

	//TODO: move this tests to some method? To which class?
	if ( mesh.getVertexBones().empty() )   throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh doesn't have vertex-bone assignemnts." );
	if ( mesh.getVertexWeights().empty() ) throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh doesn't have vertex weights." );
	if ( !mesh.isInGpuMemory() )           throw std::exception( "Direct3DRenderer::drawSkeletonMesh - mesh is not in GPU memory." );

	const bool hasNormals   = mesh.getNormalBuffer() != nullptr;
	const bool hasTexcoords = !mesh.getTexcoordBuffers().empty() && mesh.getTexcoordBuffers().front() != nullptr;

	unsigned int bufferCount = 3; //vertices + vertex-bones + vertex-weights
	if ( hasNormals ) ++bufferCount; //normals
	if ( hasTexcoords ) ++bufferCount; //texcoords

	const unsigned int vertexStride        = sizeof( float3 );
	const unsigned int vertexBonesStride   = static_cast<unsigned int>( mesh.getBonesPerVertexCount() ) * sizeof( unsigned char );
	const unsigned int vertexWeightsStride = static_cast<unsigned int>( mesh.getBonesPerVertexCount() ) * sizeof( float );
	const unsigned int normalStride        = sizeof( float3 );
	const unsigned int texcoordStride      = sizeof( float2 );

	const unsigned int vertexOffset        = 0;
	const unsigned int vertexBonesOffset   = 0;
	const unsigned int vertexWeightsOffset = 0;
	const unsigned int normalOffset        = 0;
	const unsigned int texcoordOffset      = 0;

	std::vector<unsigned int> strides;
	strides.push_back( vertexStride );
	strides.push_back( vertexBonesStride );
	strides.push_back( vertexWeightsStride );
	if ( hasNormals )   strides.push_back( normalStride );
	if ( hasTexcoords ) strides.push_back( texcoordStride );

	std::vector<unsigned int> offsets;
	offsets.push_back( vertexOffset );
	offsets.push_back( vertexBonesOffset );
	offsets.push_back( vertexWeightsOffset );
	if ( hasNormals ) offsets.push_back( normalOffset );
	if ( hasTexcoords ) offsets.push_back( texcoordOffset );

	std::vector<ID3D11Buffer*> buffers;
	buffers.push_back( mesh.getVertexBuffer() );
	buffers.push_back( mesh.getVertexBonesBuffer() );
	buffers.push_back( mesh.getVertexWeightsBuffer() );
	if ( hasNormals )   buffers.push_back( mesh.getNormalBuffer() );
	if ( hasTexcoords ) buffers.push_back( mesh.getTexcoordBuffers().front() );

	deviceContext->IASetVertexBuffers( 0, bufferCount, buffers.data(), strides.data(), offsets.data() );
	deviceContext->IASetIndexBuffer( mesh.getTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
	deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Draw the mesh.
	deviceContext->DrawIndexed( (unsigned int)mesh.getTriangles().size() * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::draw( const FontCharacter& character )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::draw - renderer not initialized." );

	const unsigned int bufferCount = 2;
	unsigned int  strides[ bufferCount ] = { sizeof( float3 ), sizeof( float2 ) };
	unsigned int  offsets[ bufferCount ] = { 0, 0 };
	ID3D11Buffer* buffers[ bufferCount ] = { character.getVertexBuffer(), FontCharacter::getDefaultTexcoordsBuffer() };

	deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, strides, offsets );
	deviceContext->IASetIndexBuffer( FontCharacter::getDefaultTriangleBuffer(), DXGI_FORMAT_R32_UINT, 0 );
	deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Draw the mesh.
	const unsigned int triangleCount = 2;
	deviceContext->DrawIndexed( triangleCount * uint3::size(), 0, 0 );
}

// Note: Shaders need to be configured and set before calling this method.
void Direct3DRendererCore::compute( uint3 threadCount )
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::compute - renderer not initialized." );

    deviceContext->Dispatch( threadCount.x, threadCount.y, threadCount.z );
}

void Direct3DRendererCore::disableShaderInputs()
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::disableShaderInputs - renderer not initialized." );

    deviceContext->IASetVertexBuffers( 0, (unsigned int)nullVertexBuffers.size(), nullVertexBuffers.data(), nullVertexBuffersStrideOffset.data(), nullVertexBuffersStrideOffset.data() );
    deviceContext->IASetIndexBuffer( nullptr, DXGI_FORMAT_UNKNOWN, 0 );
    deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_UNDEFINED );

    deviceContext->PSSetShaderResources( 0, (unsigned int)nullResources.size(), nullResources.data() );
    deviceContext->PSSetSamplers( 0, (unsigned int)nullSamplers.size(), nullSamplers.data() );
}

void Direct3DRendererCore::createNullShaderInputs()
{
    nullVertexBuffers.resize( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT  );
    nullVertexBuffersStrideOffset.resize( D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
    for ( int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ; ++i ) {
        nullVertexBuffers[ i ] = nullptr;
        nullVertexBuffersStrideOffset[ i ] = 0;
    }

    nullResources.resize( D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT );
    for ( int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i )
        nullResources[ i ] = nullptr;

    nullSamplers.resize( D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT );
    for ( int i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i )
        nullSamplers[ i ] = nullptr;
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                                        const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture )
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    deviceContext->CopyResource( destTexture->getTextureResource().Get(), srcTexture->getTextureResource().Get() );
}

void Direct3DRendererCore::copyTexture( std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > srcTexture )
{
    if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

    deviceContext->CopyResource( destTexture->getTextureResource().Get(), srcTexture->getTextureResource().Get() );
}