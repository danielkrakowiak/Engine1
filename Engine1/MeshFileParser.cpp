#include "MeshFileParser.h"

#include <algorithm>

#include "BlockMesh.h"
#include "SkeletonMesh.h"

#include "Importer.hpp"
#include "scene.h" 
#include "postprocess.h"

#include "float44.h"

using namespace Engine1;

std::vector< std::shared_ptr<BlockMesh> > MeshFileParser::parseBlockMeshFile( BlockMeshFileInfo::Format format, std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::vector< std::shared_ptr<BlockMesh> > meshes;

	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

	if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
	if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
	if ( flipUVs )                  flags |= aiProcess_FlipUVs; 

    // #TODO: refactor - use something like toString(format);
    std::string formatHint;
    if ( format == BlockMeshFileInfo::Format::OBJ )
        formatHint = "obj";
    else if ( format == BlockMeshFileInfo::Format::DAE )
        formatHint = "dae";
    else if ( format == BlockMeshFileInfo::Format::FBX )
        formatHint = "fbx";

    const int      dataSize = (int)(dataEndIt - dataIt) / sizeof(char);
    const aiScene* aiscene = importer.ReadFileFromMemory( &(*dataIt), dataSize, flags, formatHint.c_str() );

	if ( !aiscene ) throw std::exception( ("MeshFileParser::parseBlockMeshFile - parsing failed - " + std::string( importer.GetErrorString() )).c_str() );

	for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
		meshes.push_back( std::make_shared<BlockMesh>( ) );
		BlockMesh& mesh = *meshes.back();
		
		aiMesh& aimesh = *aiscene->mMeshes[ meshIndex ];

		mesh.m_vertices.reserve( aimesh.mNumVertices );
		for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_vertices.push_back( *(float3*)&aimesh.mVertices[ i ] );

		if ( aimesh.HasNormals() ) {
			mesh.m_normals.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_normals.push_back( *(float3*)&aimesh.mNormals[ i ] );
		}

        if ( aimesh.HasTangentsAndBitangents() ) {
			mesh.m_tangents.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_tangents.push_back( *(float3*)&aimesh.mTangents[ i ] );
		}

		if ( aimesh.HasTextureCoords( 0 ) ) { 
			mesh.m_texcoords.push_back( std::vector<float2>() );
			std::vector<float2>& texcoords = mesh.m_texcoords.front();

			texcoords.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) 
				texcoords.push_back( float2( aimesh.mTextureCoords[ 0 ][ i ].x, aimesh.mTextureCoords[ 0 ][ i ].y ) );
		}

		mesh.m_triangles.reserve( aimesh.mNumFaces );
		for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
			mesh.m_triangles.push_back( *reinterpret_cast<uint3*>( aimesh.mFaces[ i ].mIndices ) );
		}
	}

    for ( std::shared_ptr<BlockMesh>& mesh : meshes )
        mesh->recalculateBoundingBox();

	return meshes;
}

std::vector< std::shared_ptr<SkeletonMesh> > MeshFileParser::parseSkeletonMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	std::vector< std::shared_ptr<SkeletonMesh> > meshes;

	Assimp::Importer importer;

	unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

	if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
	if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
	if ( flipUVs )                  flags |= aiProcess_FlipUVs;

    const int      dataSize = (int)(dataEndIt - dataIt) / sizeof(char);
    const aiScene* aiscene = importer.ReadFileFromMemory( &(*dataIt), dataSize, flags );

	if ( !aiscene ) throw std::exception( "MeshFileParser::parseRiggedMeshFile - parsing failed" );

	for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
		meshes.push_back( std::make_shared<SkeletonMesh>( ) );
		SkeletonMesh& mesh = *meshes.back( );

		aiMesh& aimesh = *aiscene->mMeshes[ meshIndex ];

		mesh.m_vertices.reserve( aimesh.mNumVertices );
		for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_vertices.push_back( *(float3*)&aimesh.mVertices[ i ] );

		if ( aimesh.HasNormals( ) ) {
			mesh.m_normals.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_normals.push_back( *(float3*)&aimesh.mNormals[ i ] );
		}

        if ( aimesh.HasTangentsAndBitangents() ) {
			mesh.m_tangents.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i ) mesh.m_tangents.push_back( *(float3*)&aimesh.mTangents[ i ] );
		}

		if ( aimesh.HasTextureCoords( 0 ) ) {
			mesh.m_texcoords.push_back( std::vector<float2>() );
			std::vector<float2>& texcoords = mesh.m_texcoords.front();

			texcoords.reserve( aimesh.mNumVertices );
			for ( unsigned int i = 0; i < aimesh.mNumVertices; ++i )
				texcoords.push_back( float2( aimesh.mTextureCoords[ 0 ][ i ].x, aimesh.mTextureCoords[ 0 ][ i ].y ) );
		}

		mesh.m_triangles.reserve( aimesh.mNumFaces );
		for ( unsigned int i = 0; i < aimesh.mNumFaces; ++i ) {
			mesh.m_triangles.push_back( *reinterpret_cast<uint3*>( aimesh.mFaces[ i ].mIndices ) );
		}

		mesh.bonesPerVertexCount = BonesPerVertexCount::Type::FOUR;  //TODO: should be deduced from the file.

		{ // Get bind pose for each bone.
			for ( unsigned int boneIndex = 1; boneIndex <= aimesh.mNumBones; ++boneIndex ) {
				aiBone& aibone = *aimesh.mBones[ boneIndex - 1 ];
				std::string boneName = std::string( aibone.mName.C_Str() );
				//note: transpose because mOffsetMatrix has basis vectors in columns instead of rows
				float43 boneBindPose = float44( &aibone.mOffsetMatrix.a1 ).getTranspose().getOrientationTranslation();

				mesh.addOrModifyBone( (unsigned char)boneIndex, boneName, 0, boneBindPose );
			}
		}

		{ // Join bones together.
			for ( unsigned char boneIndex = 1; boneIndex <= mesh.getBoneCount(); ++boneIndex ) {
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
			const int count = (int)mesh.m_vertices.size( ) * static_cast<int>( mesh.bonesPerVertexCount );
			mesh.m_vertexBones.reserve( count );
			mesh.m_vertexWeights.reserve( count );
			for ( int i = 0; i < count; ++i ) {
				mesh.m_vertexBones.push_back( 0 );
				mesh.m_vertexWeights.push_back( 0.0f );
			}
		}

		{ // Assign the bones to the vertices.
			for ( unsigned int boneIndex = 1; boneIndex <= aimesh.mNumBones && boneIndex < 256; ++boneIndex ) {
				aiBone& aibone = *aimesh.mBones[ boneIndex - 1 ];

				for ( unsigned int i = 0; i < aibone.mNumWeights; ++i ) {
					aiVertexWeight& aivertexweight = aibone.mWeights[ i ];
					mesh.attachVertexToBoneIfPossible( aivertexweight.mVertexId, (unsigned char)boneIndex, aivertexweight.mWeight );
					//mesh.attachVertexToBone( aivertexweight.mVertexId, boneIndex, aivertexweight.mWeight );
				}
			}
		}
		
		// Normalize weights.
		mesh.normalizeVertexWeights();
	}

    for ( std::shared_ptr<SkeletonMesh>& mesh : meshes )
        mesh->recalculateBoundingBox();

	return meshes;
}
