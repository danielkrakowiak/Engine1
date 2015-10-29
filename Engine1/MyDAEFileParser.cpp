#include "MyDAEFileParser.h"

#include <algorithm>

#include "BlockMesh.h"
#include "SkeletonMesh.h"

#include "Importer.hpp"
#include "scene.h" 
#include "postprocess.h"

#include "float44.h"


std::vector< std::shared_ptr<BlockMesh> > MyDAEFileParser::parseBlockMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::vector< std::shared_ptr<BlockMesh> > meshes;

	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate;

	if ( invertZCoordinate ) flags |= aiProcess_MakeLeftHanded;
	if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
	if ( flipUVs ) flags |= aiProcess_FlipUVs; 

	const aiScene* aiscene = importer.ReadFileFromMemory( file.data( ), file.size( ), flags );

	if ( !aiscene ) throw std::exception( "MyDAEFileParser::parseBlockMeshFile - parsing failed" );

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

		/*if ( aimesh.HasTextureCoords( 0 ) ) { //TODO: why on character.dae there is only one texcoord (there should be one for each vertex)
			mesh.texcoords.push_back( std::vector<float2>() );
			std::vector<float2>& texcoords = mesh.texcoords.front();

			texcoords.reserve( aimesh.mNumVertices );
			for ( int i = 0; i < aimesh.mNumVertices; ++i ) texcoords.push_back( float2( aimesh.mTextureCoords[ i ]->x, aimesh.mTextureCoords[ i ]->y ) );
		}*/

		mesh.triangles.reserve( aimesh.mNumFaces );
		for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
			mesh.triangles.push_back( *reinterpret_cast<uint3*>( aimesh.mFaces[ i ].mIndices ) );
		}
	}

	return meshes;
}

std::vector< std::shared_ptr<SkeletonMesh> > MyDAEFileParser::parseSkeletonMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::vector< std::shared_ptr<SkeletonMesh> > meshes;

	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate;

	if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
	if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
	if ( flipUVs )                  flags |= aiProcess_FlipUVs;

	const aiScene* aiscene = importer.ReadFileFromMemory( file.data( ), file.size( ), flags );

	if ( !aiscene ) throw std::exception( "MyDAEFileParser::parseRiggedMeshFile - parsing failed" );

	for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
		meshes.push_back( std::make_shared<SkeletonMesh>( ) );
		SkeletonMesh& mesh = *meshes.back( );

		aiMesh& aimesh = *aiscene->mMeshes[ meshIndex ];

		mesh.vertices.reserve( aimesh.mNumVertices );
		for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.vertices.push_back( *(float3*)&aimesh.mVertices[ i ] );

		if ( aimesh.HasNormals( ) ) {
			mesh.normals.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.normals.push_back( *(float3*)&aimesh.mNormals[ i ] );
		}

		/*if ( aimesh.HasTextureCoords( 0 ) ) { //TODO: why on character.dae there is only one texcoord (there should be one for each vertex)
		mesh.texcoords.push_back( std::vector<float2>() );
		std::vector<float2>& texcoords = mesh.texcoords.front();

		texcoords.reserve( aimesh.mNumVertices );
		for ( int i = 0; i < aimesh.mNumVertices; ++i ) texcoords.push_back( float2( aimesh.mTextureCoords[ i ]->x, aimesh.mTextureCoords[ i ]->y ) );
		}*/

		mesh.triangles.reserve( aimesh.mNumFaces );
		for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
			mesh.triangles.push_back( *reinterpret_cast<uint3*>( aimesh.mFaces[ i ].mIndices ) );
		}

		mesh.bonesPerVertexCount = BonesPerVertexCount::Type::FOUR;  //TODO: should be deduced from the file.

		{ // Get bind pose for each bone.
			for ( unsigned int boneIndex = 1; boneIndex <= aimesh.mNumBones; ++boneIndex ) {
				aiBone& aibone = *aimesh.mBones[ boneIndex - 1 ];
				std::string boneName = std::string( aibone.mName.C_Str() );
				//note: transpose because mOffsetMatrix has basis vectors in columns instead of rows
				float43 boneBindPose = float44( &aibone.mOffsetMatrix.a1 ).getTranspose().getOrientationTranslation();

				mesh.addOrModifyBone( boneIndex, boneName, 0, boneBindPose );
			}
		}

		{ // Join bones together.
			for ( unsigned int boneIndex = 1; boneIndex <= mesh.getBoneCount(); ++boneIndex ) {
				SkeletonMesh::Bone bone = mesh.getBone( boneIndex );
				aiNode* aiBoneNode = aiscene->mRootNode->FindNode( bone.getName().c_str() );

				if ( aiBoneNode ) {
					bool parentBoneFound;
					unsigned char parentBoneIndex;

					// Find parent bone in the skeleton.
					std::tie( parentBoneFound, parentBoneIndex ) = mesh.findBoneIndex( aiBoneNode->mParent->mName.C_Str() );

					if ( parentBoneFound ) {
						// Attach bone to the parent bone in the skeleton.
						mesh.addOrModifyBone( boneIndex, bone.getName(), parentBoneIndex, bone.getBindPose(), bone.getBindPoseInv() );
					}
				}
			}
		}

		{ // Allocate memory for bones' weights and indices, zero the memory.
			const int count = mesh.vertices.size( ) * static_cast<int>( mesh.bonesPerVertexCount );
			mesh.vertexBones.reserve( count );
			mesh.vertexWeights.reserve( count );
			for ( int i = 0; i < count; ++i ) {
				mesh.vertexBones.push_back( 0 );
				mesh.vertexWeights.push_back( 0.0f );
			}
		}

		{ // Assign the bones to the vertices.
			for ( unsigned int boneIndex = 1; boneIndex <= aimesh.mNumBones; ++boneIndex ) {
				aiBone& aibone = *aimesh.mBones[ boneIndex - 1 ];

				for ( unsigned int i = 0; i < aibone.mNumWeights; ++i ) {
					aiVertexWeight& aivertexweight = aibone.mWeights[ i ];
					mesh.attachVertexToBoneIfPossible( aivertexweight.mVertexId, boneIndex, aivertexweight.mWeight );
					//mesh.attachVertexToBone( aivertexweight.mVertexId, boneIndex, aivertexweight.mWeight );
				}
			}
		}
		
		// Normalize weights.
		mesh.normalizeVertexWeights();
	}

	return meshes;
}
