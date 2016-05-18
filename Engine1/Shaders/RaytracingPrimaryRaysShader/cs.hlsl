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
Texture2D<float4> g_rayDirections   : register( t0 );
ByteAddressBuffer g_meshVertices    : register( t1 );
ByteAddressBuffer g_meshNormals     : register( t2 );
ByteAddressBuffer g_meshTexcoords   : register( t3 );
ByteAddressBuffer g_meshTriangles   : register( t4 );
Buffer<uint2>     g_bvhNodes        : register( t5 );
Buffer<float3>    g_bvhNodesExtents : register( t6 ); // min, max, min, max interleaved.
Buffer<uint>      g_bvhTriangles    : register( t7 );

Texture2D         g_albedoTexture            : register( t8 );
Texture2D         g_normalTexture            : register( t9 );
Texture2D         g_metalnessTexture         : register( t10 );
Texture2D         g_roughnessTexture         : register( t11 );
Texture2D         g_indexOfRefractionTexture : register( t12 );
SamplerState      g_samplerState;

// Input / Output.
RWTexture2D<float>  g_hitDistance  : register( u0 );
// Output.
RWTexture2D<float4> g_hitPosition          : register( u1 );
RWTexture2D<float4> g_hitNormal            : register( u2 );
RWTexture2D<uint>   g_hitMetalness         : register( u3 );
RWTexture2D<uint>   g_hitRoughness         : register( u4 );
RWTexture2D<uint >  g_hitIndexOfRefraction : register( u5 );
RWTexture2D<uint4>  g_hitAlbedo            : register( u6 );

bool     rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float3 boxMin, const float3 boxMax );
uint3    readTriangle( const uint index );
float3x3 readVerticesPos( const uint3 vertices_index );
float3x3 readVerticesNormals( const uint3 vertices_index );
float2x3 readVerticesTexCoords( const uint3 vertices_index );
bool     rayTriangleIntersect( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float    calcDistToTriangle( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float3   calcBarycentricCoordsInTriangle( const float3 p, const float3x3 vertices);
float3   calcInterpolatedNormal( const float3 barycentricCoords, const float3x3 verticesNormals );
float2   calcInterpolatedTexCoords( const float3 barycentricCoords, const float2x3 verticesTexCoords );

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
    float3 rayDir = g_rayDirections[ dispatchThreadId.xy ].xyz;

    // Transform the ray from world to local space.
	float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldMatrixInv ); //#TODO: ray origin could be passed in local space to avoid this calculation.
	float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldMatrixInv ) - rayOriginLocal;

    //float4 output = float4( 0.2f, 0.2f, 0.2f, 1.0f );

    // Test the ray against the bounding box
    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, boundingBoxMin, boundingBoxMax ) ) 
    {
        //output = float4( 0.0f, 0.5f, 0.2f, 1.0f );

	    int      hitTriangle           = -1;
	    float    hitDist               = 20000.0f;
        float3   hitBarycentricCoords;
        float2   hitTexCoords; 
        float3   hitNormal;        

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

            uint2 bvhNodeData = g_bvhNodes[ bvhNodeIndex ];

		    // Determine if BVH node is an inner node or a leaf node by checking the highest bit.
		    // Inner node if highest bit is 0, leaf node if 1.
		    if ( !(bvhNodeData.x & 0x80000000) ) 
            { // Inner node.
		        // If ray intersects inner node, push indices of left and right child nodes on the stack.
			    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, g_bvhNodesExtents[ bvhNodeIndex * 2 ], g_bvhNodesExtents[ bvhNodeIndex * 2 + 1 ] )) {
				
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
                    const uint     triangleIdx = g_bvhTriangles[ i ];
				    const uint3    trianglee   = readTriangle( triangleIdx );
                    const float3x3 verticesPos = readVerticesPos( trianglee );

                    if ( rayTriangleIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos ) )
                    {
                        const float dist = calcDistToTriangle( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos );

                        if ( dist < hitDist )
                        {
                            hitDist     = dist;
                            hitTriangle = triangleIdx;

                            const float3 hitPos = rayOriginLocal.xyz + rayDirLocal.xyz * dist;
                            hitBarycentricCoords = calcBarycentricCoordsInTriangle( hitPos, verticesPos );

                            const float3x3 verticesNormals = readVerticesNormals( trianglee );
                            hitNormal = calcInterpolatedNormal( hitBarycentricCoords, verticesNormals );

                            const float2x3 verticesTexCoords = readVerticesTexCoords( trianglee );
                            hitTexCoords = calcInterpolatedTexCoords( hitBarycentricCoords, verticesTexCoords );

                        }

                        //break;
                    }
                }
            }
        }

        if ( hitTriangle != -1 )
        {
            // Write to output only if found hit is closer than the existing one at that pixel.
            if ( hitDist < g_hitDistance[ dispatchThreadId.xy ] ) 
            {
                g_hitPosition[ dispatchThreadId.xy ]          = float4( rayOrigin + rayDir * hitDist, 0.0f );
                g_hitDistance[ dispatchThreadId.xy ]          = hitDist;
                g_hitNormal[ dispatchThreadId.xy ]            = float4( hitNormal, 0.0f ); //TODO: use normal map as well.
                g_hitAlbedo[ dispatchThreadId.xy ]            = uint4( g_albedoTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ) * 255.0f );
                g_hitMetalness[ dispatchThreadId.xy ]         = uint( g_metalnessTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * 255.0f );   
                g_hitRoughness[ dispatchThreadId.xy ]         = uint( g_roughnessTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * 255.0f );    
                g_hitIndexOfRefraction[ dispatchThreadId.xy ] = uint( g_indexOfRefractionTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * 255.0f );   
            }
        }
    }
}

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

