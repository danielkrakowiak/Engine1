#include "SkeletonMesh.h"

#include <fstream>
#include <assert.h>

#include "MyOBJFileParser.h"
#include "MyDAEFileParser.h"

#include "StringUtil.h"
#include "Direct3DUtil.h"

#include "TextFile.h"

#include <d3d11.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

std::shared_ptr<SkeletonMesh> SkeletonMesh::createFromFile( const SkeletonMeshFileInfo& fileInfo )
{
	return createFromFile( fileInfo.getPath( ), fileInfo.getFormat( ), fileInfo.getIndexInFile(), fileInfo.getInvertZCoordinate( ), fileInfo.getInvertVertexWindingOrder( ), fileInfo.getFlipUVs( ) );
}

std::shared_ptr<SkeletonMesh> SkeletonMesh::createFromFile( const std::string& path, const SkeletonMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::shared_ptr< std::vector<char> > fileData = TextFile::load( path );

	std::shared_ptr<SkeletonMesh> mesh = createFromMemory( *fileData, format, indexInFile, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	mesh->getFileInfo().setPath( path );
	mesh->getFileInfo().setIndexInFile( indexInFile );
	mesh->getFileInfo().setFormat( format );
	mesh->getFileInfo().setInvertZCoordinate( invertZCoordinate );
	mesh->getFileInfo().setInvertVertexWindingOrder( invertVertexWindingOrder );
	mesh->getFileInfo().setFlipUVs( flipUVs );

	return mesh;
}

std::vector< std::shared_ptr<SkeletonMesh> > SkeletonMesh::createFromFile( const std::string& path, const SkeletonMeshFileInfo::Format format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::shared_ptr< std::vector<char> > fileData = TextFile::load( path );

	std::vector< std::shared_ptr<SkeletonMesh> > meshes = createFromMemory( *fileData, format, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	int indexInFile = 0;
	for ( std::shared_ptr<SkeletonMesh> mesh : meshes ) {
		mesh->getFileInfo().setPath( path );
		mesh->getFileInfo().setIndexInFile( indexInFile++ );
		mesh->getFileInfo().setFormat( format );
		mesh->getFileInfo().setInvertZCoordinate( invertZCoordinate );
		mesh->getFileInfo().setInvertVertexWindingOrder( invertVertexWindingOrder );
		mesh->getFileInfo().setFlipUVs( flipUVs );
	}

	return meshes;
}

std::shared_ptr<SkeletonMesh> SkeletonMesh::createFromMemory( const std::vector<char>& fileData, const SkeletonMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( indexInFile < 0 )
		throw std::exception( "SkeletonMesh::createFromMemory - 'index in file' parameter cannot be negative." );

	std::vector< std::shared_ptr<SkeletonMesh> > meshes = createFromMemory( fileData, format, invertZCoordinate, invertVertexWindingOrder, flipUVs );

	if ( indexInFile < (int)meshes.size() )
		return meshes.at( indexInFile );
	else
		throw std::exception( "SkeletonMesh::createFromFile - no mesh at given index in file." );
}

std::vector< std::shared_ptr<SkeletonMesh> > SkeletonMesh::createFromMemory( const std::vector<char>& fileData, const SkeletonMeshFileInfo::Format format, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( SkeletonMeshFileInfo::Format::DAE == format ) {
		return MyDAEFileParser::parseSkeletonMeshFile( fileData, invertZCoordinate, invertVertexWindingOrder, flipUVs );
	}

	throw std::exception( "SkeletonMesh::createFromMemory() - incorrect 'format' argument." );
}

SkeletonMesh::SkeletonMesh() :
bonesPerVertexCount( BonesPerVertexCount::Type::ZERO )
{}

SkeletonMesh::~SkeletonMesh() 
{}

Asset::Type SkeletonMesh::getType() const
{
	return Asset::Type::SkeletonMesh;
}

std::vector< std::shared_ptr<const Asset> > SkeletonMesh::getSubAssets( ) const
{
	return std::vector< std::shared_ptr<const Asset> >();
}

std::vector< std::shared_ptr<Asset> > SkeletonMesh::getSubAssets()
{
	return std::vector< std::shared_ptr<Asset> >( );
}

void SkeletonMesh::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	throw std::exception( "SkeletonMesh::swapSubAsset - there are no sub-assets to be swapped." );
}

void SkeletonMesh::setFileInfo( const SkeletonMeshFileInfo& fileInfo )
{
	this->fileInfo = fileInfo;
}

const SkeletonMeshFileInfo& SkeletonMesh::getFileInfo( ) const
{
	return fileInfo;
}

SkeletonMeshFileInfo& SkeletonMesh::getFileInfo( )
{
	return fileInfo;
}

void SkeletonMesh::loadCpuToGpu( ID3D11Device& device )
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::loadCpuToGpu - Mesh not loaded in CPU memory." );

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
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh vertices failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::vertexBuffer" );
		Direct3DUtil::setResourceName( *vertexBuffer.Get(), resourceName );
#endif
	}

	if ( vertexBones.size() > 0 ) {
		D3D11_BUFFER_DESC vertexBonesBufferDesc;
		vertexBonesBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBonesBufferDesc.ByteWidth = sizeof(unsigned char)* vertexBones.size();
		vertexBonesBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBonesBufferDesc.CPUAccessFlags = 0;
		vertexBonesBufferDesc.MiscFlags = 0;
		vertexBonesBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexBonesDataPtr;
		vertexBonesDataPtr.pSysMem = vertexBones.data();
		vertexBonesDataPtr.SysMemPitch = 0;
		vertexBonesDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexBonesBufferDesc, &vertexBonesDataPtr, vertexBonesBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh vertex-bones failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::vertexBonesBuffer" );
		Direct3DUtil::setResourceName( *vertexBonesBuffer.Get(), resourceName );
#endif
	}

	if ( vertexWeights.size() > 0 ) {
		D3D11_BUFFER_DESC vertexWeightsBufferDesc;
		vertexWeightsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexWeightsBufferDesc.ByteWidth = sizeof(float)* vertexWeights.size();
		vertexWeightsBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexWeightsBufferDesc.CPUAccessFlags = 0;
		vertexWeightsBufferDesc.MiscFlags = 0;
		vertexWeightsBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexWeightsDataPtr;
		vertexWeightsDataPtr.pSysMem = vertexWeights.data();
		vertexWeightsDataPtr.SysMemPitch = 0;
		vertexWeightsDataPtr.SysMemSlicePitch = 0;

		HRESULT result = device.CreateBuffer( &vertexWeightsBufferDesc, &vertexWeightsDataPtr, vertexWeightsBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh vertex-weights failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::vertexWeightsBuffer" );
		Direct3DUtil::setResourceName( *vertexWeightsBuffer.Get(), resourceName );
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
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh normals failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::normalBuffer" );
		Direct3DUtil::setResourceName( *normalBuffer.Get(), resourceName );
#endif
	}

	std::list< std::vector<float2> >::iterator texcoordsIt, texcoordsEnd = texcoords.end();

	for ( texcoordsIt = texcoords.begin(); texcoordsIt != texcoordsEnd; ++texcoordsIt ) {
		if ( texcoordsIt->empty() ) throw std::exception( "SkeletonMesh::loadToGpu - One of mesh's texcoord sets is empty" );

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
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh texcoords failed" );

		texcoordBuffers.push_back( buffer );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::texcoordBuffer[" ) + std::to_string( texcoordBuffers.size() - 1 ) + std::string( "]" );
		Direct3DUtil::setResourceName( *buffer.Get(), resourceName );
#endif
	}

	{
		if ( triangles.empty() ) throw std::exception( "SkeletonMesh::loadToGpu - Mesh has no triangles" );

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
		if ( result < 0 ) throw std::exception( "SkeletonMesh::loadToGpu - Buffer creation for mesh triangles failed" );

#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMesh::triangleBuffer" );
		Direct3DUtil::setResourceName( *triangleBuffer.Get(), resourceName );
#endif
	}
}

