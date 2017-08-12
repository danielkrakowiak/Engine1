#include "MeshUtil.h"

#include "BlockMesh.h"

#include <algorithm>

using namespace Engine1;

std::shared_ptr< BlockMesh > MeshUtil::mergeMeshes( const std::vector< std::shared_ptr< BlockMesh > >& meshes )
{
    if ( meshes.empty() )
        throw std::exception( "MeshUtil::mergeMeshes - no meshes to merge." );

    for ( auto& mesh : meshes ) {
        if ( !mesh->isInCpuMemory() )
            throw std::exception( "MeshUtil::mergeMeshes - some of the input meshes are not in CPU memory." );
    }

    int  vertexCount        = 0;
    int  triangleCount      = 0;
    int  texcoordSetCount   = 0;
    bool hasNormalsTangents = !meshes[ 0 ]->getNormals().empty();

    // Count vertices and triangles.
    for ( auto& mesh : meshes ) 
    {
        vertexCount   += (int)mesh->getVertices().size();
        triangleCount += (int)mesh->getTriangles().size();

        texcoordSetCount = std::max( texcoordSetCount, mesh->getTexcoordsCount() );

        if ( hasNormalsTangents == mesh->getNormals().empty() )
            throw std::exception( "MeshUtil::mergeMeshes - some of the passed meshes have normals/tangents and some do not. Merging is possible only if all meshes (or none) have normals/tangents." );
    }

    std::shared_ptr< BlockMesh > mergedMesh = std::make_shared< BlockMesh >( vertexCount, hasNormalsTangents, texcoordSetCount, triangleCount );

    // Copy vertex data from input meshes to the merged mesh.
    // Recalculate vertex indices in triangles taken from input meshes.
    int vertexIndexShift = 0;
    int triangleIndexShift = 0;
    std::vector< uint3 >& mergedMeshTriangles = mergedMesh->getTriangles();

    for ( auto& mesh : meshes )
    {
        const int meshVertexCount = (int)mesh->getVertices().size();

        std::memcpy( &mergedMesh->getVertices()[ vertexIndexShift ], mesh->getVertices().data(), meshVertexCount * sizeof( float3 ) );
        std::memcpy( &mergedMesh->getNormals()[ vertexIndexShift ], mesh->getNormals().data(), meshVertexCount * sizeof( float3 ) );
        std::memcpy( &mergedMesh->getTangents()[ vertexIndexShift ], mesh->getTangents().data(), meshVertexCount * sizeof( float3 ) );

        // Copy texcoords or fill with zeros if not present in input mesh.
        for ( int texcoordSetIndex = 0; texcoordSetIndex < texcoordSetCount; ++texcoordSetIndex )
        {
            if ( mesh->getTexcoordsCount() > texcoordSetIndex )
                std::memcpy( &mergedMesh->getTexcoords( texcoordSetIndex )[ vertexIndexShift ], mesh->getTexcoords( texcoordSetIndex ).data(), meshVertexCount * sizeof( float2 ) );
            else
                std::memset( &mergedMesh->getTexcoords( texcoordSetIndex )[ vertexIndexShift ], 0, meshVertexCount * sizeof( float2 ) );
        }

        const int meshTriangleCount = (int)mesh->getTriangles().size();

        const uint3 vertexIndexShiftVec( vertexIndexShift, vertexIndexShift, vertexIndexShift );

        // Copy triangles from input mesh to the merged mesh - but recalculate it's indices to account for shift in vertices.
        for ( int triangleIndex = 0; triangleIndex < meshTriangleCount; ++triangleIndex )
            mergedMeshTriangles[ triangleIndexShift + triangleIndex ] = mesh->getTriangles()[ triangleIndex ] + vertexIndexShiftVec;

        vertexIndexShift   += (int)mesh->getVertices().size();
        triangleIndexShift += (int)mesh->getTriangles().size();
    }

    return mergedMesh;
}

void MeshUtil::flipTexcoordsVertically( BlockMesh& mesh )
{
    // Flip vertical texcoord.
    for ( auto& texcoordsSet : mesh.m_texcoords )
    {
        for ( auto& texcoord : texcoordsSet ) {
            texcoord.y = 1.0f - texcoord.y;
        }
    }
}

void MeshUtil::flipTangents( BlockMesh& mesh )
{
    for ( auto& tangent : mesh.m_tangents ) 
    {
        tangent = -tangent;
    }
}

void MeshUtil::flipNormals( BlockMesh& mesh )
{
    for ( auto& normal : mesh.m_normals ) 
    {
        normal = -normal;
    }
}

void MeshUtil::invertVertexWindingOrder( BlockMesh& mesh )
{
    for ( auto& triangle : mesh.m_triangles )
    {
        uint3 triangleTemp = triangle;

        triangle.x = triangleTemp.z;
        triangle.z = triangleTemp.x;
    }
}