uint3 readTriangle( const uint index ) 
{
    const uint address = index * 12; // 12 = 3 components * 4 bytes.

    return uint3(
        asuint( g_meshTriangles.Load( address ) ),
        asuint( g_meshTriangles.Load( address + 4 ) ),
        asuint( g_meshTriangles.Load( address + 8 ) ) 
    );
}

float3x3 readVerticesPos( const uint3 vertices_index ) 
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( g_meshVertices.Load( address.x ) ),
        asfloat( g_meshVertices.Load( address.x + 4 ) ),
        asfloat( g_meshVertices.Load( address.x + 8 ) ),
        // Vertex 2.
        asfloat( g_meshVertices.Load( address.y ) ),
        asfloat( g_meshVertices.Load( address.y + 4 ) ),
        asfloat( g_meshVertices.Load( address.y + 8 ) ),
        // Vertex 3.
        asfloat( g_meshVertices.Load( address.z ) ),
        asfloat( g_meshVertices.Load( address.z + 4 ) ),
        asfloat( g_meshVertices.Load( address.z + 8 ) )
    );
}

float3x3 readVerticesNormals( const uint3 vertices_index ) 
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( g_meshNormals.Load( address.x ) ),
        asfloat( g_meshNormals.Load( address.x + 4 ) ),
        asfloat( g_meshNormals.Load( address.x + 8 ) ),
        // Vertex 2.
        asfloat( g_meshNormals.Load( address.y ) ),
        asfloat( g_meshNormals.Load( address.y + 4 ) ),
        asfloat( g_meshNormals.Load( address.y + 8 ) ),
        // Vertex 3.
        asfloat( g_meshNormals.Load( address.z ) ),
        asfloat( g_meshNormals.Load( address.z + 4 ) ),
        asfloat( g_meshNormals.Load( address.z + 8 ) )
    );
}

float2x3 readVerticesTexCoords( const uint3 vertices_index )
{
    const uint3 address = vertices_index * 8; // 8 = 2 components * 4 bytes.

    return float2x3(
        // row 0 - U coord - vertex 1, 2, 3.
        asfloat(g_meshTexcoords.Load(address.x)),
        asfloat(g_meshTexcoords.Load(address.y)),
        asfloat(g_meshTexcoords.Load(address.z)),
        // row 1 - V coord - vertex 1, 2, 3.
        asfloat(g_meshTexcoords.Load(address.x + 4)),
        asfloat(g_meshTexcoords.Load(address.y + 4)),
        asfloat(g_meshTexcoords.Load(address.z + 4))
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

float3 calcInterpolatedNormal( const float3 barycentricCoords, const float3x3 verticesNormals )
{
    return float3(
        barycentricCoords.x * verticesNormals[0] +
        barycentricCoords.y * verticesNormals[1] +
        barycentricCoords.z * verticesNormals[2]
    );
}

float2 calcInterpolatedTexCoords( const float3 barycentricCoords, const float2x3 verticesTexCoords )
{
    return float2( dot( barycentricCoords, verticesTexCoords[0] ), dot( barycentricCoords, verticesTexCoords[1] ) );
}