void SkeletonMesh::loadGpuToCpu()
{
	throw std::exception( "SkeletonMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.
}

void SkeletonMesh::unloadFromCpu()
{
	vertices.clear();
	vertices.shrink_to_fit();
	vertexWeights.clear();
	vertexWeights.shrink_to_fit();
	vertexBones.clear();
	vertexBones.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	texcoords.clear(); // Calls destructor on each texcoord set.
	triangles.clear();
	triangles.shrink_to_fit();
}

void SkeletonMesh::unloadFromGpu()
{
	vertexBuffer.Reset();
	vertexWeightsBuffer.Reset();
	vertexBonesBuffer.Reset();
	normalBuffer.Reset();

	while ( !texcoordBuffers.empty( ) ) {
		texcoordBuffers.front( ).Reset( );
		texcoordBuffers.pop_front( );
	}

	triangleBuffer.Reset();
}

bool SkeletonMesh::isInCpuMemory() const
{
	return !vertices.empty() && !triangles.empty() && !vertexWeights.empty() && !vertexBones.empty();
}

bool SkeletonMesh::isInGpuMemory() const
{
	return vertexBuffer && triangleBuffer && vertexWeightsBuffer && vertexBonesBuffer;
}

const std::vector<float3>& SkeletonMesh::getVertices() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertices - Mesh not loaded in CPU memory." );
	return vertices;
}

