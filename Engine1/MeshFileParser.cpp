#include "MeshFileParser.h"

#include <algorithm>

#include "BlockMesh.h"
#include "SkeletonMesh.h"

#include "BVHTreeBufferParser.h"

#include "Assimp/Importer.hpp"
#include "Assimp/Exporter.hpp"
#include "Assimp/scene.h" 
#include "Assimp/postprocess.h"

#include "float44.h"

using namespace Engine1;

std::vector< std::shared_ptr<BlockMesh> > MeshFileParser::parseBlockMeshFile( BlockMeshFileInfo::Format format, std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
    if ( format == BlockMeshFileInfo::Format::BLOCKMESH )
    {
        std::shared_ptr< BlockMesh > mesh = parseBlockMeshFileOwnFormat( dataIt, dataEndIt );
        
        std::vector< std::shared_ptr<BlockMesh> > meshes = { mesh };

        return meshes;
    }
    else
        return parseBlockMeshFileAssimp( format, dataIt, dataEndIt, invertZCoordinate, invertVertexWindingOrder, flipUVs );
}

std::vector< std::shared_ptr<SkeletonMesh> > MeshFileParser::parseSkeletonMeshFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
    // #TODO: Should also handle own format parsing.
    return parseSkeletonMeshFileAssimp( dataIt, dataEndIt, invertZCoordinate, invertVertexWindingOrder, flipUVs );
}

