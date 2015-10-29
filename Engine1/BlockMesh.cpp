#include "BlockMesh.h"

#include <assert.h>
#include <fstream>
#include <vector>

#include <d3d11.h>

#include "MyOBJFileParser.h"
#include "MyDAEFileParser.h"

#include "StringUtil.h"
#include "Direct3DUtil.h"

#include "File.h"

using Microsoft::WRL::ComPtr;

std::vector< std::shared_ptr<BlockMesh> > BlockMesh::createFromFile( const std::string& path, const FileFormat format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::shared_ptr< std::vector<char> > fileData = File::loadText( path );

	return createFromMemory( *fileData, format, invertZCoordinate, invertVertexWindingOrder, flipUVs );
}

std::vector< std::shared_ptr<BlockMesh> > BlockMesh::createFromMemory( std::vector<char>& fileData, const FileFormat format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( FileFormat::OBJ == format ) {

		std::vector< std::shared_ptr<BlockMesh> > meshes;
		meshes.push_back( MyOBJFileParser::parseBlockMeshFile( fileData, invertZCoordinate, invertVertexWindingOrder, flipUVs ) );

		return meshes;

	} else if ( FileFormat::DAE == format ) {
		return MyDAEFileParser::parseBlockMeshFile( fileData, invertZCoordinate, invertVertexWindingOrder, flipUVs );
	}

	throw std::exception( "BlockMesh::createFromMemory() - incorrect 'format' argument." );
}

BlockMesh::BlockMesh()
{}

BlockMesh::~BlockMesh()
{}

void BlockMesh::loadCpuToGpu( ID3D11Device& device )
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::loadCpuToGpu - Mesh not loaded in CPU memory." );

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
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh vertices failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::vertexBuffer" );
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
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh normals failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::normalBuffer" );
		Direct3DUtil::setResourceName( *normalBuffer.Get(), resourceName );
#endif
	}

	std::list< std::vector<float2> >::iterator texcoordsIt, texcoordsEnd = texcoords.end();

	for ( texcoordsIt = texcoords.begin(); texcoordsIt != texcoordsEnd; ++texcoordsIt ) {
		if ( texcoordsIt->empty() ) throw std::exception( "BlockMesh::loadCpuToGpu - One of mesh's texcoord sets is empty" );

		D3D11_BUFFER_DESC texcoordBufferDesc;
		texcoordBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		texcoordBufferDesc.ByteWidth = sizeof(float2)* texcoordsIt->size();
		texcoordBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		texcoordBufferDesc.CPUAccessFlags = 0;
		texcoordBufferDesc.MiscFlags = 0;
		texcoordBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA texcoordDataPtr;
		texcoordDataPtr.pSysMem = texcoordsIt->data();
		texcoordDataPtr.SysMemPitch = 0;
		texcoordDataPtr.SysMemSlicePitch = 0;

		ComPtr<ID3D11Buffer> buffer;

		HRESULT result = device.CreateBuffer( &texcoordBufferDesc, &texcoordDataPtr, buffer.GetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh texcoords failed" );

		texcoordBuffers.push_back( buffer );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::texcoordBuffer[" ) + std::to_string( texcoordBuffers.size() - 1 ) + std::string( "]" );
		Direct3DUtil::setResourceName( *buffer.Get(), resourceName );
#endif
	}

	{
		if ( triangles.empty() ) throw std::exception( "BlockMesh::loadToGpu - Mesh has no triangles" );

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
		if ( result < 0 ) throw std::exception( "BlockMesh::loadToGpu - Buffer creation for mesh triangles failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::triangleBuffer" );
		Direct3DUtil::setResourceName( *triangleBuffer.Get(), resourceName );
#endif
	}
}

void BlockMesh::loadGpuToCpu()
{
	throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.
}

void BlockMesh::unloadFromCpu()
{
	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	texcoords.clear(); // Calls destructor on each texcoord set.
	triangles.clear();
	triangles.shrink_to_fit();
}

void BlockMesh::unloadFromGpu()
{
	vertexBuffer.Reset();
	normalBuffer.Reset();

	while ( !texcoordBuffers.empty() ) {
		texcoordBuffers.front( ).Reset();
		texcoordBuffers.pop_front();
	}

	triangleBuffer.Reset();
}

bool BlockMesh::isInCpuMemory() const
{
	return !vertices.empty() && !triangles.empty();
}

bool BlockMesh::isInGpuMemory() const
{
	return vertexBuffer && triangleBuffer;
}

const std::vector<float3>& BlockMesh::getVertices() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getVertices - Mesh not loaded in CPU memory." );

	return vertices;
}

std::vector<float3>& BlockMesh::getVertices()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getVertices - Mesh not loaded in CPU memory." );

	return vertices;
}

const std::vector<float3>& BlockMesh::getNormals() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getNormals - Mesh not loaded in CPU memory." );

	return normals;
}

std::vector<float3>& BlockMesh::getNormals()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getNormals - Mesh not loaded in CPU memory." );

	return normals;
}

int BlockMesh::getTexcoordsCount() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoordsCount - Mesh not loaded in CPU memory." );

	return texcoords.size();
}

const std::vector<float2>& BlockMesh::getTexcoords( int setIndex ) const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)texcoords.size() ) throw std::exception( "BlockMesh::getTexcoords: Trying to access texcoords at non-existing index." );

	std::list< std::vector<float2> >::const_iterator it = texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) it++;

	return *it;
}

std::vector<float2>& BlockMesh::getTexcoords( int setIndex )
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)texcoords.size() ) throw std::exception( "BlockMesh::getTexcoords: Trying to access texcoords at non-existing index." );

	std::list< std::vector<float2> >::iterator it = texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) it++;

	return *it;
}

const std::vector<uint3>& BlockMesh::getTriangles() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTriangles - Mesh not loaded in CPU memory." );

	return triangles;
}

std::vector<uint3>& BlockMesh::getTriangles()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTriangles - Mesh not loaded in CPU memory." );

	return triangles;
}

ID3D11Buffer* BlockMesh::getVertexBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getVertexBuffer - Mesh not loaded in GPU memory." );

	return vertexBuffer.Get();
}

ID3D11Buffer* BlockMesh::getNormalBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getNormalBuffer - Mesh not loaded in GPU memory." );

	return normalBuffer.Get();
}

std::list< ID3D11Buffer* > BlockMesh::getTexcoordBuffers() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTexcoordBuffers - Mesh not loaded in GPU memory." );

	std::list< ID3D11Buffer* > tmpTexcoordBuffers;

	for ( auto it = texcoordBuffers.begin(); it != texcoordBuffers.end(); ++it ) {
		tmpTexcoordBuffers.push_back( it->Get() );
	}

	return tmpTexcoordBuffers;
}

ID3D11Buffer* BlockMesh::getTriangleBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTriangleBuffer - Mesh not loaded in GPU memory." );

	return triangleBuffer.Get();
}