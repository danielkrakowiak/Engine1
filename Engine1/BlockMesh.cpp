#include "BlockMesh.h"

#include <assert.h>
#include <fstream>
#include <vector>

#include <d3d11.h>

#include "MeshFileParser.h"
#include "BlockMeshFileInfoParser.h"

#include "StringUtil.h"
#include "Direct3DUtil.h"
#include "MathUtil.h"

#include "TextFile.h"
#include "BinaryFile.h"

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
    std::shared_ptr< std::vector<char> > fileData;
    if ( BlockMeshFileInfo::Format::OBJ == format || BlockMeshFileInfo::Format::DAE == format ) 
	    fileData = TextFile::load( path );
    else if ( BlockMeshFileInfo::Format::FBX == format || BlockMeshFileInfo::Format::BLOCKMESH == format )
        fileData = BinaryFile::load( path );

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
    std::shared_ptr< std::vector<char> > fileData;
    if ( BlockMeshFileInfo::Format::OBJ == format || BlockMeshFileInfo::Format::DAE == format ) 
	    fileData = TextFile::load( path );
    else if ( BlockMeshFileInfo::Format::FBX == format )
        fileData = BinaryFile::load( path );

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
    return MeshFileParser::parseBlockMeshFile( format, dataIt, dataEndIt, invertZCoordinate, invertVertexWindingOrder, flipUVs );
}

BlockMesh::BlockMesh()
{}

BlockMesh::BlockMesh( const int vertexCount, const bool hasNormalsTangents, const int texcoordsSetCount, const int triangleCount )
{
    m_vertices.resize( vertexCount );
    
    if ( hasNormalsTangents ) {
        m_normals.resize( vertexCount );
        m_tangents.resize( vertexCount );
    }

    m_texcoords.resize( texcoordsSetCount );
    for ( auto& texcoords : m_texcoords ) {
        texcoords.resize( vertexCount );
    }

    m_triangles.resize( triangleCount );
}

BlockMesh::~BlockMesh()
{}

void BlockMesh::saveToFile( const std::string& path, const BlockMeshFileInfo::Format format )
{
    std::vector< char > data;

    MeshFileParser::writeBlockMeshFile( data, format, *this );

    BlockMeshFileInfo::FileType fileType = BlockMeshFileInfo::getFileTypeFromFormat( format );
    if ( fileType == BlockMeshFileInfo::FileType::Binary )
        BinaryFile::save( path, data );
    else if ( fileType == BlockMeshFileInfo::FileType::Textual )
        TextFile::save( path, data );
}

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
	this->m_fileInfo = fileInfo;
}

const BlockMeshFileInfo& BlockMesh::getFileInfo() const
{
	return m_fileInfo;
}

BlockMeshFileInfo& BlockMesh::getFileInfo( )
{
	return m_fileInfo;
}