std::vector<float3>& SkeletonMesh::getVertices()
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertices - Mesh not loaded in CPU memory." );
	return vertices;
}

std::vector<float>& SkeletonMesh::getVertexWeights()
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertexWeights - Mesh not loaded in CPU memory." );
	return vertexWeights;
}

const std::vector<float>& SkeletonMesh::getVertexWeights() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertexWeights - Mesh not loaded in CPU memory." );
	return vertexWeights;
}

const std::vector<unsigned char>& SkeletonMesh::getVertexBones() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertexBones - Mesh not loaded in CPU memory." );
	return vertexBones;
}

std::vector<unsigned char>& SkeletonMesh::getVertexBones()
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getVertexBones - Mesh not loaded in CPU memory." );
	return vertexBones;
}

const std::vector<float3>& SkeletonMesh::getNormals() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getNormals - Mesh not loaded in CPU memory." );
	return normals;
}

std::vector<float3>& SkeletonMesh::getNormals()
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getNormals - Mesh not loaded in CPU memory." );
	return normals;
}

int SkeletonMesh::getTexcoordsCount() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getTexcoordsCount - Mesh not loaded in CPU memory." );
	return texcoords.size();
}

const std::vector<float2>& SkeletonMesh::getTexcoords( int setIndex ) const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)texcoords.size() ) throw std::exception( "SkeletonMesh::getTexcoords: Trying to access texcoords at non-existing index" );

	std::list< std::vector<float2> >::const_iterator it = texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) ++it;

	return *it;
}

std::vector<float2>& SkeletonMesh::getTexcoords( int setIndex )
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getTexcoords - Mesh not loaded in CPU memory." );
	if ( setIndex >= (int)texcoords.size() ) throw std::exception( "SkeletonMesh::getTexcoords: Trying to access texcoords at non-existing index" );

	std::list< std::vector<float2> >::iterator it = texcoords.begin();
	for ( int i = 0; i < setIndex; ++i ) ++it;

	return *it;
}

const std::vector<uint3>& SkeletonMesh::getTriangles() const
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getTriangles - Mesh not loaded in CPU memory." );
	return triangles;
}

std::vector<uint3>& SkeletonMesh::getTriangles()
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::getTriangles - Mesh not loaded in CPU memory." );
	return triangles;
}

ID3D11Buffer* SkeletonMesh::getVertexBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getVertexBuffer - Mesh not loaded in GPU memory." );
	return vertexBuffer.Get();
}

