#include "BlockMesh.h"

#include <assert.h>
#include <fstream>
#include <vector>

#include <d3d11.h>

#include "MyOBJFileParser.h"
#include "MyDAEFileParser.h"
#include "BlockMeshFileInfoParser.h"

#include "StringUtil.h"
#include "Direct3DUtil.h"
#include "MathUtil.h"

#include "TextFile.h"

#include "BVHTree.h"
#include "BVHTreeBuffer.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

std::shared_ptr<BlockMesh> BlockMesh::createFromFile( const BlockMeshFileInfo& fileInfo )
{
	return createFromFile( fileInfo.getPath( ), fileInfo.getFormat( ), fileInfo.getIndexInFile(), fileInfo.getInvertZCoordinate( ), fileInfo.getInvertVertexWindingOrder( ), fileInfo.getFlipUVs( ) );
}

std::shared_ptr<BlockMesh> BlockMesh::createFromFile( const std::string& path, const BlockMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::shared_ptr< std::vector<char> > fileData = TextFile::load( path );

    std::shared_ptr<BlockMesh> mesh = createFromMemory( fileData->cbegin( ), fileData->cend( ), format, indexInFile, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	// Save path in the loaded mesh.
	mesh->getFileInfo().setPath( path );
	mesh->getFileInfo().setIndexInFile( indexInFile );
	mesh->getFileInfo().setFormat( format );
	mesh->getFileInfo().setInvertZCoordinate( invertZCoordinate );
	mesh->getFileInfo().setInvertVertexWindingOrder( invertVertexWindingOrder );
	mesh->getFileInfo().setFlipUVs( flipUVs );

	return mesh;
}

std::vector< std::shared_ptr<BlockMesh> > BlockMesh::createFromFile( const std::string& path, const BlockMeshFileInfo::Format format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::shared_ptr< std::vector<char> > fileData = TextFile::load( path );

    std::vector< std::shared_ptr<BlockMesh> > meshes = createFromMemory( fileData->cbegin( ), fileData->cend( ), format, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	// Save path in the loaded meshes.
	int indexInFile = 0;
	for ( std::shared_ptr<BlockMesh> mesh : meshes ) 
	{
		mesh->getFileInfo().setPath( path );
		mesh->getFileInfo().setIndexInFile( indexInFile++ );
		mesh->getFileInfo().setFormat( format );
		mesh->getFileInfo().setInvertZCoordinate( invertZCoordinate );
		mesh->getFileInfo().setInvertVertexWindingOrder( invertVertexWindingOrder );
		mesh->getFileInfo().setFlipUVs( flipUVs );
	}

	return meshes;
}

std::shared_ptr<BlockMesh> BlockMesh::createFromMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const BlockMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( indexInFile < 0 )
		throw std::exception( "BlockMesh::createFromMemory - 'index in file' parameter cannot be negative." );

    std::vector< std::shared_ptr<BlockMesh> > meshes = createFromMemory( dataIt, dataEndIt, format, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	if ( indexInFile < (int)meshes.size( ) )
		return meshes.at( indexInFile );
	else
		throw std::exception( "BlockMesh::createFromFile - no mesh at given index in file." );
}

std::vector< std::shared_ptr<BlockMesh> > BlockMesh::createFromMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const BlockMeshFileInfo::Format format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( BlockMeshFileInfo::Format::OBJ == format ) 
    {
        return MyOBJFileParser::parseBlockMeshFile( dataIt, dataEndIt, invertZCoordinate, invertVertexWindingOrder, flipUVs );
	} 
    else if ( BlockMeshFileInfo::Format::DAE == format ) 
    {
        return MyDAEFileParser::parseBlockMeshFile( dataIt, dataEndIt, invertZCoordinate, invertVertexWindingOrder, flipUVs );
	}

	throw std::exception( "BlockMesh::createFromMemory() - incorrect 'format' argument." );
}

BlockMesh::BlockMesh() :
boundingBoxMin( float3::ZERO ),
boundingBoxMax( float3::ZERO )
{}

BlockMesh::~BlockMesh()
{}

Asset::Type BlockMesh::getType( ) const
{
	return Asset::Type::BlockMesh;
}

std::vector< std::shared_ptr<const Asset> > BlockMesh::getSubAssets( ) const
{
	return std::vector< std::shared_ptr<const Asset> >( );
}

std::vector< std::shared_ptr<Asset> > BlockMesh::getSubAssets()
{
	return std::vector< std::shared_ptr<Asset> >( );
}

void BlockMesh::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	throw std::exception( "BlockMesh::swapSubAsset - there are no sub-assets to be swapped." );
}

