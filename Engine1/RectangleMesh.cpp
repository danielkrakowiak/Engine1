#include "RectangleMesh.h"

#include <d3d11_3.h>

#include "DX11Util.h"

using namespace Engine1;

RectangleMesh::RectangleMesh()
{
	{ // Create vertices.
		m_vertices.push_back( float3( 0.0f, 1.0f, 0.0f ) ); //clockwise order
		m_vertices.push_back( float3( 1.0f, 1.0f, 0.0f ) );
		m_vertices.push_back( float3( 1.0f, 0.0f, 0.0f ) );
		m_vertices.push_back( float3( 0.0f, 0.0f, 0.0f ) );
	}

	{ // Create normals.
	m_normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	m_normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	m_normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	m_normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
}

	{ // Create texcoords.
		m_texcoords.push_back( float2( 0.0f, 0.0f ) );
		m_texcoords.push_back( float2( 1.0f, 0.0f ) );
		m_texcoords.push_back( float2( 1.0f, 1.0f ) );
		m_texcoords.push_back( float2( 0.0f, 1.0f ) );
	}

	{ // Create triangles.
		m_triangles.push_back( uint3( 0, 1, 2 ) ); //clockwise order
		m_triangles.push_back( uint3( 0, 2, 3 ) );
	}
}


RectangleMesh::~RectangleMesh()
{
}

void RectangleMesh::loadCpuToGpu( ID3D11Device3& device, bool reload )
{
    if ( reload )
        throw std::exception( "RectangleMesh::loadCpuToGpu - reload not yet implemented." );

	if ( m_vertices.size() > 0 && !m_vertexBuffer ) {
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(float3) * (unsigned int)m_vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexDataPtr;
		vertexDataPtr.pSysMem = m_vertices.data();
		vertexDataPtr.SysMemPitch = 0;
		vertexDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexBufferDesc, &vertexDataPtr, m_vertexBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh vertices failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::vertexBuffer" );
		DX11Util::setResourceName( *m_vertexBuffer.Get(), resourceName );
#endif
	}

	if ( m_normals.size() > 0 && !m_normalBuffer ) {
		D3D11_BUFFER_DESC normalBufferDesc;
		normalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		normalBufferDesc.ByteWidth = sizeof(float3) * (unsigned int)m_normals.size();
		normalBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		normalBufferDesc.CPUAccessFlags = 0;
		normalBufferDesc.MiscFlags = 0;
		normalBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA normalDataPtr;
		normalDataPtr.pSysMem = m_normals.data();
		normalDataPtr.SysMemPitch = 0;
		normalDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &normalBufferDesc, &normalDataPtr, m_normalBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh normals failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::normalBuffer" );
		DX11Util::setResourceName( *m_normalBuffer.Get(), resourceName );
#endif
	}

	if ( m_texcoords.size() > 0 && !m_texcoordBuffer ) {

		D3D11_BUFFER_DESC texcoordBufferDesc;
		texcoordBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		texcoordBufferDesc.ByteWidth = sizeof(float2) * (unsigned int)m_texcoords.size();
		texcoordBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		texcoordBufferDesc.CPUAccessFlags = 0;
		texcoordBufferDesc.MiscFlags = 0;
		texcoordBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA texcoordDataPtr;
		texcoordDataPtr.pSysMem = m_texcoords.data();
		texcoordDataPtr.SysMemPitch = 0;
		texcoordDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &texcoordBufferDesc, &texcoordDataPtr, m_texcoordBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh texcoords failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::texcoordsBuffer" );
		DX11Util::setResourceName( *m_texcoordBuffer.Get(), resourceName );
#endif
	}

	if ( m_triangles.empty() ) throw std::exception( "RectangleMesh::loadToGpu - Mesh has no triangles." );

    if ( !m_triangleBuffer ) {
		D3D11_BUFFER_DESC triangleBufferDesc;
		triangleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		triangleBufferDesc.ByteWidth = sizeof(uint3) * (unsigned int)m_triangles.size();
		triangleBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		triangleBufferDesc.CPUAccessFlags = 0;
		triangleBufferDesc.MiscFlags = 0;
		triangleBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA triangleDataPtr;
		triangleDataPtr.pSysMem = m_triangles.data();
		triangleDataPtr.SysMemPitch = 0;
		triangleDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &triangleBufferDesc, &triangleDataPtr, m_triangleBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh triangles failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::triangleBuffer" );
		DX11Util::setResourceName( *m_triangleBuffer.Get(), resourceName );
#endif
	}
}

void RectangleMesh::loadGpuToCpu()
{
	throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.
}

void RectangleMesh::unloadFromCpu()
{
	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_normals.clear();
	m_normals.shrink_to_fit();
	m_texcoords.clear();
	m_texcoords.shrink_to_fit();
	m_triangles.clear();
	m_triangles.shrink_to_fit();
}

void RectangleMesh::unloadFromGpu()
{
	m_vertexBuffer.Reset();
	m_normalBuffer.Reset();
	m_texcoordBuffer.Reset();
	m_triangleBuffer.Reset();
}

bool RectangleMesh::isInCpuMemory() const
{
	return !m_vertices.empty() && !m_triangles.empty();
}

bool RectangleMesh::isInGpuMemory() const
{
	return m_vertexBuffer && m_triangleBuffer;
}

const std::vector<float3>& RectangleMesh::getVertices() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getVertices - Mesh not loaded in CPU memory." );

	return m_vertices;
}

const std::vector<float3>& RectangleMesh::getNormals() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getNormals - Mesh not loaded in CPU memory." );

	return m_normals;
}

const std::vector<float2>& RectangleMesh::getTexcoords() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getTexcoords - Mesh not loaded in CPU memory." );

	return m_texcoords;
}

const std::vector<uint3>& RectangleMesh::getTriangles() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getTriangles - Mesh not loaded in CPU memory." );

	return m_triangles;
}

ID3D11Buffer* RectangleMesh::getVertexBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getVertexBuffer - Mesh not loaded in GPU memory." );

	return m_vertexBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getNormalBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getNormalBuffer - Mesh not loaded in GPU memory." );

	return m_normalBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getTexcoordBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getTexcoordBuffer - Mesh not loaded in GPU memory." );

	return m_texcoordBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getTriangleBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getTriangleBuffer - Mesh not loaded in GPU memory." );

	return m_triangleBuffer.Get();
}