ID3D11Buffer* SkeletonMesh::getVertexWeightsBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getVertexWeightsBuffer - Mesh not loaded in GPU memory." );
	return vertexWeightsBuffer.Get();
}

ID3D11Buffer* SkeletonMesh::getVertexBonesBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getVertexBonesBuffer - Mesh not loaded in GPU memory." );
	return vertexBonesBuffer.Get();
}

ID3D11Buffer* SkeletonMesh::getNormalBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getNormalBuffer - Mesh not loaded in GPU memory." );
	return normalBuffer.Get();
}

std::list< ID3D11Buffer* > SkeletonMesh::getTexcoordBuffers() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getTexcoordBuffers - Mesh not loaded in GPU memory." );
	
	std::list< ID3D11Buffer* > tmpTexcoordBuffers;

	for ( auto it = texcoordBuffers.begin( ); it != texcoordBuffers.end( ); ++it ) {
		tmpTexcoordBuffers.push_back( it->Get( ) );
	}

	return tmpTexcoordBuffers;
}

ID3D11Buffer* SkeletonMesh::getTriangleBuffer() const
{
	if ( !isInGpuMemory() ) throw std::exception( "SkeletonMesh::getTriangleBuffer - Mesh not loaded in GPU memory." );
	return triangleBuffer.Get();
}

void SkeletonMesh::attachVertexToBone( int vertexIndex, unsigned char boneIndex, float weight )
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::attachVertexToBone - Mesh not loaded in CPU memory." );

	if ( bonesPerVertexCount == BonesPerVertexCount::Type::ZERO ) throw std::exception( "SkeletonMesh::attachVertexToBone - mesh's bonesPerVertexCount value is ZERO - cannot attach vertices." );

	const int bonesPerVertexCountInt = static_cast<int>( bonesPerVertexCount );

	unsigned char* selectedVertexBones = &vertexBones.at( vertexIndex * bonesPerVertexCountInt );
	float*         selectedVertexWeights = &vertexWeights.at( vertexIndex * bonesPerVertexCountInt );

	if ( selectedVertexBones[ 0 ] == 0 ) { // If all slots are empty.
		selectedVertexBones[ 0 ] = boneIndex;
		selectedVertexWeights[ 0 ] = weight;
	} else if ( selectedVertexBones[ bonesPerVertexCountInt - 1 ] != 0 && selectedVertexWeights[ bonesPerVertexCountInt - 1 ] > weight ) { // All slots are taken and the already assigned bones have higher weight than the new bone.
		throw std::exception( "SkeletonMesh::attachVertexToBone - vertex has all the bones assigned - no more empty slots to add a new bone and all existing weights are higher than the new one." );
	} else { // Some slots are empty or some bones have lower weight than the new bone.
		// Insert the new bone at the last slot.
		int boneSlotIndex = bonesPerVertexCountInt - 1;
		selectedVertexBones[ boneSlotIndex ] = boneIndex;
		selectedVertexWeights[ boneSlotIndex ] = weight;

		// Move the new bone to the correct slot - to keep descending order of weights.
		while ( boneSlotIndex > 0 && ( selectedVertexBones[ boneSlotIndex - 1 ] == 0 || selectedVertexWeights[ boneSlotIndex - 1 ] < selectedVertexWeights[ boneSlotIndex ] ) ) {
			unsigned char tmpBoneIndex = selectedVertexBones[ boneSlotIndex - 1 ];
			float         tmpWeight    = selectedVertexWeights[ boneSlotIndex - 1 ];

			selectedVertexBones[ boneSlotIndex - 1 ]   = selectedVertexBones[ boneSlotIndex ];
			selectedVertexWeights[ boneSlotIndex - 1 ] = selectedVertexWeights[ boneSlotIndex ];

			selectedVertexBones[ boneSlotIndex ]   = tmpBoneIndex;
			selectedVertexWeights[ boneSlotIndex ] = tmpWeight;

			--boneSlotIndex;
		}
	}
}


