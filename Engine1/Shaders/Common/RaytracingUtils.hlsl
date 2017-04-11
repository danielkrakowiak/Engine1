
#ifndef RAYTRACING_UTILS_HLSL
#define RAYTRACING_UTILS_HLSL

#include "Common\Constants.hlsl"

bool rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float3 boxMin, const float3 boxMax )
{
	float tmin = -15000.0f;
	float tmax =  15000.0f;
 
	float3 t1 = ( boxMin - rayOrigin ) / rayDir;
	float3 t2 = ( boxMax - rayOrigin ) / rayDir;
 
	tmin = max(tmin, min(t1.x, t2.x));
	tmax = min(tmax, max(t1.x, t2.x));
	tmin = max(tmin, min(t1.y, t2.y));
	tmax = min(tmax, max(t1.y, t2.y));
	tmin = max(tmin, min(t1.z, t2.z));
	tmax = min(tmax, max(t1.z, t2.z));

    return ( tmax >= tmin && tmax > 0.0f );
}

// Counts only intersections closer than ray length.
bool rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float rayLength, const float3 boxMin, const float3 boxMax )
{
	float tmin = -15000.0f;
	float tmax =  15000.0f;
 
	float3 t1 = ( boxMin - rayOrigin ) / rayDir;
	float3 t2 = ( boxMax - rayOrigin ) / rayDir;
 
	tmin = max(tmin, min(t1.x, t2.x));
	tmax = min(tmax, max(t1.x, t2.x));
	tmin = max(tmin, min(t1.y, t2.y));
	tmax = min(tmax, max(t1.y, t2.y));
	tmin = max(tmin, min(t1.z, t2.z));
	tmax = min(tmax, max(t1.z, t2.z));

    return ( tmax >= tmin && tmax > 0.0f && tmin < rayLength );
}

uint3 readTriangle( const uint index, ByteAddressBuffer meshTriangles ) 
{
    const uint address = index * 12; // 12 = 3 components * 4 bytes.

    return uint3(
        asuint( meshTriangles.Load( address ) ),
        asuint( meshTriangles.Load( address + 4 ) ),
        asuint( meshTriangles.Load( address + 8 ) ) 
    );
}

float3x3 readVerticesPos( const uint3 vertices_index, ByteAddressBuffer meshVertices ) 
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( meshVertices.Load( address.x ) ),
        asfloat( meshVertices.Load( address.x + 4 ) ),
        asfloat( meshVertices.Load( address.x + 8 ) ),
        // Vertex 2.
        asfloat( meshVertices.Load( address.y ) ),
        asfloat( meshVertices.Load( address.y + 4 ) ),
        asfloat( meshVertices.Load( address.y + 8 ) ),
        // Vertex 3.
        asfloat( meshVertices.Load( address.z ) ),
        asfloat( meshVertices.Load( address.z + 4 ) ),
        asfloat( meshVertices.Load( address.z + 8 ) )
    );
}

float3x3 readVerticesNormals( const uint3 vertices_index, ByteAddressBuffer meshNormals ) 
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( meshNormals.Load( address.x ) ),
        asfloat( meshNormals.Load( address.x + 4 ) ),
        asfloat( meshNormals.Load( address.x + 8 ) ),
        // Vertex 2.
        asfloat( meshNormals.Load( address.y ) ),
        asfloat( meshNormals.Load( address.y + 4 ) ),
        asfloat( meshNormals.Load( address.y + 8 ) ),
        // Vertex 3.
        asfloat( meshNormals.Load( address.z ) ),
        asfloat( meshNormals.Load( address.z + 4 ) ),
        asfloat( meshNormals.Load( address.z + 8 ) )
    );
}