void BlockMesh::loadCpuToGpu( ID3D11Device& device, bool reload )
{
	if ( !isInCpuMemory() ) 
        throw std::exception( "BlockMesh::loadCpuToGpu - Mesh not loaded in CPU memory." );

	if ( m_vertices.size() > 0 && ( !m_vertexBuffer || reload ) ) {
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth           = sizeof(float3)* (unsigned int)m_vertices.size();
		vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		vertexBufferDesc.CPUAccessFlags      = 0;
		vertexBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexDataPtr;
		vertexDataPtr.pSysMem          = m_vertices.data();
		vertexDataPtr.SysMemPitch      = 0;
		vertexDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexBufferDesc, &vertexDataPtr, m_vertexBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh vertices failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)m_vertices.size() * 3;

        result = device.CreateShaderResourceView( m_vertexBuffer.Get(), &resourceDesc, m_vertexBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - creating vertex buffer shader resource on GPU failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::vertexBuffer" );
		Direct3DUtil::setResourceName( *m_vertexBuffer.Get(), resourceName );
#endif
	}

	if ( m_normals.size() > 0 && ( !m_normalBuffer || reload ) ) {
		D3D11_BUFFER_DESC normalBufferDesc;
		normalBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		normalBufferDesc.ByteWidth           = sizeof(float3) * (unsigned int)m_normals.size();
		normalBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		normalBufferDesc.CPUAccessFlags      = 0;
		normalBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		normalBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA normalDataPtr;
		normalDataPtr.pSysMem = m_normals.data();
		normalDataPtr.SysMemPitch = 0;
		normalDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &normalBufferDesc, &normalDataPtr, m_normalBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh normals failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)m_normals.size() * 3;

        result = device.CreateShaderResourceView( m_normalBuffer.Get(), &resourceDesc, m_normalBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - creating normal buffer shader resource on GPU failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::normalBuffer" );
		Direct3DUtil::setResourceName( *m_normalBuffer.Get(), resourceName );
#endif
	}

    if ( m_tangents.size() > 0 && ( !m_tangentBuffer || reload ) ) {
		D3D11_BUFFER_DESC tangentBufferDesc;
		tangentBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
		tangentBufferDesc.ByteWidth           = sizeof(float3) * (unsigned int)m_tangents.size();
		tangentBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
		tangentBufferDesc.CPUAccessFlags      = 0;
		tangentBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		tangentBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA tangentDataPtr;
		tangentDataPtr.pSysMem = m_tangents.data();
		tangentDataPtr.SysMemPitch = 0;
		tangentDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &tangentBufferDesc, &tangentDataPtr, m_tangentBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh tangents failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)m_tangents.size() * 3;

        result = device.CreateShaderResourceView( m_tangentBuffer.Get(), &resourceDesc, m_tangentBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - creating tangent buffer shader resource on GPU failed." );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::tangentBuffer" );
		Direct3DUtil::setResourceName( *m_tangentBuffer.Get(), resourceName );
#endif
	}


    // If reloading - destroy old texcoord set buffers/views on GPU.
    if ( reload )
    {
        m_texcoordBuffers.clear();
        m_texcoordBufferResources.clear();
    }

	std::list< std::vector<float2> >::iterator texcoordsIt, texcoordsEnd = m_texcoords.end();
    int texcoordsIndex = -1;

	for ( texcoordsIt = m_texcoords.begin(); texcoordsIt != texcoordsEnd; ++texcoordsIt ) {
        ++texcoordsIndex;

        if ( texcoordsIndex < (int)m_texcoordBuffers.size() )
            continue; // Skip texcoords which are already loaded.

		if ( texcoordsIt->empty() ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - One of mesh's texcoord sets is empty" );

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
		if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - Buffer creation for mesh texcoords failed" );

        m_texcoordBuffers.push_back( buffer );

        ComPtr<ID3D11ShaderResourceView> bufferResource;

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)texcoordsIt->size() * 2;

        result = device.CreateShaderResourceView( buffer.Get(), &resourceDesc, bufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - creating texcoord buffer shader resource on GPU failed." );

        m_texcoordBufferResources.push_back( bufferResource );

#if defined(_DEBUG) 
		std::string resourceName = std::string( "BlockMesh::texcoordBuffer[" ) + std::to_string( m_texcoordBuffers.size() - 1 ) + std::string( "]" );
		Direct3DUtil::setResourceName( *buffer.Get(), resourceName );
#endif
	}

    if ( m_triangles.empty() ) 
        throw std::exception( "BlockMesh::loadToGpu - Mesh has no triangles" );

    if ( !m_triangleBuffer || reload ) {
        D3D11_BUFFER_DESC triangleBufferDesc;
        triangleBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
        triangleBufferDesc.ByteWidth           = sizeof(uint3) * (unsigned int)m_triangles.size();
        triangleBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
        triangleBufferDesc.CPUAccessFlags      = 0;
        triangleBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        triangleBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA triangleDataPtr;
        triangleDataPtr.pSysMem = m_triangles.data();
        triangleDataPtr.SysMemPitch = 0;
        triangleDataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &triangleBufferDesc, &triangleDataPtr, m_triangleBuffer.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadToGpu - Buffer creation for mesh triangles failed" );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
        resourceDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
        resourceDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;
        resourceDesc.BufferEx.FirstElement = 0;
        resourceDesc.BufferEx.NumElements  = (unsigned int)m_triangles.size() * 3;

        result = device.CreateShaderResourceView( m_triangleBuffer.Get(), &resourceDesc, m_triangleBufferResource.ReleaseAndGetAddressOf() );
        if ( result < 0 ) 
            throw std::exception( "BlockMesh::loadCpuToGpu - creating triangle buffer shader resource on GPU failed." );

#if defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::triangleBuffer" );
        Direct3DUtil::setResourceName( *m_triangleBuffer.Get(), resourceName );
#endif
	}

    if ( getBvhTree() )
        loadBvhTreeToGpu( device, reload );
}

