#include "DX11RendererCore.h"

#include "RectangleMesh.h"
#include "BlockMesh.h"
#include "SkeletonMesh.h"
#include "Font.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "ComputeShader.h"
#include "SkeletonMeshVertexShader.h"

#include "MathUtil.h"

#include <d3d11_3.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

ID3D11RenderTargetView* RenderTargets::getRTV( size_t idx, int mipmapLevel ) const
{
	if ( idx < typeFloat.size() ) return typeFloat[ idx ]->getRenderTargetView( mipmapLevel );
	else                          idx -= typeFloat.size();

	if ( idx < typeFloat2.size() ) return typeFloat2[ idx ]->getRenderTargetView( mipmapLevel );
	else                           idx -= typeFloat2.size();

	if ( idx < typeFloat3.size() ) return typeFloat3[ idx ]->getRenderTargetView( mipmapLevel );
	else                           idx -= typeFloat3.size();

	if ( idx < typeFloat4.size() ) return typeFloat4[ idx ]->getRenderTargetView( mipmapLevel );
	else                           idx -= typeFloat4.size();

	if ( idx < typeUchar.size() ) return typeUchar[ idx ]->getRenderTargetView( mipmapLevel );
	else                          idx -= typeUchar.size();

	if ( idx < typeUchar4.size() ) return typeUchar4[ idx ]->getRenderTargetView( mipmapLevel );
	else                           idx -= typeUchar4.size();

	return nullptr;
}