float3x3 readVerticesTangents(const uint3 vertices_index, ByteAddressBuffer meshTangents )
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( meshTangents.Load(address.x)),
        asfloat( meshTangents.Load(address.x + 4)),
        asfloat( meshTangents.Load(address.x + 8)),
        // Vertex 2.
        asfloat( meshTangents.Load(address.y)),
        asfloat( meshTangents.Load(address.y + 4)),
        asfloat( meshTangents.Load(address.y + 8)),
        // Vertex 3.
        asfloat( meshTangents.Load(address.z)),
        asfloat( meshTangents.Load(address.z + 4)),
        asfloat( meshTangents.Load(address.z + 8))
    );
}

float2x3 readVerticesTexCoords( const uint3 vertices_index, ByteAddressBuffer meshTexcoords )
{
    const uint3 address = vertices_index * 8; // 8 = 2 components * 4 bytes.

    return float2x3(
        // row 0 - U coord - vertex 1, 2, 3.
        asfloat( meshTexcoords.Load(address.x)),
        asfloat( meshTexcoords.Load(address.y)),
        asfloat( meshTexcoords.Load(address.z)),
        // row 1 - V coord - vertex 1, 2, 3.
        asfloat( meshTexcoords.Load(address.x + 4)),
        asfloat( meshTexcoords.Load(address.y + 4)),
        asfloat( meshTexcoords.Load(address.z + 4))
    );
}

bool rayTriangleIntersect( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices )
{
    const float dot1 = dot( rayDir, cross( vertices[0] - rayOrigin, vertices[1] - rayOrigin ));
	const float dot2 = dot( rayDir, cross( vertices[1] - rayOrigin, vertices[2] - rayOrigin ));
	const float dot3 = dot( rayDir, cross( vertices[2] - rayOrigin, vertices[0] - rayOrigin ));

    // Without backface culling:
    //return ( ( dot1 < 0 && dot2 < 0 && dot3 < 0 ) || ( dot1 > 0 && dot2 > 0 && dot3 > 0 ) );

    // With backface culling:
    return (dot1 < 0 && dot2 < 0 && dot3 < 0);
}

float calcDistToTriangle( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices )
{
    const float3 trianglePlaneNormal   = normalize( cross( vertices[ 1 ] - vertices[ 0 ], vertices[ 2 ] - vertices[ 0 ] ));
	const float  trianglePlaneDistance = -dot( vertices[ 0 ], trianglePlaneNormal );
	
	const float rayTriangleDot    = dot( rayDir, trianglePlaneNormal );
	const float distFromRayOrigin = -( dot( trianglePlaneNormal, rayOrigin ) + trianglePlaneDistance ) / rayTriangleDot;

    return distFromRayOrigin;
}

float3 calcBarycentricCoordsInTriangle( const float3 p, const float3x3 vertices )
{
    float3 barycentricCoords;

    float3 edge0 = vertices[1] - vertices[0];
    float3 edge1 = vertices[2] - vertices[0];
    float3 edge2 = p - vertices[0];


    float d00 = dot( edge0, edge0 );
    float d01 = dot( edge0, edge1 );
    float d11 = dot( edge1, edge1 );
    float d20 = dot( edge2, edge0 );
    float d21 = dot( edge2, edge1 );
    float denom = d00 * d11 - d01 * d01;

    barycentricCoords.y = (d11 * d20 - d01 * d21) / denom;
    barycentricCoords.z = (d00 * d21 - d01 * d20) / denom;
    barycentricCoords.x = 1.0f - barycentricCoords.y - barycentricCoords.z;

    return barycentricCoords;
}

float3 calcInterpolatedVector(const float3 barycentricCoords, const float3x3 vectors)
{
    return float3(
        barycentricCoords.x * vectors[0] +
        barycentricCoords.y * vectors[1] +
        barycentricCoords.z * vectors[2]
    );
}

float2 calcInterpolatedTexCoords( const float3 barycentricCoords, const float2x3 verticesTexCoords )
{
    return float2( dot( barycentricCoords, verticesTexCoords[0] ), dot( barycentricCoords, verticesTexCoords[1] ) );
}

#endif