void BlockMesh::loadGpuToCpu()
{
	throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.
}

void BlockMesh::unloadFromCpu()
{
	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_normals.clear();
	m_normals.shrink_to_fit();
    m_tangents.clear();
	m_tangents.shrink_to_fit();
	m_texcoords.clear(); // Calls destructor on each texcoord set.
	m_triangles.clear();
	m_triangles.shrink_to_fit();
}

void BlockMesh::unloadFromGpu()
{
	m_vertexBuffer.Reset();
    m_vertexBufferResource.Reset();
	m_normalBuffer.Reset();
    m_normalBufferResource.Reset();
    m_tangentBuffer.Reset();
    m_tangentBufferResource.Reset();

	while ( !m_texcoordBuffers.empty() ) {
		m_texcoordBuffers.front( ).Reset();
		m_texcoordBuffers.pop_front();
	}

    while ( !m_texcoordBufferResources.empty() ) {
        m_texcoordBufferResources.front().Reset();
        m_texcoordBufferResources.pop_front();
    }

	m_triangleBuffer.Reset();
    m_triangleBufferResource.Reset();
}

bool BlockMesh::isInCpuMemory() const
{
	return !m_vertices.empty() && !m_triangles.empty();
}

bool BlockMesh::isInGpuMemory() const
{
	return m_vertexBuffer && m_vertexBufferResource && m_triangleBuffer && m_triangleBufferResource;
}

const std::vector<float3>& BlockMesh::getVertices() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getVertices - Mesh not loaded in CPU memory." );

	return m_vertices;
}

std::vector<float3>& BlockMesh::getVertices()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getVertices - Mesh not loaded in CPU memory." );

	return m_vertices;
}

const std::vector<float3>& BlockMesh::getNormals() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getNormals - Mesh not loaded in CPU memory." );

	return m_normals;
}

std::vector<float3>& BlockMesh::getNormals()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getNormals - Mesh not loaded in CPU memory." );

	return m_normals;
}

const std::vector<float3>& BlockMesh::getTangents() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTangents - Mesh not loaded in CPU memory." );

	return m_tangents;
}

std::vector<float3>& BlockMesh::getTangents()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTangents - Mesh not loaded in CPU memory." );

	return m_tangents;
}

int BlockMesh::getTexcoordsCount() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoordsCount - Mesh not loaded in CPU memory." );

	return (int)m_texcoords.size();
}

const std::vector<float2>& BlockMesh::getTexcoords( int setIndex ) const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)m_texcoords.size() ) throw std::exception( "BlockMesh::getTexcoords: Trying to access texcoords at non-existing index." );

    //#TODO: use std::advance() ?
	std::list< std::vector<float2> >::const_iterator it = m_texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) it++;

	return *it;
}

std::vector<float2>& BlockMesh::getTexcoords( int setIndex )
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)m_texcoords.size() ) throw std::exception( "BlockMesh::getTexcoords: Trying to access texcoords at non-existing index." );

	std::list< std::vector<float2> >::iterator it = m_texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) it++;

	return *it;
}

const std::vector<uint3>& BlockMesh::getTriangles() const
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTriangles - Mesh not loaded in CPU memory." );

	return m_triangles;
}

std::vector<uint3>& BlockMesh::getTriangles()
{
	if ( !isInCpuMemory() ) throw std::exception( "BlockMesh::getTriangles - Mesh not loaded in CPU memory." );

	return m_triangles;
}

ID3D11Buffer* BlockMesh::getVertexBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getVertexBuffer - Mesh not loaded in GPU memory." );

	return m_vertexBuffer.Get();
}

