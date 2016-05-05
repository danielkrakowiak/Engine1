#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float3   rayOrigin;      // Ray origin in world space.
    float    pad1;
    float4x4 worldMatrixInv; // Transform from world to local space.
    float3   boundingBoxMin; // In local space.
    float    pad2;
    float3   boundingBoxMax; // In local space.
    float    pad3;
};

// Input.
Texture2D<float4> rayDirections   : register( t0 );
ByteAddressBuffer meshVertices    : register( t1 );
//Buffer<float3>    meshNormals   : register( t2 );
//Buffer<float2>    meshTexcoords : register( t3 );
ByteAddressBuffer meshTriangles   : register( t2 );
Buffer<uint2>     bvhNodes        : register( t3 );
Buffer<float3>    bvhNodesExtents : register( t4 ); // min, max, min, max interleaved.
Buffer<uint>      bvhTriangles    : register( t5 );
;
// Input / Output.
RWTexture2D<float>  hitDistance       : register( u0 );
// Output.
RWTexture2D<float2> barycentricCoords : register( u1 );

bool     rayBoxIntersect( float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax );
uint3    getTriangle( uint index );
float3x3 getVertices( uint3 index );
bool     rayTriangleIntersect( float3 rayOrigin, float3 rayDir, float3x3 vertices );
float    calcDistToTriangle( float3 rayOrigin, float3 rayDir, float3x3 vertices );
float3   calcBarycentricCoordsInTriangle(float3 p, float3x3 vertices);

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(16, 16, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    float3 rayDir = rayDirections.Load( int3( dispatchThreadId.xy, 0 ) ).xyz;

    // Transform the ray from world to local space.
	float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldMatrixInv ); //#TODO: ray origin could be passed in local space to avoid this calculation.
	float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldMatrixInv ) - rayOriginLocal;

    //float4 output = float4( 0.2f, 0.2f, 0.2f, 1.0f );

    // Test the ray against the bounding box
    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, boundingBoxMin, boundingBoxMax ) ) 
    {
        //output = float4( 0.0f, 0.5f, 0.2f, 1.0f );

	    int    hitTriangle          = -1;
	    float  hitDist              = 20000.0f;
        float3 hitBarycentricCoords = float3(0.0f, 0.0f, 0.0f);

        // TODO: Size of the stack could be passed as argument in constant buffer?
        const uint BVH_STACK_SIZE = 32; 

	    // Stack of BVH nodes (as indices) which were visited or will be visited.
	    uint bvhStack[ BVH_STACK_SIZE ];
	    uint bvhStackIndex = 0;

        // Push root node on the stack.
	    bvhStack[ bvhStackIndex++ ] = 0; 

        // While the stack is not empty.
	    while ( bvhStackIndex > 0 ) {
		
		    // Pop a node from the stack.
		    int bvhNodeIndex = bvhStack[ --bvhStackIndex ];

            uint2 bvhNodeData = bvhNodes[ bvhNodeIndex ];

		    // Determine if BVH node is an inner node or a leaf node by checking the highest bit.
		    // Inner node if highest bit is 0, leaf node if 1.
		    if ( !(bvhNodeData.x & 0x80000000) ) 
            { // Inner node.
		        // If ray intersects inner node, push indices of left and right child nodes on the stack.
			    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, bvhNodesExtents[ bvhNodeIndex * 2 ], bvhNodesExtents[ bvhNodeIndex * 2 + 1 ] )) {
				
				    bvhStack[ bvhStackIndex++ ] = bvhNodeData.x; // Left child node index.
				    bvhStack[ bvhStackIndex++ ] = bvhNodeData.y; // Right child node index.
				
				    // Return if stack size is exceeded. 
                    //TODO: Maybe we can remove that check and check it before running the shader.
				    if ( bvhStackIndex > BVH_STACK_SIZE )
					    return; 
			    }
            }
            else
            { // Leaf node.

                // Loop over every triangle in the leaf node.
			    // bvhNodeData.x - triangle count (and top-bit set to 1 to indicate leaf node).
			    // bvhNodeData.y - first triangles index.
                const uint triangleCount      = bvhNodeData.x & 0x7fffffff;
                const uint firstTriangleIndex = bvhNodeData.y;
                const uint lastTriangleIndex  = firstTriangleIndex + triangleCount;
			    for ( uint i = firstTriangleIndex; i < lastTriangleIndex; ++i ) 
                {
                    const uint     triangleIdx = bvhTriangles[ i ];
				    const uint3    trianglee   = getTriangle( triangleIdx );
                    const float3x3 vertices    = getVertices( trianglee );

                    if ( rayTriangleIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, vertices ) )
                    {
                        const float dist = calcDistToTriangle( rayOriginLocal.xyz, rayDirLocal.xyz, vertices );

                        if ( dist < hitDist )
                        {
                            hitDist     = dist;
                            hitTriangle = triangleIdx;

                            const float3 hitPos = rayOriginLocal.xyz + rayDirLocal.xyz * dist;
                            hitBarycentricCoords = calcBarycentricCoordsInTriangle( hitPos, vertices );
                        }

                        //break;
                    }
                }
            }
        }

        if ( hitTriangle != -1 )
        {
            // Write to output only if found hit is closer than the existing one at that pixel.
            if ( hitDist < hitDistance[ dispatchThreadId.xy] ) {
                hitDistance[ dispatchThreadId.xy ]     = hitDist;
                barycentricCoords[dispatchThreadId.xy] = hitBarycentricCoords.xy;
            }
        }
    }
}

bool rayBoxIntersect( float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax )
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

uint3 getTriangle( uint index ) 
{
    const uint address = index * 12; // 12 = 3 components * 4 bytes.

    return uint3(
        asuint( meshTriangles.Load( address ) ),
        asuint( meshTriangles.Load( address + 4 ) ),
        asuint( meshTriangles.Load( address + 8 ) ) 
    );
}

float3x3 getVertices( uint3 index ) 
{
    const uint3 address = index * 12; // 12 = 3 components * 4 bytes.

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

bool rayTriangleIntersect( float3 rayOrigin, float3 rayDir, float3x3 vertices )
{
    const float dot1 = dot( rayDir, cross( vertices[0] - rayOrigin, vertices[1] - rayOrigin ));
	const float dot2 = dot( rayDir, cross( vertices[1] - rayOrigin, vertices[2] - rayOrigin ));
	const float dot3 = dot( rayDir, cross( vertices[2] - rayOrigin, vertices[0] - rayOrigin ));

    // Without backface culling:
    //return ( ( dot1 < 0 && dot2 < 0 && dot3 < 0 ) || ( dot1 > 0 && dot2 > 0 && dot3 > 0 ) );

    // With backface culling:
    return (dot1 < 0 && dot2 < 0 && dot3 < 0);
}

float calcDistToTriangle( float3 rayOrigin, float3 rayDir, float3x3 vertices )
{
    const float3 trianglePlaneNormal   = normalize( cross( vertices[ 1 ] - vertices[ 0 ], vertices[ 2 ] - vertices[ 0 ] ));
	const float  trianglePlaneDistance = -dot( vertices[ 0 ], trianglePlaneNormal );
	
	const float rayTriangleDot    = dot( rayDir, trianglePlaneNormal );
	const float distFromRayOrigin = -( dot( trianglePlaneNormal, rayOrigin ) + trianglePlaneDistance ) / rayTriangleDot;

    return distFromRayOrigin;
}

float3 calcBarycentricCoordsInTriangle( float3 p, float3x3 vertices )
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



