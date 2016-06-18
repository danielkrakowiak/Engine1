#include "MyOBJFileParser.h"

#include <algorithm>

#include "BlockMesh.h"
#include "SkeletonMesh.h"

#include "Importer.hpp"
#include "scene.h" 
#include "postprocess.h"

#include "float44.h"

using namespace Engine1;

std::vector< std::shared_ptr<BlockMesh> > MyOBJFileParser::parseBlockMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::vector< std::shared_ptr<BlockMesh> > meshes;

	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

	if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
	if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
	if ( flipUVs )                  flags |= aiProcess_FlipUVs;
    if ( flipUVs )                  flags |= aiProcess_FlipUVs; 

    const int      dataSize = (int)(dataEndIt - dataIt) / sizeof(char);
    const aiScene* aiscene = importer.ReadFileFromMemory( &(*dataIt), dataSize, flags );

	if ( !aiscene ) throw std::exception( "MyOBJFileParser::parseBlockMeshFile - parsing failed" );

	for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
		meshes.push_back( std::make_shared<BlockMesh>( ) );
		BlockMesh& mesh = *meshes.back();
		
		aiMesh& aimesh = *aiscene->mMeshes[ meshIndex ];

		mesh.vertices.reserve( aimesh.mNumVertices );
		for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.vertices.push_back( *(float3*)&aimesh.mVertices[ i ] );

		if ( aimesh.HasNormals() ) {
			mesh.normals.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.normals.push_back( *(float3*)&aimesh.mNormals[ i ] );
		}

        if ( aimesh.HasTangentsAndBitangents() ) {
			mesh.tangents.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.tangents.push_back( *(float3*)&aimesh.mTangents[ i ] );
		}

		if ( aimesh.HasTextureCoords( 0 ) ) { 
			mesh.texcoords.push_back( std::vector<float2>() );
			std::vector<float2>& texcoords = mesh.texcoords.front();

			texcoords.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) 
				texcoords.push_back( float2( aimesh.mTextureCoords[ 0 ][ i ].x, aimesh.mTextureCoords[ 0 ][ i ].y ) );
		}

		mesh.triangles.reserve( aimesh.mNumFaces );
		for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
			mesh.triangles.push_back( *reinterpret_cast<uint3*>( aimesh.mFaces[ i ].mIndices ) );
		}
	}

    for ( std::shared_ptr<BlockMesh>& mesh : meshes )
        mesh->recalculateBoundingBox();

	return meshes;
}