std::vector< std::shared_ptr<BlockMesh> > MeshFileParser::parseBlockMeshFileAssimp( BlockMeshFileInfo::Format format, std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
    std::vector< std::shared_ptr<BlockMesh> > meshes;

    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

    if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
    if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
    if ( flipUVs )                  flags |= aiProcess_FlipUVs;

    // #TODO: refactor - use something like toString(format);
    const std::string formatHint = BlockMeshFileInfo::formatToString( format );

    const int      dataSize = (int)( dataEndIt - dataIt ) / sizeof( char );
    const aiScene* aiscene = importer.ReadFileFromMemory( &( *dataIt ), dataSize, flags, formatHint.c_str() );

    if ( !aiscene ) throw std::exception( ( "MeshFileParser::parseBlockMeshFile - parsing failed - " + std::string( importer.GetErrorString() ) ).c_str() );

    for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
        meshes.push_back( std::make_shared<BlockMesh>() );
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

std::shared_ptr<BlockMesh> MeshFileParser::parseBlockMeshFileOwnFormat( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt )
{
    std::shared_ptr< BlockMesh > mesh = std::make_shared< BlockMesh >();

    const int dataSize = (int)( dataEndIt - dataIt ) / sizeof( char );

    std::vector<char>::const_iterator dataCurrIt = dataIt;

    // Read vertex count.
    int vertexCount = 0; 
    std::memcpy( &vertexCount, &(*dataCurrIt), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read vertices.
    mesh->m_vertices.resize( vertexCount );
    const int verticesDataSize = vertexCount * sizeof( float3 );
    std::memcpy( mesh->m_vertices.data(), &(*dataCurrIt), verticesDataSize );
    dataCurrIt += verticesDataSize;

    // Read normals count.
    int normalsCount = 0; 
    std::memcpy( &normalsCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read normals.
    mesh->m_normals.resize( normalsCount );
    const int normalsDataSize = normalsCount * sizeof( float3 );
    std::memcpy( mesh->m_normals.data(), &( *dataCurrIt ), normalsDataSize );
    dataCurrIt += normalsDataSize;

    // Read tangents count.
    int tangentsCount = 0;
    std::memcpy( &tangentsCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read tangents.
    mesh->m_tangents.resize( tangentsCount );
    const int tangentsDataSize = normalsCount * sizeof( float3 );
    std::memcpy( mesh->m_tangents.data(), &( *dataCurrIt ), tangentsDataSize );
    dataCurrIt += tangentsDataSize;

    // Read texcoords set count.
    int texcoordsSetCount = 0;
    std::memcpy( &texcoordsSetCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read texcoords (all sets).
    mesh->m_texcoords.resize( texcoordsSetCount );
    for ( auto& texcoords : mesh->m_texcoords )
    {
        // Read texcoords count.
        int texcoordsCount = 0;
        std::memcpy( &texcoordsCount, &( *dataCurrIt ), sizeof( int ) );
        dataCurrIt += sizeof( int );

        // Read tangents.
        texcoords.resize( texcoordsCount );
        const int texcoordsDataSize = texcoordsCount * sizeof( float2 );
        std::memcpy( texcoords.data(), &( *dataCurrIt ), texcoordsDataSize );
        dataCurrIt += texcoordsDataSize;
    }

    // Read triangle count.
    int triangleCount = 0;
    std::memcpy( &triangleCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read triangles.
    mesh->m_triangles.resize( triangleCount );
    const int trianglesDataSize = triangleCount * sizeof( uint3 );
    std::memcpy( mesh->m_triangles.data(), &( *dataCurrIt ), trianglesDataSize );
    dataCurrIt += trianglesDataSize;

    // Read bounding box.
    float3 bbMin, bbMax;
    std::memcpy( &bbMin, &( *dataCurrIt ), sizeof( float3 ) );
    dataCurrIt += sizeof( float3 );
    std::memcpy( &bbMax, &( *dataCurrIt ), sizeof( float3 ) );
    dataCurrIt += sizeof( float3 );
    mesh->m_boundingBox.set( bbMin, bbMax );

    // Read if mesh has BVH tree.
    bool hasBVHTree = false;
    std::memcpy( &hasBVHTree, &( *dataCurrIt ), sizeof( bool ) );
    dataCurrIt += sizeof( bool );

    // Read BVH tree.
    if ( hasBVHTree )
        mesh->setBvhTree( BVHTreeBufferParser::parseBVHTreeFile( dataCurrIt, dataEndIt ) );

    return mesh;
}

std::vector< std::shared_ptr<SkeletonMesh> > MeshFileParser::parseSkeletonMeshFileAssimp( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
    std::vector< std::shared_ptr<SkeletonMesh> > meshes;

    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;

    if ( invertZCoordinate )        flags |= aiProcess_MakeLeftHanded;
    if ( invertVertexWindingOrder ) flags |= aiProcess_FlipWindingOrder;
    if ( flipUVs )                  flags |= aiProcess_FlipUVs;

    const int      dataSize = (int)( dataEndIt - dataIt ) / sizeof( char );
    const aiScene* aiscene = importer.ReadFileFromMemory( &( *dataIt ), dataSize, flags );

    if ( !aiscene ) throw std::exception( "MeshFileParser::parseRiggedMeshFile - parsing failed" );

    for ( unsigned int meshIndex = 0; meshIndex < aiscene->mNumMeshes; ++meshIndex ) {
        meshes.push_back( std::make_shared<SkeletonMesh>() );
        SkeletonMesh& mesh = *meshes.back();

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
            const int count = (int)mesh.m_vertices.size() * static_cast<int>( mesh.bonesPerVertexCount );
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

std::shared_ptr<SkeletonMesh> MeshFileParser::parseSkeletonMeshFileOwnFormat( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt )
{
    dataIt;
    dataEndIt;

    throw std::exception( "MeshFileParser::parseSkeletonMeshFileOwnFormat - not yet implemented." );
}

void MeshFileParser::writeBlockMeshFile( std::vector< char >& data, const BlockMeshFileInfo::Format format, const BlockMesh& mesh )
{
    if ( format == BlockMeshFileInfo::Format::BLOCKMESH )
        writeBlockMeshFileOwnFormat( data, mesh );
    else
        writeBlockMeshFileAssimp( data, format, mesh );
}

void MeshFileParser::writeBlockMeshFileAssimp( std::vector< char >& data, const BlockMeshFileInfo::Format format, const BlockMesh& mesh )
{
    aiScene     aiscene          = {};
    aiNode      airootNode       = {};
    aiMesh      aimesh           = {};
    aiMesh*     aimeshes[ 1 ]    = { &aimesh };
    aiMaterial  aimaterial       = {};
    aiMaterial* aimaterials[ 1 ] = { &aimaterial };

    unsigned int airootNodeMeshes[ 1 ] = { 0 };

    airootNode.mNumMeshes = 1;
    airootNode.mMeshes    = airootNodeMeshes;

    aiscene.mRootNode     = &airootNode;
    aiscene.mNumMeshes    = 1;
    aiscene.mMeshes       = aimeshes;
    aiscene.mNumMaterials = 1;
    aiscene.mMaterials    = aimaterials;

    { // Assign vertices and normals.
        aimesh.mNumVertices = (unsigned int)mesh.getVertices().size();
        // Note: Casting away const to avoid copying data - because we know that Assimp won't modify the data.
        aimesh.mVertices = reinterpret_cast<aiVector3D*>( const_cast<float3*>( mesh.getVertices().data() ) );
        aimesh.mNormals  = !mesh.getNormals().empty() ? reinterpret_cast<aiVector3D*>( const_cast<float3*>( mesh.getNormals().data() ) ) : nullptr;
        aimesh.mTangents = !mesh.getTangents().empty() ? reinterpret_cast<aiVector3D*>( const_cast<float3*>( mesh.getTangents().data() ) ) : nullptr;
    }

    std::vector< std::vector< aiVector3D > > aitexcoordSets;
    { // Copy texcoords.
        int aitexcoordSetCount = std::min( mesh.getTexcoordsCount(), AI_MAX_NUMBER_OF_TEXTURECOORDS );
        aitexcoordSets.resize( aitexcoordSetCount );

        for ( int texcoordSetIndex = 0; texcoordSetIndex < aitexcoordSetCount; ++texcoordSetIndex ) {
            const std::vector< float2 >& meshTexcoordSet = mesh.getTexcoords( texcoordSetIndex );
            std::vector< aiVector3D >&   aitexcoordSet = aitexcoordSets[ texcoordSetIndex ];

            // Copy texcoords from mesh to Assimp mesh.
            aitexcoordSet.resize( mesh.getTexcoords( texcoordSetIndex ).size() );
            for ( unsigned int vertexIndex = 0; vertexIndex < aimesh.mNumVertices; ++vertexIndex )
                aitexcoordSet[ vertexIndex ] = aiVector3D( meshTexcoordSet[ vertexIndex ].x, meshTexcoordSet[ vertexIndex ].y, 0.0f );

            aimesh.mTextureCoords[ texcoordSetIndex ] = aitexcoordSet.data();
            aimesh.mNumUVComponents[ texcoordSetIndex ] = 2; // Two components - U and V.
        }
    }

    std::vector< aiFace > aifaces;
    { // Copy triangles.
        aimesh.mNumFaces = (unsigned int)mesh.getTriangles().size();
        aifaces.resize( aimesh.mNumFaces );

        // Note: Casting away const to avoid copying data - because we know that Assimp won't modify the data.
        std::vector< uint3 >& meshTriangles = const_cast<std::vector< uint3 >&>( mesh.getTriangles() );

        aimesh.mFaces = aifaces.data();
        for ( unsigned int faceIndex = 0; faceIndex < aimesh.mNumFaces; ++faceIndex ) {
            aifaces[ faceIndex ].mNumIndices = 3;
            aifaces[ faceIndex ].mIndices = reinterpret_cast<unsigned int*>( &meshTriangles[ faceIndex ] );
        }
    }

    { // Export mesh to file (in memory).
        const std::string formatString = BlockMeshFileInfo::formatToString( format );

        Assimp::Exporter exporter;
        const aiExportDataBlob* aidata = exporter.ExportToBlob( &aiscene, formatString, 0 );

        // Copy mesh file to the data vector.
        data.clear();
        data.resize( aidata->size );
        std::memcpy( data.data(), aidata->data, aidata->size );
    }

    { // Nullify Assimp mesh pointers to avoid deleting the original mesh data or deleting temporary data twice.
        for ( unsigned int faceIndex = 0; faceIndex < aimesh.mNumFaces; ++faceIndex )
            aifaces[ faceIndex ].mIndices = nullptr;

        std::memset( &aimesh, 0, sizeof( aimesh ) );
        std::memset( &aiscene, 0, sizeof( aiscene ) );
        std::memset( &airootNode, 0, sizeof( airootNode ) );
    }
}

void MeshFileParser::writeBlockMeshFileOwnFormat( std::vector< char >& data, const BlockMesh& mesh )
{
    const int verticesDataSize  = (int)mesh.getVertices().size() * sizeof( float3 );
    const int normalsDataSize   = (int)mesh.getNormals().size() * sizeof( float3 );
    const int tangentsDataSize  = (int)mesh.getTangents().size() * sizeof( float3 );
    const int trianglesDataSize = (int)mesh.getTriangles().size() * sizeof( uint3 );

    int totalSize = 0;
    totalSize += 3 * sizeof( int ); // Vertices, normals, tangents counts.
    totalSize += verticesDataSize;  // Vertices.
    totalSize += normalsDataSize;   // Normals.
    totalSize += tangentsDataSize;  // Tangents.

    totalSize += ( 1 + mesh.getTexcoordsCount() ) * sizeof( int ); // Texcoord set count, and texcoord count in each set.
    for ( int texcordSetIdx = 0; texcordSetIdx < mesh.getTexcoordsCount(); ++texcordSetIdx )
        totalSize += (int)mesh.getTexcoords( texcordSetIdx ).size() * sizeof( float2 ); // Texcoords.

    totalSize += sizeof( int );     // Triangle count.
    totalSize += trianglesDataSize; // Triangles.

    totalSize += 2 * sizeof( float3 ); // Bounding box.
    totalSize += sizeof( bool );       // Has BVH tree flag.

    const int bvhTreeSize = mesh.getBvhTree() ? BVHTreeBufferParser::getSizeOfBVHTreeFile( *mesh.getBvhTree() ) : 0;

    data.reserve( totalSize + bvhTreeSize );
    data.resize( totalSize );

    std::vector< char >::iterator dataIt = data.begin();

    // Write vertex count.
    const int vertexCount = (int)mesh.getVertices().size();
    std::memcpy( &(*dataIt), &vertexCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write vertices.
    std::memcpy( &(*dataIt), mesh.getVertices().data(), verticesDataSize );
    dataIt += verticesDataSize;

    // Write normals count.
    const int normalCount = (int)mesh.getNormals().size();
    std::memcpy( &(*dataIt), &normalCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write normals.
    std::memcpy( &( *dataIt ), mesh.getNormals().data(), normalsDataSize );
    dataIt += normalsDataSize;

    // Write tangents count.
    const int tangentsCount = (int)mesh.getTangents().size();
    std::memcpy( &( *dataIt ), &tangentsCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write tangents.
    std::memcpy( &( *dataIt ), mesh.getTangents().data(), tangentsDataSize );
    dataIt += tangentsDataSize;

    // Write texcoord set count.
    const int texcoordsSetCount = (int)mesh.getTexcoordsCount();
    std::memcpy( &( *dataIt ), &texcoordsSetCount, sizeof( int ) );
    dataIt += sizeof( int );

    for ( int texcordSetIdx = 0; texcordSetIdx < mesh.getTexcoordsCount(); ++texcordSetIdx )
    {
        // Write texcoords count.
        const int texcoordsCount = (int)mesh.getTexcoords( texcordSetIdx ).size();
        std::memcpy( &( *dataIt ), &texcoordsCount, sizeof( int ) );
        dataIt += sizeof( int );

        // Write texcoords.
        const int texcoordsDataSize = (int)mesh.getTexcoords( texcordSetIdx ).size() * sizeof( float2 );
        std::memcpy( &( *dataIt ), mesh.getTexcoords( texcordSetIdx ).data(), texcoordsDataSize );
        dataIt += texcoordsDataSize;
    }

    // Write triangles count.
    const int trianglesCount = (int)mesh.getTriangles().size();
    std::memcpy( &( *dataIt ), &trianglesCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write triangles.
    std::memcpy( &( *dataIt ), mesh.getTriangles().data(), trianglesDataSize );
    dataIt += trianglesDataSize;

    // Write bounding box.
    const BoundingBox bbBox = mesh.getBoundingBox();
    const float3 bbMin( bbBox.getMin() );
    const float3 bbMax( bbBox.getMax() );
    std::memcpy( &( *dataIt ), &bbMin, sizeof( bbMin ) );
    dataIt += sizeof( bbMin );
    std::memcpy( &( *dataIt ), &bbMax, sizeof( bbMax ) );
    dataIt += sizeof( bbMax );

    // Write if it has BVH tree.
    const bool hasBvhTree = mesh.getBvhTree() != nullptr;
    std::memcpy( &( *dataIt ), &hasBvhTree, sizeof( bool ) );
    dataIt += sizeof( bool );

    // Write BVH tree.
    if ( mesh.getBvhTree() )
        BVHTreeBufferParser::writeBVHTreeFile( data, *mesh.getBvhTree() );
}
