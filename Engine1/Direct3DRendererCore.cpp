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
currentRenderTargets(),
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
    //#TODO: Should deviceContext->IASetInputLayout( nullptr ); be called here? is Input Assembler used in Compute Shaders?
    if ( computeShaderEnabled ) {
        deviceContext->CSSetShader( nullptr, nullptr, 0 );
        currentComputeShader.reset();

        computeShaderEnabled = false;
    }
}

void Direct3DRendererCore::enableRenderTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >& renderTargets, 
                                                const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget )
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
	for ( const std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > >& renderTarget : renderTargets )
		renderTargetViews.push_back( renderTarget->getRenderTargetView() );

	// Get depth render target view if passed.
	if ( depthRenderTarget )
		depthRenderTargetView = depthRenderTarget->getDepthStencilView();

	// Enable render targets.
	deviceContext->OMSetRenderTargets( (unsigned int)renderTargetViews.size(), renderTargetViews.data(), depthRenderTargetView );

    // Save current render targets.
    currentRenderTargets.clear();
    currentRenderTargets.insert( currentRenderTargets.begin(), renderTargets.begin(), renderTargets.end() );
    currentDepthRenderTarget = depthRenderTarget;
}

void Direct3DRendererCore::enableComputeTarget( std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > computeTarget )
{
    //#TODO: WARNING: For pixel shaders, UAVStartSlot param should be equal to the number of render-target views being bound.

    if ( !computeTarget ) throw std::exception( "Direct3DRendererCore::enableComputeTarget - passed target is nullptr." );

    if ( computeTarget != currentComputeTarget.lock() )
    {
        ID3D11UnorderedAccessView* uavs[1] = { computeTarget->getUnorderedAccessView() };
        deviceContext->CSSetUnorderedAccessViews( 0, 1, uavs, nullptr );
        
        currentComputeTarget = computeTarget;
    }
}

void Direct3DRendererCore::disableRenderTargets()
{
    if ( !currentRenderTargets.empty() || currentDepthRenderTarget.lock() )
    {
        deviceContext->OMSetRenderTargets( 0, nullptr, nullptr );

        currentRenderTargets.clear();
        currentDepthRenderTarget.reset();
    }
}

void Direct3DRendererCore::disableComputeTargets()
{
    if ( currentComputeTarget.lock() )
    {
        ID3D11UnorderedAccessView* uavs[1] = { nullptr };
        deviceContext->CSSetUnorderedAccessViews( 0, 1, uavs, nullptr );
        //deviceContext->OMSetRenderTargetsAndUnorderedAccessViews( D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, 0, nullptr, nullptr );

        currentComputeTarget.reset();
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