void BlockMesh::setFileInfo( const BlockMeshFileInfo& fileInfo )
{
	this->fileInfo = fileInfo;
}

const BlockMeshFileInfo& BlockMesh::getFileInfo() const
{
	return fileInfo;
}

BlockMeshFileInfo& BlockMesh::getFileInfo( )
{
	return fileInfo;
}

void BlockMesh::loadCpuToGpu( ID3D11Device& device, bool reload )
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::loadCpuToGpu - Mesh not loaded in CPU memory." );

    if ( reload )
        throw std::exception( "BlockMesh::loadCpuToGpu - reload not yet implemented." );

	if ( vertices.size() > 0 && !vertexBuffer ) {
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth           = sizeof(float3)* (unsigned int)vertices.size();
		vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		vertexBufferDesc.CPUAccessFlags      = 0;
		vertexBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexDataPtr;
		vertexDataPtr.pSysMem          = vertices.data();
		vertexDataPtr.SysMemPitch      = 0;
		vertexDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexBufferDesc, &vertexDataPtr, vertexBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh vertices failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)vertices.size() * 3;

        result = device.CreateShaderResourceView( vertexBuffer.Get(), &resourceDesc, vertexBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating vertex buffer shader resource on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::vertexBuffer" );
		Direct3DUtil::setResourceName( *vertexBuffer.Get(), resourceName );
#endif
	}

	if ( normals.size() > 0 && !normalBuffer ) {
		D3D11_BUFFER_DESC normalBufferDesc;
		normalBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		normalBufferDesc.ByteWidth           = sizeof(float3) * (unsigned int)normals.size();
		normalBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		normalBufferDesc.CPUAccessFlags      = 0;
		normalBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		normalBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA normalDataPtr;
		normalDataPtr.pSysMem = normals.data();
		normalDataPtr.SysMemPitch = 0;
		normalDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &normalBufferDesc, &normalDataPtr, normalBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh normals failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)normals.size() * 3;

        result = device.CreateShaderResourceView( normalBuffer.Get(), &resourceDesc, normalBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating normal buffer shader resource on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::normalBuffer" );
		Direct3DUtil::setResourceName( *normalBuffer.Get(), resourceName );
#endif
	}

    if ( tangents.size() > 0 && !tangentBuffer ) {
		D3D11_BUFFER_DESC tangentBufferDesc;
		tangentBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		tangentBufferDesc.ByteWidth           = sizeof(float3) * (unsigned int)tangents.size();
		tangentBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		tangentBufferDesc.CPUAccessFlags      = 0;
		tangentBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		tangentBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA tangentDataPtr;
		tangentDataPtr.pSysMem = tangents.data();
		tangentDataPtr.SysMemPitch = 0;
		tangentDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &tangentBufferDesc, &tangentDataPtr, tangentBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh tangents failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)tangents.size() * 3;

        result = device.CreateShaderResourceView( tangentBuffer.Get(), &resourceDesc, tangentBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating tangent buffer shader resource on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::tangentBuffer" );
		Direct3DUtil::setResourceName( *tangentBuffer.Get(), resourceName );
#endif
	}

	std::list< std::vector<float2> >::iterator texcoordsIt, texcoordsEnd = texcoords.end();
    int texcoordsIndex = -1;

	for ( texcoordsIt = texcoords.begin(); texcoordsIt != texcoordsEnd; ++texcoordsIt ) {
        ++texcoordsIndex;

        if ( texcoordsIndex < (int)texcoordBuffers.size() )
            continue; // Skip texcoords which are already loaded.

		if ( texcoordsIt->empty() ) throw std::exception( "BlockMesh::loadCpuToGpu - One of mesh's texcoord sets is empty" );

		D3D11_BUFFER_DESC texcoordBufferDesc;
		texcoordBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		texcoordBufferDesc.ByteWidth           = sizeof(float2) * (unsigned int)texcoordsIt->size();
		texcoordBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		texcoordBufferDesc.CPUAccessFlags      = 0;
		texcoordBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		texcoordBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA texcoordDataPtr;
		texcoordDataPtr.pSysMem = texcoordsIt->data();
		texcoordDataPtr.SysMemPitch = 0;
		texcoordDataPtr.SysMemSlicePitch = 0;

		ComPtr<ID3D11Buffer> buffer;

		HRESULT result = device.CreateBuffer( &texcoordBufferDesc, &texcoordDataPtr, buffer.GetAddressOf() );
		if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh texcoords failed" );

        texcoordBuffers.push_back( buffer );

        ComPtr<ID3D11ShaderResourceView> bufferResource;

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)texcoordsIt->size() * 2;

        result = device.CreateShaderResourceView( buffer.Get(), &resourceDesc, bufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating texcoord buffer shader resource on GPU failed." );

        texcoordBufferResources.push_back( bufferResource );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::texcoordBuffer[" ) + std::to_string( texcoordBuffers.size() - 1 ) + std::string( "]" );
		Direct3DUtil::setResourceName( *buffer.Get(), resourceName );
#endif
	}

    if ( triangles.empty() ) throw std::exception( "BlockMesh::loadToGpu - Mesh has no triangles" );

    if ( !triangleBuffer ) {
        D3D11_BUFFER_DESC triangleBufferDesc;
        triangleBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
        triangleBufferDesc.ByteWidth           = sizeof(uint3) * (unsigned int)triangles.size();
        triangleBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
        triangleBufferDesc.CPUAccessFlags      = 0;
        triangleBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        triangleBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA triangleDataPtr;
        triangleDataPtr.pSysMem = triangles.data();
        triangleDataPtr.SysMemPitch = 0;
        triangleDataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &triangleBufferDesc, &triangleDataPtr, triangleBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadToGpu - Buffer creation for mesh triangles failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)triangles.size() * 3;

        result = device.CreateShaderResourceView( triangleBuffer.Get(), &resourceDesc, triangleBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating triangle buffer shader resource on GPU failed." );

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
    tangents.clear();
	tangents.shrink_to_fit();
	texcoords.clear(); // Calls destructor on each texcoord set.
	triangles.clear();
	triangles.shrink_to_fit();
}

