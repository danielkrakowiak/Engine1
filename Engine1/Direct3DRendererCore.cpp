#include "Direct3DRendererCore.h"

#include "RectangleMesh.h"
#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "Font.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "SkeletonMeshVertexShader.h"
#include "RenderTargetTexture2D.h"
#include "RenderTargetDepthTexture2D.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Direct3DRendererCore::Direct3DRendererCore() :
deviceContext( nullptr ),
currentRenderTargets(),
currentRasterizerState( nullptr ),
currentDepthStencilState( nullptr ),
currentBlendState( nullptr )
{}


Direct3DRendererCore::~Direct3DRendererCore()
{}

void Direct3DRendererCore::initialize( ID3D11DeviceContext& deviceContext )
{
	this->deviceContext = &deviceContext;
}

void Direct3DRendererCore::enableShaders( const VertexShader& vertexShader, const FragmentShader& fragmentShader )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableShaders - renderer not initialized." );

	// Check if currently set vertex shader is the same as the one to be enabled - do nothing then. 
	if ( currentVertexShader.expired() || !currentVertexShader.lock()->isSame( vertexShader ) ) 
	{
		deviceContext->IASetInputLayout( &vertexShader.getInputLauout() );
		deviceContext->VSSetShader( &vertexShader.getShader(), nullptr, 0 );
	}

	// Check if currently set fragment shader is the same as the one to be enabled - do nothing then. 
	if ( currentFragmentShader.expired() || !currentFragmentShader.lock()->isSame( fragmentShader ) ) 
	{
		deviceContext->PSSetShader( &fragmentShader.getShader(), nullptr, 0 );
	}
}

void Direct3DRendererCore::enableRenderTargets( const std::vector< std::shared_ptr<RenderTarget2D> >& renderTargets, const std::shared_ptr<RenderTargetDepth2D> depthRenderTarget )
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - renderer not initialized." );
	if ( renderTargets.size() > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ) throw std::exception( "Direct3DRendererCore::enableRenderTargets - too many render targets passed. Number exceeds the supported maximum." );

	bool sameAsCurrent = true;

	{ // Check if render targets to be enabled are the same as the current ones.
		if ( renderTargets.size() == currentRenderTargets.size() ) {
			for ( unsigned int i = 0; i < renderTargets.size(); ++i ) {
				// Check each pair of render targets at corresponding indexes.
				if ( renderTargets.at( i ) != currentRenderTargets.at( i ).lock() ) {
					sameAsCurrent = false;
					break;
				}
			}
		} else {
			// Render targets are not the same as the current ones, because they have different count.
			sameAsCurrent = false;
		}

		// Check if depth render target to be enabled is the same as the current one.
		if ( depthRenderTarget != currentDepthRenderTarget.lock() )
			sameAsCurrent = false;
	}

	// If render targets to be enabled are the same as the current ones - do nothing.
	if ( sameAsCurrent )
		return;

	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	ID3D11DepthStencilView* depthRenderTargetView = nullptr;

	// Collect render target views from passed render targets.
	for ( const std::shared_ptr<RenderTarget2D>& renderTarget : renderTargets )
		renderTargetViews.push_back( renderTarget->getRenderTarget() );

	// Get depth render target view if passed.
	if ( depthRenderTarget )
		depthRenderTargetView = depthRenderTarget->getDepthRenderTarget();

	// Enable render targets.
	deviceContext->OMSetRenderTargets( renderTargetViews.size(), renderTargetViews.data(), depthRenderTargetView );
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

void Direct3DRendererCore::enableDefaultBlendState()
{
	if ( !deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultBlendState - renderer not initialized." );

	// Change blend state if new state is different than the current one.
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
	deviceContext->DrawIndexed( mesh.getTriangles().size() * uint3::size(), 0, 0 );
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
	deviceContext->DrawIndexed( mesh.getTriangles().size() * uint3::size(), 0, 0 );
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
	deviceContext->DrawIndexed( mesh.getTriangles().size() * uint3::size(), 0, 0 );
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