bool SkeletonMesh::attachVertexToBoneIfPossible( int vertexIndex, unsigned char boneIndex, float weight )
{
	if ( !isInCpuMemory() ) throw std::exception( "SkeletonMesh::attachVertexToBoneIfPossible - Mesh not loaded in CPU memory." );
	if ( bonesPerVertexCount == BonesPerVertexCount::Type::ZERO ) throw std::exception( "SkeletonMesh::attachVertexToBone - mesh's bonesPerVertexCount value is ZERO - cannot attach vertices." );

	const int bonesPerVertexCountInt = static_cast<int>( bonesPerVertexCount );

	unsigned char* selectedVertexBones = &vertexBones.at( vertexIndex * bonesPerVertexCountInt );
	float* selectedVertexWeights = &vertexWeights.at( vertexIndex * bonesPerVertexCountInt );

	if ( selectedVertexBones[ 0 ] == 0 ) { // If all slots are empty.
		selectedVertexBones[ 0 ] = boneIndex;
		selectedVertexWeights[ 0 ] = weight;

		return true;

	} else if ( selectedVertexBones[ bonesPerVertexCountInt - 1 ] == 0 || selectedVertexWeights[ bonesPerVertexCountInt - 1 ] < weight ) { // Some slots are empty or some bones have lower weight than the new bone.
		// Insert the new bone at the last slot.
		int boneSlotIndex = bonesPerVertexCountInt - 1;
		selectedVertexBones[ boneSlotIndex ]   = boneIndex;
		selectedVertexWeights[ boneSlotIndex ] = weight;

		// Move the new bone to the correct slot - to keep descending order of weights.
		while ( boneSlotIndex > 0 && ( selectedVertexBones[ boneSlotIndex - 1 ] == 0 || selectedVertexWeights[ boneSlotIndex - 1 ] < selectedVertexWeights[ boneSlotIndex ] ) ) {
			unsigned char tmpBoneIndex = selectedVertexBones[ boneSlotIndex - 1 ];
			float         tmpWeight    = selectedVertexWeights[ boneSlotIndex - 1 ];

			selectedVertexBones[ boneSlotIndex - 1 ]   = selectedVertexBones[ boneSlotIndex ];
			selectedVertexWeights[ boneSlotIndex - 1 ] = selectedVertexWeights[ boneSlotIndex ];

			selectedVertexBones[ boneSlotIndex ]   = tmpBoneIndex;
			selectedVertexWeights[ boneSlotIndex ] = tmpWeight;

			--boneSlotIndex;
		}

		return true;
	}

	// Otherwise - all slots are taken and the already assigned bones have higher weight than the new bone - ignore the new bone.
	return false;
}

void SkeletonMesh::normalizeVertexWeights()
{
	const unsigned int bonesPerVertexCountInt = static_cast<unsigned int>( bonesPerVertexCount );

	for ( unsigned int i = 0; i < vertexWeights.size( ); i += bonesPerVertexCountInt )
	{
		float* selectedVertexWeights = &vertexWeights.at( i );

		// Calculate sum of weights.
		float weightsSum = 0.0f;
		for ( unsigned int j = 0; j < bonesPerVertexCountInt; ++j )
			weightsSum += selectedVertexWeights[ j ];

		// Normalize weights if needed.
		if ( weightsSum > 0.0f && weightsSum != 1.0f ) {
			for ( unsigned int j = 0; j < bonesPerVertexCountInt; ++j )
				selectedVertexWeights[ j ] /= weightsSum;
		}
	}
}

unsigned char SkeletonMesh::getBoneCount( ) const 
{ 
	return static_cast<unsigned char>( bones.size( ) ); 
}

BonesPerVertexCount::Type SkeletonMesh::getBonesPerVertexCount( ) const 
{
	return bonesPerVertexCount;
}

SkeletonMesh::Bone::Bone() :
	name( "" ),
	parentBoneIndex( 0 )
{
	bindPose.identity();
	bindPoseInv.identity();
}

