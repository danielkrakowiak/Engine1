#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 localToWorldMatrix;    // Transform from local to world space.
    float4x4 worldToLocalMatrix; // Transform from world to local space.
    float3   boundingBoxMin; // In local space.
    float    pad1;
    float3   boundingBoxMax; // In local space.
    float    pad2;
    float2   outputTextureSize;
    float2   pad3;
	float3   lightPosition;
	float    pad4;
};

// Input.
Texture2D<float4> g_rayOrigins       : register( t0 );
//Texture2D<float4> g_contributionTerm : register( t1 ); // How much of the ray color is visible by the camera. Used to avoid checking shadows for useless rays.
ByteAddressBuffer g_meshVertices     : register( t1 );
ByteAddressBuffer g_meshTexcoords    : register( t2 );
ByteAddressBuffer g_meshTriangles    : register( t3 );
Buffer<uint2>     g_bvhNodes         : register( t4 );
Buffer<float3>    g_bvhNodesExtents  : register( t5 ); // min, max, min, max interleaved.
Buffer<uint>      g_bvhTriangles     : register( t6 );

Texture2D         g_alphaTexture     : register( t7 );
SamplerState      g_samplerState;

// Input / Output.
RWTexture2D<uint>  g_illumination    : register( u0 );

bool     rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float3 boxMin, const float3 boxMax );
uint3    readTriangle( const uint index );
float3x3 readVerticesPos( const uint3 vertices_index );
float2x3 readVerticesTexCoords( const uint3 vertices_index );
bool     rayTriangleIntersect( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float    calcDistToTriangle( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float3   calcBarycentricCoordsInTriangle( const float3 p, const float3x3 vertices);
float3   calcInterpolatedVector(const float3 barycentricCoords, const float3x3 vectors);
float2   calcInterpolatedTexCoords( const float3 barycentricCoords, const float2x3 verticesTexCoords );

static const float requiredContributionTerm = 0.35f; // Discard rays which color is visible in less than 5% by the camera.

static const float minHitDist = 0.01f;

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
    const float2 texcoords = (float2)dispatchThreadId.xy / outputTextureSize;

	const float3 rayOrigin        = g_rayOrigins.SampleLevel( g_samplerState, texcoords, 0.0f ).xyz;
	//const float3 contributionTerm = g_contributionTerm.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

	// If all position components are zeros - ignore.
    if ( !any( rayOrigin ) /*|| dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm*/ ) { 
        return;
    }

    const float3 rayDir = normalize( lightPosition - rayOrigin );

	// #TODO: Discard ray with zero contribution.
    
    // Transform the ray from world to local space.
	const float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldToLocalMatrix ); //#TODO: ray origin could be passed in local space to avoid this calculation.
	const float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldToLocalMatrix ) - rayOriginLocal;

    // Test the ray against the bounding box
    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, boundingBoxMin, boundingBoxMax ) ) 
    {
        //output = float4( 0.0f, 0.5f, 0.2f, 1.0f );
		bool hit = false;

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

						//#TODO: Maybe could ignore calculating distance - it's only to avoid self-collision.
						if ( dist > minHitDist )
						{
							//#TODO: Could sample alpha texture here...
							//const float2x3 verticesTexCoords = readVerticesTexCoords(trianglee);
							//const float2   hitTexCoords      = calcInterpolatedTexCoords(hitBarycentricCoords, verticesTexCoords);

							g_illumination[ dispatchThreadId.xy ] = 50;
						}

                        break;
                    }
                }
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
    return ( ( dot1 < 0 && dot2 < 0 && dot3 < 0 ) || ( dot1 > 0 && dot2 > 0 && dot3 > 0 ) );

    // With backface culling:
    //return (dot1 < 0 && dot2 < 0 && dot3 < 0);
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

float3 calcInterpolatedVector( const float3 barycentricCoords, const float3x3 vectors )
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