ID3D11Buffer* BlockMesh::getNormalBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getNormalBuffer - Mesh not loaded in GPU memory." );

	return m_normalBuffer.Get();
}

ID3D11Buffer* BlockMesh::getTangentBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTangentBuffer - Mesh not loaded in GPU memory." );

	return m_tangentBuffer.Get();
}

std::list< ID3D11Buffer* > BlockMesh::getTexcoordBuffers() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTexcoordBuffers - Mesh not loaded in GPU memory." );

	std::list< ID3D11Buffer* > tmpTexcoordBuffers;

	for ( auto it = m_texcoordBuffers.begin(); it != m_texcoordBuffers.end(); ++it ) {
		tmpTexcoordBuffers.push_back( it->Get() );
	}

	return tmpTexcoordBuffers;
}

ID3D11Buffer* BlockMesh::getTriangleBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTriangleBuffer - Mesh not loaded in GPU memory." );

	return m_triangleBuffer.Get();
}

ID3D11ShaderResourceView* BlockMesh::getVertexBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getVertexBufferResource - Mesh not loaded in GPU memory." );

    return m_vertexBufferResource.Get();
}

ID3D11ShaderResourceView* BlockMesh::getNormalBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getNormalBufferResource - Mesh not loaded in GPU memory." );

    return m_normalBufferResource.Get();
}

ID3D11ShaderResourceView* BlockMesh::getTangentBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTangentBufferResource - Mesh not loaded in GPU memory." );

    return m_tangentBufferResource.Get();
}

std::list< ID3D11ShaderResourceView* > BlockMesh::getTexcoordBufferResources() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTexcoordBufferResources - Mesh not loaded in GPU memory." );

    std::list< ID3D11ShaderResourceView* > tmpTexcoordBufferResources;

    for ( auto it = m_texcoordBufferResources.begin(); it != m_texcoordBufferResources.end(); ++it ) {
        tmpTexcoordBufferResources.push_back( it->Get() );
    }

    return tmpTexcoordBufferResources;
}

ID3D11ShaderResourceView* BlockMesh::getTriangleBufferResource() const
{
    if ( !isInGpuMemory() ) throw std::exception( "BlockMesh::getTriangleBufferResource - Mesh not loaded in GPU memory." );

    return m_triangleBufferResource.Get();
}

void BlockMesh::recalculateBoundingBox()
{
    m_boundingBox = MathUtil::calculateBoundingBox( m_vertices );
}

BoundingBox BlockMesh::getBoundingBox() const
{
    return m_boundingBox;
}

void BlockMesh::buildBvhTree()
{
    std::shared_ptr< BVHTree > bvhTree = std::make_shared< BVHTree >( *this );
   
    m_bvhTree = std::make_shared< BVHTreeBuffer >( *bvhTree );

    reorganizeTrianglesToMatchBvhTree();

    m_bvhTree->clearTriangles();
}