SkeletonMesh::Bone::Bone( const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose ) :
	name( name ),
	parentBoneIndex( parentBoneIndex ),
	bindPose( bindPose ),
	bindPoseInv( bindPose.getScaleOrientationTranslationInverse( ) )
{}

SkeletonMesh::Bone::Bone( const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose, const float43& bindPoseInv ) :
	name( name ),
	parentBoneIndex( parentBoneIndex ),
	bindPose( bindPose ),
	bindPoseInv( bindPoseInv )
{}

void SkeletonMesh::addOrModifyBone( const unsigned char boneIndex, const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose )
{
	// Note: boneIndex is in range 1 - 255.
	if ( boneIndex == 0 ) throw std::exception( "SkeletonMesh::addOrModifyBone - boneIndex cannot be 0." );

#ifdef _DEBUG
	unsigned char checkedBoneIndex = 1;
	for ( Bone& bone : bones )
	{
		if ( checkedBoneIndex != boneIndex && bone.getName().compare( name ) == 0 ) {
			throw std::exception( ( std::string( "SkeletonMesh::addOrModifyBone - another bone with such name already exists. (name = \"" ) + name + std::string( "\")." ) ).c_str() );
		}
		++checkedBoneIndex;
	}
#endif

	// Increse the size of the vector of bones if needed.
	if ( bones.size() < boneIndex ) bones.resize( boneIndex );

	// Assign the bone.
	bones.at( boneIndex - 1 ) = Bone( name, parentBoneIndex, bindPose );
}

void SkeletonMesh::addOrModifyBone( const unsigned char boneIndex, const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose, const float43& bindPoseInv )
{
	// Note: boneIndex is in range 1 - 255.
	if ( boneIndex == 0 ) throw std::exception( "SkeletonMesh::addOrModifyBone - boneIndex cannot be 0." );

#ifdef _DEBUG
	unsigned char checkedBoneIndex = 1;
	for ( Bone& bone : bones )
	{
		if ( checkedBoneIndex != boneIndex && bone.getName().compare( name ) == 0 ) {
			throw std::exception( ( std::string( "SkeletonMesh::addOrModifyBone - another bone with such name already exists. (name = \"" ) + name + std::string( "\")." ) ).c_str() );
		}
		++checkedBoneIndex;
	}
#endif

	// Increse the size of the vector of bones if needed.
	if ( bones.size() < boneIndex ) bones.resize( boneIndex );

	// Assign the bone.
	bones.at( boneIndex - 1 ) = Bone( name, parentBoneIndex, bindPose, bindPoseInv );
}

unsigned char SkeletonMesh::getBoneIndex( const std::string& name ) const
{
	for ( unsigned char i = 0; i < bones.size(); ++i ) {
		if ( bones.at( i ).getName().compare( name ) == 0 ) return i + 1; // i + 1 - because bone indexing starts at 1.
	}

	//bone not found
	throw std::exception( ( std::string( "SkeletonMesh::getBoneIndex - there's no bone with such name (name=\"" ) + name + std::string( "\")." ) ).c_str() );
}

std::tuple< bool, unsigned char > SkeletonMesh::findBoneIndex( const std::string& name ) const
{
	for ( unsigned char i = 0; i < bones.size(); ++i ) {
		if ( bones.at( i ).getName().compare( name ) == 0 ) return std::make_tuple( true, i + 1 ); // i + 1 - because bone indexing starts at 1.
	}

	return std::make_tuple( false, 0 );
}

const SkeletonMesh::Bone& SkeletonMesh::getBone( const std::string& name ) const
{
	return bones.at( getBoneIndex( name ) );
}

const SkeletonMesh::Bone& SkeletonMesh::getBone( unsigned char index ) const
{
	// Note: boneIndex is in range 1 - 255.
	if ( index == 0 ) throw std::exception( "SkeletonMesh::getBone - boneIndex cannot be 0." );

	return bones.at( index - 1 );
}