void BlockMesh::unloadFromGpu()
{
	vertexBuffer.Reset();
    vertexBufferResource.Reset();
	normalBuffer.Reset();
    normalBufferResource.Reset();
    tangentBuffer.Reset();
    tangentBufferResource.Reset();

	while ( !texcoordBuffers.empty() ) {
		texcoordBuffers.front( ).Reset();
		texcoordBuffers.pop_front();
	}

    while ( !texcoordBufferResources.empty() ) {
        texcoordBufferResources.front().Reset();
        texcoordBufferResources.pop_front();
    }

	triangleBuffer.Reset();
    triangleBufferResource.Reset();
}

bool BlockMesh::isInCpuMemory() const
{
	return !vertices.empty() && !triangles.empty();
}

bool BlockMesh::isInGpuMemory() const
{
	return vertexBuffer && vertexBufferResource && triangleBuffer && triangleBufferResource;
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

const std::vector<float3>& BlockMesh::getTangents() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTangents - Mesh not loaded in CPU memory." );

	return tangents;
}

std::vector<float3>& BlockMesh::getTangents()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTangents - Mesh not loaded in CPU memory." );

	return tangents;
}

int BlockMesh::getTexcoordsCount() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoordsCount - Mesh not loaded in CPU memory." );

	return (int)texcoords.size();
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

ID3D11Buffer* BlockMesh::getTangentBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTangentBuffer - Mesh not loaded in GPU memory." );

	return tangentBuffer.Get();
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

ID3D11ShaderResourceView* BlockMesh::getVertexBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getVertexBufferResource - Mesh not loaded in GPU memory." );

    return vertexBufferResource.Get();
}

ID3D11ShaderResourceView* BlockMesh::getNormalBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getNormalBufferResource - Mesh not loaded in GPU memory." );

    return normalBufferResource.Get();
}