void BlockMesh::loadBvhTreeToGpu( ID3D11Device& device, const bool reload )
{
    if ( !m_bvhTreeBufferNodesGpu || reload ) 
    {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( BVHTreeBuffer::Node ) * (unsigned int)m_bvhTree->getNodes().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = m_bvhTree->getNodes().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, m_bvhTreeBufferNodesGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH nodes failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32G32_UINT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)m_bvhTree->getNodes().size();

        result = device.CreateShaderResourceView( m_bvhTreeBufferNodesGpu.Get(), &resourceDesc, m_bvhTreeBufferNodesGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH nodes shader resource view on GPU failed." );

#if defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhNodes" );
        Direct3DUtil::setResourceName( *m_bvhTreeBufferNodesGpu.Get(), resourceName );
#endif
	}

    if ( !m_bvhTreeBufferNodesExtentsGpu || reload ) {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( BVHTreeBuffer::NodeExtents ) * (unsigned int)m_bvhTree->getNodesExtents().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = m_bvhTree->getNodesExtents().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, m_bvhTreeBufferNodesExtentsGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH nodes extents failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32G32B32_FLOAT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)m_bvhTree->getNodesExtents().size() * 2; // Multiplied by 2, because SRV accesses 
                                                                                                      // each float3 separately instead of pair <float3, float3> (min, max).

        result = device.CreateShaderResourceView( m_bvhTreeBufferNodesExtentsGpu.Get(), &resourceDesc, m_bvhTreeBufferNodesExtentsGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH nodes extents shader resource view on GPU failed." );

#if defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhNodesExtents" );
        Direct3DUtil::setResourceName( *m_bvhTreeBufferNodesExtentsGpu.Get(), resourceName );
#endif
	}

    if ( ( !m_bvhTreeBufferTrianglesGpu || reload ) && !m_bvhTree->getTriangles().empty() ) {
        D3D11_BUFFER_DESC desc;
        desc.Usage               = D3D11_USAGE_DEFAULT;
        desc.ByteWidth           = sizeof( unsigned int ) * (unsigned int)m_bvhTree->getTriangles().size();
        desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA dataPtr;
        dataPtr.pSysMem          = m_bvhTree->getTriangles().data();
        dataPtr.SysMemPitch      = 0;
        dataPtr.SysMemSlicePitch = 0;

        HRESULT result = device.CreateBuffer( &desc, &dataPtr, m_bvhTreeBufferTrianglesGpu.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadBvhTreeToGpu - Buffer creation for BVH triangles failed." );

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
        resourceDesc.Format              = DXGI_FORMAT_R32_UINT;
        resourceDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
        resourceDesc.Buffer.FirstElement = 0;
        resourceDesc.Buffer.NumElements  = (unsigned int)m_bvhTree->getTriangles().size();

        result = device.CreateShaderResourceView( m_bvhTreeBufferTrianglesGpu.Get(), &resourceDesc, m_bvhTreeBufferTrianglesGpuSRV.ReleaseAndGetAddressOf() );
        if ( result < 0 ) throw std::exception( "BlockMesh::loadCpuToGpu - creating BVH triangles shader resource view on GPU failed." );

#if defined(_DEBUG) 
        std::string resourceName = std::string( "BlockMesh::bvhTriangles" );
        Direct3DUtil::setResourceName( *m_bvhTreeBufferTrianglesGpu.Get(), resourceName );
#endif
	}
}

void BlockMesh::unloadBvhTreeFromGpu()
{
    m_bvhTreeBufferNodesGpu.Reset();
    m_bvhTreeBufferNodesExtentsGpu.Reset();
    m_bvhTreeBufferTrianglesGpu.Reset();
}

std::shared_ptr< const BVHTreeBuffer > BlockMesh::getBvhTree() const
{
    return m_bvhTree;
}

void BlockMesh::setBvhTree( std::shared_ptr< BVHTreeBuffer > bvhTree )
{
    m_bvhTree = bvhTree;
}

void BlockMesh::reorganizeTrianglesToMatchBvhTree()
{
    if ( !m_bvhTree )
        throw std::exception( "BlockMesh::reorganizeTrianglesToMatchBvhTree - BVH Tree Buffer needs to be built first." );

    const std::vector<uint3> trianglesCopy( m_triangles ); //#TOD: Optimization: Use std::move() here?

    const std::vector< unsigned int >& bvhTriangles = m_bvhTree->getTriangles();

    assert( !bvhTriangles.empty() );

    // Note: BVH tree may have a bit more triangles, because some of them are duplicated (single triangle contained in multiple nodes).
    const int bvhTriangleCount = (int)bvhTriangles.size();

    // Resize triangles vector.
    m_triangles.clear(); // NOTE: Clear to avoid copying old content during vector resize.
    m_triangles.reserve( bvhTriangleCount ); 
    m_triangles.resize( bvhTriangleCount );

    // Copy triangles in the same order as in BVH.
    for ( int i = 0; i < bvhTriangleCount; ++i )
        m_triangles[ i ] = trianglesCopy[ bvhTriangles[ i ] ];
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferNodesShaderResourceView() const
{
    return m_bvhTreeBufferNodesGpuSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferNodesExtentsShaderResourceView() const
{
    return m_bvhTreeBufferNodesExtentsGpuSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BlockMesh::getBvhTreeBufferTrianglesShaderResourceView() const
{
    return m_bvhTreeBufferTrianglesGpuSRV;
}