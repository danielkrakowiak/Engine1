#include "RectangleMesh.h"

#include <d3d11.h>

#include "Direct3DUtil.h"

using namespace Engine1;

RectangleMesh::RectangleMesh()
{
	{ // Create vertices.
		vertices.push_back( float3( 0.0f, 1.0f, 0.0f ) ); //clockwise order
		vertices.push_back( float3( 1.0f, 1.0f, 0.0f ) );
		vertices.push_back( float3( 1.0f, 0.0f, 0.0f ) );
		vertices.push_back( float3( 0.0f, 0.0f, 0.0f ) );
	}

	{ // Create normals.
	normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
	normals.push_back( float3( 0.0f, 0.0f, 1.0f ) );
}

	{ // Create texcoords.
		texcoords.push_back( float2( 0.0f, 0.0f ) );
		texcoords.push_back( float2( 1.0f, 0.0f ) );
		texcoords.push_back( float2( 1.0f, 1.0f ) );
		texcoords.push_back( float2( 0.0f, 1.0f ) );
	}

	{ // Create triangles.
		triangles.push_back( uint3( 0, 1, 2 ) ); //clockwise order
		triangles.push_back( uint3( 0, 2, 3 ) );
	}
}


RectangleMesh::~RectangleMesh()
{
	int g = 5;
}

void RectangleMesh::loadCpuToGpu( ID3D11Device& device )
{
	if ( vertices.size() > 0 ) {
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(float3)* vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexDataPtr;
		vertexDataPtr.pSysMem = vertices.data();
		vertexDataPtr.SysMemPitch = 0;
		vertexDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexBufferDesc, &vertexDataPtr, vertexBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh vertices failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::vertexBuffer" );
		Direct3DUtil::setResourceName( *vertexBuffer.Get(), resourceName );
#endif
	}

	if ( normals.size() > 0 ) {
		D3D11_BUFFER_DESC normalBufferDesc;
		normalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		normalBufferDesc.ByteWidth = sizeof(float3)* normals.size();
		normalBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		normalBufferDesc.CPUAccessFlags = 0;
		normalBufferDesc.MiscFlags = 0;
		normalBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA normalDataPtr;
		normalDataPtr.pSysMem = normals.data();
		normalDataPtr.SysMemPitch = 0;
		normalDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &normalBufferDesc, &normalDataPtr, normalBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh normals failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::normalBuffer" );
		Direct3DUtil::setResourceName( *normalBuffer.Get(), resourceName );
#endif
	}

	if ( texcoords.size() > 0 ) {

		D3D11_BUFFER_DESC texcoordBufferDesc;
		texcoordBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		texcoordBufferDesc.ByteWidth = sizeof(float2)* texcoords.size();
		texcoordBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		texcoordBufferDesc.CPUAccessFlags = 0;
		texcoordBufferDesc.MiscFlags = 0;
		texcoordBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA texcoordDataPtr;
		texcoordDataPtr.pSysMem = texcoords.data();
		texcoordDataPtr.SysMemPitch = 0;
		texcoordDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &texcoordBufferDesc, &texcoordDataPtr, texcoordBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh texcoords failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::texcoordsBuffer" );
		Direct3DUtil::setResourceName( *texcoordBuffer.Get(), resourceName );
#endif
	}

	{
		if ( triangles.empty() ) throw std::exception( "RectangleMesh::loadToGpu - Mesh has no triangles." );

		D3D11_BUFFER_DESC triangleBufferDesc;
		triangleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		triangleBufferDesc.ByteWidth = sizeof(uint3)* triangles.size();
		triangleBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		triangleBufferDesc.CPUAccessFlags = 0;
		triangleBufferDesc.MiscFlags = 0;
		triangleBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA triangleDataPtr;
		triangleDataPtr.pSysMem = triangles.data();
		triangleDataPtr.SysMemPitch = 0;
		triangleDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &triangleBufferDesc, &triangleDataPtr, triangleBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "RectangleMesh::loadToGpu - Buffer creation for mesh triangles failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "RectangleMesh::triangleBuffer" );
		Direct3DUtil::setResourceName( *triangleBuffer.Get(), resourceName );
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
	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	triangles.clear();
	triangles.shrink_to_fit();
}

void RectangleMesh::unloadFromGpu()
{
	vertexBuffer.Reset();
	normalBuffer.Reset();
	texcoordBuffer.Reset();
	triangleBuffer.Reset();
}

bool RectangleMesh::isInCpuMemory() const
{
	return !vertices.empty() && !triangles.empty();
}

bool RectangleMesh::isInGpuMemory() const
{
	return vertexBuffer && triangleBuffer;
}

const std::vector<float3>& RectangleMesh::getVertices() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getVertices - Mesh not loaded in CPU memory." );

	return vertices;
}

const std::vector<float3>& RectangleMesh::getNormals() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getNormals - Mesh not loaded in CPU memory." );

	return normals;
}

const std::vector<float2>& RectangleMesh::getTexcoords() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getTexcoords - Mesh not loaded in CPU memory." );

	return texcoords;
}

const std::vector<uint3>& RectangleMesh::getTriangles() const
{
	if ( !isInCpuMemory() ) throw std::exception( "RectangleMesh::getTriangles - Mesh not loaded in CPU memory." );

	return triangles;
}

ID3D11Buffer* RectangleMesh::getVertexBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getVertexBuffer - Mesh not loaded in GPU memory." );

	return vertexBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getNormalBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getNormalBuffer - Mesh not loaded in GPU memory." );

	return normalBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getTexcoordBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getTexcoordBuffer - Mesh not loaded in GPU memory." );

	return texcoordBuffer.Get();
}

ID3D11Buffer* RectangleMesh::getTriangleBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "RectangleMesh::getTriangleBuffer - Mesh not loaded in GPU memory." );

	return triangleBuffer.Get();
}