ID3D11ShaderResourceView* BlockMesh::getTangentBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTangentBufferResource - Mesh not loaded in GPU memory." );

    return tangentBufferResource.Get();
}

std::list< ID3D11ShaderResourceView* > BlockMesh::getTexcoordBufferResources() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTexcoordBufferResources - Mesh not loaded in GPU memory." );

    std::list< ID3D11ShaderResourceView* > tmpTexcoordBufferResources;

    for ( auto it = texcoordBufferResources.begin(); it != texcoordBufferResources.end(); ++it ) {
        tmpTexcoordBufferResources.push_back( it->Get() );
    }

    return tmpTexcoordBufferResources;
}

ID3D11ShaderResourceView* BlockMesh::getTriangleBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTriangleBufferResource - Mesh not loaded in GPU memory." );

    return triangleBufferResource.Get();
}

void BlockMesh::recalculateBoundingBox()
{
    std::tie( boundingBoxMin, boundingBoxMax ) = MathUtil::calculateBoundingBox( vertices );
}

std::tuple<float3, float3> BlockMesh::getBoundingBox() const
{
    return std::make_tuple(boundingBoxMin, boundingBoxMax);
}

void BlockMesh::buildBvhTree()
{
    bvhTree       = std::make_shared< BVHTree >( *this );
    bvhTreeBuffer = std::make_shared< BVHTreeBuffer >( *bvhTree );
}

void BlockMesh::loadBvhTreeToGpu( ID3D11Device& device )
{
    if ( !bvhTreeBufferNodesGpu ) {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( BVHTreeBuffer::Node ) * (unsigned int)bvhTreeBuffer->getNodes().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = bvhTreeBuffer->getNodes().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, bvhTreeBufferNodesGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH nodes failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32G32_UINT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)bvhTreeBuffer->getNodes().size();

        result = device.CreateShaderResourceView( bvhTreeBufferNodesGpu.Get(), &resourceDesc, bvhTreeBufferNodesGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH nodes shader resource view on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhNodes" );
        Direct3DUtil::setResourceName( *bvhTreeBufferNodesGpu.Get(), resourceName );
#endif
	}

    if ( !bvhTreeBufferNodesExtentsGpu ) {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( BVHTreeBuffer::NodeExtents ) * (unsigned int)bvhTreeBuffer->getNodesExtents().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = bvhTreeBuffer->getNodesExtents().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, bvhTreeBufferNodesExtentsGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH nodes extents failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32G32B32_FLOAT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)bvhTreeBuffer->getNodesExtents().size() * 2; // Multiplied by 2, because SRV accesses 
                                                                                                      // each float3 separately instead of pair <float3, float3> (min, max).

        result = device.CreateShaderResourceView( bvhTreeBufferNodesExtentsGpu.Get(), &resourceDesc, bvhTreeBufferNodesExtentsGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH nodes extents shader resource view on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhNodesExtents" );
        Direct3DUtil::setResourceName( *bvhTreeBufferNodesExtentsGpu.Get(), resourceName );
#endif
	}

    if ( !bvhTreeBufferTrianglesGpu ) {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( unsigned int ) * (unsigned int)bvhTreeBuffer->getTriangles().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = bvhTreeBuffer->getTriangles().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, bvhTreeBufferTrianglesGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH triangles failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32_UINT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)bvhTreeBuffer->getTriangles().size();

        result = device.CreateShaderResourceView( bvhTreeBufferTrianglesGpu.Get(), &resourceDesc, bvhTreeBufferTrianglesGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH triangles shader resource view on GPU failed." );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhTriangles" );
        Direct3DUtil::setResourceName( *bvhTreeBufferTrianglesGpu.Get(), resourceName );
#endif
	}
}

std::shared_ptr< const BVHTree > BlockMesh::getBvhTree() const
{
    return bvhTree;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferNodesShaderResourceView() const
{
    return bvhTreeBufferNodesGpuSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferNodesExtentsShaderResourceView() const
{
    return bvhTreeBufferNodesExtentsGpuSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferTrianglesShaderResourceView() const
{
    return bvhTreeBufferTrianglesGpuSRV;
}