ID3D11UnorderedAccessView* RenderTargets::getUAV( size_t idx, int mipmapLevel ) const
{
	if ( idx < typeFloat.size() ) return typeFloat[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                          idx -= typeFloat.size();

	if ( idx < typeFloat2.size() ) return typeFloat2[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                           idx -= typeFloat2.size();

	if ( idx < typeFloat3.size() ) return typeFloat3[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                           idx -= typeFloat3.size();

	if ( idx < typeFloat4.size() ) return typeFloat4[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                           idx -= typeFloat4.size();

	if ( idx < typeUchar.size() ) return typeUchar[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                          idx -= typeUchar.size();

	if ( idx < typeUchar4.size() ) return typeUchar4[ idx ]->getUnorderedAccessView( mipmapLevel );
	else                           idx -= typeUchar4.size();

	return nullptr;
}

size_t RenderTargets::getCount() const
{
	return typeFloat.size() + typeFloat2.size()
		+ typeFloat3.size() + typeFloat4.size()
		+ typeUchar.size() + typeUchar4.size();
}

DX11RendererCore::DX11RendererCore() :
m_deviceContext( nullptr ),
viewportDimensions( float2::ZERO ),
viewportTopLeft( float2::ZERO ),
viewportDepthMin( 0.0f ),
viewportDepthMax( 0.0f ),
m_currentRTVs(),
m_currentDSV( nullptr ),
m_graphicsShaderEnabled( false ),
m_computeShaderEnabled( false ),
m_currentRasterizerState( nullptr ),
m_currentDepthStencilState( nullptr ),
m_currentBlendState( nullptr )
{
    createNullShaderInputs();
}


DX11RendererCore::~DX11RendererCore()
{}

void DX11RendererCore::initialize( ID3D11DeviceContext3& deviceContext )
{
	this->m_deviceContext = &deviceContext;
}

void DX11RendererCore::disableRenderingPipeline()
{
    disableRenderingShaders();
    disableRenderTargets();
    enableDefaultBlendState();
    enableDefaultRasterizerState();
    enableDefaultDepthStencilState();
    disableShaderInputs();
}

void DX11RendererCore::disableComputePipeline()
{
    disableComputeShaders();
	disableRenderTargets();
}

void DX11RendererCore::enableRenderingShaders( std::shared_ptr<const VertexShader> vertexShader, std::shared_ptr<const FragmentShader> fragmentShader )
{
    //#TODO: To be removed after Renderer refactor.

    if ( fragmentShader )
	    enableRenderingShaders( *vertexShader, *fragmentShader );
    else
        enableRenderingShaders( *vertexShader );
}

void DX11RendererCore::enableRenderingShaders( const VertexShader& vertexShader )
{
    if ( !m_deviceContext )
        throw std::exception( "Direct3DRendererCore::enableRenderingShaders - renderer not initialized." );

    disableComputeShaders();

    // Check if currently set vertex shader is the same as the one to be enabled - do nothing then. 
    if ( m_currentVertexShader != &vertexShader ) {
        m_deviceContext->IASetInputLayout( &vertexShader.getInputLauout() );
        m_deviceContext->VSSetShader( &vertexShader.getShader(), nullptr, 0 );

        m_currentVertexShader = &vertexShader;
    }

    // Disable fragment shader.
    if ( m_currentFragmentShader )
    {
        m_deviceContext->PSSetShader( nullptr, nullptr, 0 );

        m_currentFragmentShader = nullptr;
    }

    m_graphicsShaderEnabled = true;
}

void DX11RendererCore::enableRenderingShaders( const VertexShader& vertexShader, const FragmentShader& fragmentShader )
{
    if ( !m_deviceContext )
        throw std::exception( "Direct3DRendererCore::enableRenderingShaders - renderer not initialized." );

    disableComputeShaders();

    // Check if currently set vertex shader is the same as the one to be enabled - do nothing then. 
    if ( m_currentVertexShader != &vertexShader ) 
    {
        m_deviceContext->IASetInputLayout( &vertexShader.getInputLauout() );
        m_deviceContext->VSSetShader( &vertexShader.getShader(), nullptr, 0 );

        m_currentVertexShader = &vertexShader;
    }

    // Check if currently set fragment shader is the same as the one to be enabled - do nothing then.
    if ( m_currentFragmentShader != &fragmentShader ) 
    {
        m_deviceContext->PSSetShader( &fragmentShader.getShader(), nullptr, 0 );

        m_currentFragmentShader = &fragmentShader;
    }

    m_graphicsShaderEnabled = true;
}

void DX11RendererCore::enableComputeShader( std::shared_ptr<const ComputeShader> computeShader )
{
    //#TODO: To be removed after Renderer refactor.

    enableComputeShader( *computeShader );
}

void DX11RendererCore::enableComputeShader( const ComputeShader& computeShader )
{
    if ( !m_deviceContext ) 
        throw std::exception( "Direct3DRendererCore::enableComputeShader - renderer not initialized." );

    disableRenderingShaders();

    // Check if currently set compute shader is the same as the one to be enabled - do nothing then. 
    if ( m_currentComputeShader != &computeShader ) 
    {
        m_deviceContext->CSSetShader( &computeShader.getShader(), nullptr, 0 );

        m_currentComputeShader = &computeShader;
    }

    m_computeShaderEnabled = true;
}

void DX11RendererCore::disableRenderingShaders()
{
    if ( m_graphicsShaderEnabled ) 
    {
        m_deviceContext->IASetInputLayout( nullptr );
        m_deviceContext->VSSetShader( nullptr, nullptr, 0 );
        m_deviceContext->PSSetShader( nullptr, nullptr, 0 );
        
        m_currentVertexShader   = nullptr;
        m_currentFragmentShader = nullptr;

        m_graphicsShaderEnabled = false;
    }
}

void DX11RendererCore::disableComputeShaders()
{
    if ( m_computeShaderEnabled ) 
    {
        m_deviceContext->CSSetShader( nullptr, nullptr, 0 );

        m_currentComputeShader = nullptr;

        m_computeShaderEnabled = false;
    }
}

void DX11RendererCore::setViewport( float2 dimensions, float2 topLeft, float depthMin, float depthMax )
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

void DX11RendererCore::enableRenderTargets( 
    const RenderTargets& renderTargets,
	const RenderTargets& unorderedAccessTargets,
    const int mipmapLevel )
{
	if ( !m_deviceContext ) 
		throw std::exception( "Direct3DRendererCore::enableRenderTargets - renderer-core not initialized." );

	if ( renderTargets.depth && renderTargets.depthStencil )
		throw std::exception( "Direct3DRendererCore::enableRenderTargets - depth and depth-stencil were passed, but only one of them can be enabled." );

    const auto rtvCount = renderTargets.getCount();
    const auto uavCount = unorderedAccessTargets.getCount();

    // Disable redundant RTVs and UAVs (by disabling them all).
    if ( rtvCount < m_currentRTVs.size() || uavCount < m_currentUAVs.size() )
        disableRenderTargets();

	bool rtvSameAsCurrent = true;
	{
		if ( rtvCount > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )
			throw std::exception( "Direct3DRendererCore::enableRenderTargets - too many render targets passed. Number exceeds the supported maximum." );

		// Check if render targets to be enabled are the same as the current ones.
		if ( rtvCount == m_currentRTVs.size() )
		{
			for ( auto rtvIdx = 0u; rtvIdx < m_currentRTVs.size(); ++rtvIdx )
			{
                assert( renderTargets.getRTV( rtvIdx, mipmapLevel ) != nullptr );
				if ( renderTargets.getRTV( rtvIdx, mipmapLevel ) != m_currentRTVs[ rtvIdx ] )
				{
					rtvSameAsCurrent = false;
					break;
				}
			}
		}
		else
		{
			rtvSameAsCurrent = false;
		}

		// Check if depth render target to be enabled is the same as the current one.
		ID3D11DepthStencilView* depthRTV = nullptr;
		if ( renderTargets.depthStencil )
			depthRTV = renderTargets.depthStencil->getDepthStencilView( mipmapLevel );
		else if ( renderTargets.depth )
			depthRTV = renderTargets.depth->getDepthStencilView( mipmapLevel );

		if ( depthRTV != m_currentDSV )
		{
			rtvSameAsCurrent = false;
		}

		if ( !rtvSameAsCurrent )
        {
		    // Collect and save render target views from passed render targets.
		    m_currentRTVs.clear();
		    m_currentRTVs.reserve( rtvCount );
		    for ( auto rtvIdx = 0u; rtvIdx < rtvCount; ++rtvIdx )
		    {
                assert( renderTargets.getRTV( rtvIdx, mipmapLevel ) != nullptr );
			    m_currentRTVs.push_back( renderTargets.getRTV( rtvIdx, mipmapLevel ) );
		    }

		    // Get and save depth render target view if passed.
		    m_currentDSV = depthRTV;

		    // Enable render targets.
            if ( !m_currentRTVs.empty() || m_currentDSV )
            {
		        m_deviceContext->OMSetRenderTargets( 
                    static_cast<UINT>(m_currentRTVs.size()), 
                    m_currentRTVs.data(), 
                    m_currentDSV );
            }
        }
	}

	bool uavSameAsCurrent = true;
	{
		// Check if UAVs to be enabled are the same as the current ones.
		if ( uavCount == m_currentUAVs.size() )
		{
			for ( auto uavIdx = 0u; uavIdx < m_currentUAVs.size(); ++uavIdx )
			{
                assert( unorderedAccessTargets.getUAV( uavIdx, mipmapLevel ) != nullptr );
				if ( unorderedAccessTargets.getUAV( uavIdx, mipmapLevel ) != m_currentUAVs[ uavIdx ] )
				{
					uavSameAsCurrent = false;
					break;
				}
			}
		}
		else
		{
			uavSameAsCurrent = false;
		}

        // Note: even if only RTVs have changed, UAVS need to be re-bound at correct slots.
		if ( !rtvSameAsCurrent || !uavSameAsCurrent )
        {
		    // Collect and save UAVs from passed render targets.
		    m_currentUAVs.clear();
		    m_currentUAVs.reserve( uavCount );
		    for ( auto uavIdx = 0u; uavIdx < uavCount; ++uavIdx )
		    {
                assert( unorderedAccessTargets.getUAV( uavIdx, mipmapLevel ) != nullptr );
			    m_currentUAVs.push_back( unorderedAccessTargets.getUAV( uavIdx, mipmapLevel ) );
		    }

		    // Enable UAV targets.
		    // Note: UAVStartSlot param should be equal to the number of bound RTVs.
            if ( !m_currentUAVs.empty() )
            {
		        m_deviceContext->CSSetUnorderedAccessViews( 
			        static_cast<UINT>(m_currentRTVs.size()), 
			        static_cast<UINT>(m_currentUAVs.size()),
			        m_currentUAVs.data(), 
			        nullptr );
            }
        }
	}
}

void DX11RendererCore::disableRenderTargets()
{
    if ( !m_currentRTVs.empty() || m_currentDSV || !m_currentUAVs.empty() )
    {
		for ( auto i = 0; i < m_currentRTVs.size(); ++i ) {
			m_currentRTVs[ i ] = nullptr;
		}

		for ( auto i = 0; i < m_currentUAVs.size(); ++i ) {
			m_currentUAVs[ i ] = nullptr;
		}

        if (!m_currentRTVs.empty())
        {
            m_deviceContext->OMSetRenderTargets( 
			    static_cast<UINT>(m_currentRTVs.size()), 
			    m_currentRTVs.data(), 
			    nullptr );
        }

        if (!m_currentUAVs.empty())
        {
		    m_deviceContext->CSSetUnorderedAccessViews( 
			    static_cast<UINT>(m_currentRTVs.size()), 
			    static_cast<UINT>(m_currentUAVs.size()), 
			    m_currentUAVs.data(), 
			    nullptr );
        }

        m_currentRTVs.clear();
        m_currentDSV = nullptr;
		m_currentUAVs.clear();
    }
}

void DX11RendererCore::enableRasterizerState( ID3D11RasterizerState& rasterizerState )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableRasterizerState - renderer not initialized." );

	// Change rasterizer state if new state is different than the current one.
	if ( &rasterizerState != m_currentRasterizerState ) {
		m_deviceContext->RSSetState( &rasterizerState );

		m_currentRasterizerState = &rasterizerState;
	}
}

void DX11RendererCore::enableDepthStencilState( ID3D11DepthStencilState& depthStencilState )
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDepthStencilState - renderer not initialized." );

	// Change depth stencil state if new state is different than the current one.
	if ( &depthStencilState != m_currentDepthStencilState ) {
		m_deviceContext->OMSetDepthStencilState( &depthStencilState, 0 );

		m_currentDepthStencilState = &depthStencilState;
	}
}

void DX11RendererCore::enableBlendState( ID3D11BlendState& blendState )
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

void DX11RendererCore::enableDefaultRasterizerState()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultRasterizerState - renderer not initialized." );

    if ( m_currentRasterizerState ) {
        m_deviceContext->RSSetState( nullptr );

        m_currentRasterizerState = nullptr;
    }
}

void DX11RendererCore::enableDefaultDepthStencilState()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultDepthStencilState - renderer not initialized." );

    if ( m_currentDepthStencilState ) {
        m_deviceContext->OMSetDepthStencilState( nullptr, 0 );

        m_currentDepthStencilState = nullptr;
    }
}

void DX11RendererCore::enableDefaultBlendState()
{
	if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::enableDefaultBlendState - renderer not initialized." );

	if ( m_currentBlendState ) {

		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState( nullptr, nullptr, sampleMask );

		m_currentBlendState = nullptr;
	}
}

// Note: Shaders need to be configured and set before calling this method.
void DX11RendererCore::draw( const RectangleMesh& mesh )
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
void DX11RendererCore::draw( const BlockMesh& mesh )
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
void DX11RendererCore::draw( const SkeletonMesh& mesh )
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
void DX11RendererCore::draw( const FontCharacter& character )
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
void DX11RendererCore::compute( uint3 groupCount )
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::compute - renderer not initialized." );

    m_deviceContext->Dispatch( groupCount.x, groupCount.y, groupCount.z );
}

void DX11RendererCore::disableShaderInputs()
{
    if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::disableShaderInputs - renderer not initialized." );

    m_deviceContext->IASetVertexBuffers( 0, (unsigned int)m_nullVertexBuffers.size(), m_nullVertexBuffers.data(), m_nullVertexBuffersStrideOffset.data(), m_nullVertexBuffersStrideOffset.data() );
    m_deviceContext->IASetIndexBuffer( nullptr, DXGI_FORMAT_UNKNOWN, 0 );
    m_deviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_UNDEFINED );

    m_deviceContext->PSSetShaderResources( 0, (unsigned int)m_nullResources.size(), m_nullResources.data() );
    m_deviceContext->PSSetSamplers( 0, (unsigned int)m_nullSamplers.size(), m_nullSamplers.data() );
}

void DX11RendererCore::createNullShaderInputs()
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