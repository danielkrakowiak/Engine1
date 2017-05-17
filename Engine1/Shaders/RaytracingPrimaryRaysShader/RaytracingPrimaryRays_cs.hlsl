#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\RaytracingUtils.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3   rayOrigin;      // Ray origin in world space.
    float    pad1;
    float4x4 localToWorldMatrix; // Transform from local to world space.
    float4x4 worldToLocalMatrix; // Transform from world to local space.
    float3   boundingBoxMin; // In local space.
    float    pad2;
    float3   boundingBoxMax; // In local space.
    float    pad3;
    float    alphaMul;
    float3   pad4;
    float3   emissiveMul;
    float    pad5;
    float3   albedoMul;
    float    pad6;
    float3   normalMul;
    float    pad7;
    float    metalnessMul;
    float3   pad8;
    float    roughnessMul;
    float3   pad9;
    float    indexOfRefractionMul;
    float3   pad10;
};

// Input.
Texture2D<float4> g_rayDirections   : register( t0 );
ByteAddressBuffer g_meshVertices    : register( t1 );
ByteAddressBuffer g_meshNormals     : register( t2 );
ByteAddressBuffer g_meshTangents    : register( t3 );
ByteAddressBuffer g_meshTexcoords   : register( t4 );
ByteAddressBuffer g_meshTriangles   : register( t5 );
Buffer<uint2>     g_bvhNodes        : register( t6 );
Buffer<float3>    g_bvhNodesExtents : register( t7 ); // min, max, min, max interleaved.

Texture2D         g_emissiveTexture          : register( t8 );
Texture2D         g_albedoTexture            : register( t9 );
Texture2D         g_normalTexture            : register( t10 );
Texture2D         g_metalnessTexture         : register( t11 );
Texture2D         g_roughnessTexture         : register( t12 );
Texture2D         g_indexOfRefractionTexture : register( t13 );
SamplerState      g_samplerState;

// Input / Output.
RWTexture2D<float>  g_hitDistance  : register( u0 );
// Output.
RWTexture2D<float4> g_hitPosition          : register( u1 );
RWTexture2D<float4> g_hitNormal            : register( u2 );
RWTexture2D<uint>   g_hitMetalness         : register( u3 );
RWTexture2D<uint>   g_hitRoughness         : register( u4 );
RWTexture2D<uint >  g_hitIndexOfRefraction : register( u5 );
RWTexture2D<uint4>  g_hitEmissive          : register( u6 );
RWTexture2D<uint4>  g_hitAlbedo            : register( u7 );

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
	float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldToLocalMatrix ); //#TODO: ray origin could be passed in local space to avoid this calculation.
	float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldToLocalMatrix ) - rayOriginLocal;

    //float4 output = float4( 0.2f, 0.2f, 0.2f, 1.0f );

    // Test the ray against the bounding box
    if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, boundingBoxMin, boundingBoxMax ) ) 
    {
        //output = float4( 0.0f, 0.5f, 0.2f, 1.0f );

	    int      hitTriangle           = -1;
	    float    hitDist               = 20000.0f;

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
			    for ( uint triangleIdx = firstTriangleIndex; triangleIdx < lastTriangleIndex; ++triangleIdx ) 
                {
				    const uint3    trianglee   = readTriangle( triangleIdx, g_meshTriangles );
                    const float3x3 verticesPos = readVerticesPos( trianglee, g_meshVertices );

                    if ( rayTriangleIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos ) )
                    {
                        const float dist = calcDistToTriangle( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos );

                        if ( dist < hitDist )
                        {
                            hitDist     = dist;
                            hitTriangle = triangleIdx;
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
                 // Read triangle data.
                const uint3    trianglee   = readTriangle( hitTriangle, g_meshTriangles );
                const float3x3 verticesPos = readVerticesPos( trianglee, g_meshVertices );

                const float3 hitPos = rayOriginLocal.xyz + rayDirLocal.xyz * hitDist;
                const float3 hitBarycentricCoords = calcBarycentricCoordsInTriangle( hitPos, verticesPos );

                const float3x3 verticesNormals = readVerticesNormals( trianglee, g_meshNormals );
                float3         hitNormal       = calcInterpolatedVector( hitBarycentricCoords, verticesNormals );

                const float3x3 verticesTangents = readVerticesTangents( trianglee, g_meshTangents );
                float3         hitTangent       = calcInterpolatedVector( hitBarycentricCoords, verticesTangents );

                // Transform normal and tangent from local to world space.
                hitNormal  = mul( float4( hitNormal, 0.0f ), localToWorldMatrix ).xyz;
                hitTangent = mul( float4( hitTangent, 0.0f ), localToWorldMatrix ).xyz;

                const float3 hitBitangent = cross( hitTangent, hitNormal );

                const float2x3 verticesTexCoords = readVerticesTexCoords( trianglee, g_meshTexcoords );
                const float2   hitTexCoords      = calcInterpolatedTexCoords( hitBarycentricCoords, verticesTexCoords );

                float3x3 tangentToWorldMatrix = float3x3(
                    normalize(hitTangent),
                    normalize(hitBitangent),
                    normalize(hitNormal)
                );
			
                const float3 normalFromMap = (g_normalTexture.SampleLevel(g_samplerState, hitTexCoords, 0.0f).rgb * normalMul - 0.5f) * 2.0f;
                hitNormal = normalize(mul(normalFromMap, tangentToWorldMatrix));

                g_hitPosition[ dispatchThreadId.xy ]          = float4( rayOrigin + rayDir * hitDist, 0.0f );
                g_hitDistance[ dispatchThreadId.xy ]          = hitDist;
                g_hitNormal[ dispatchThreadId.xy ]            = float4( hitNormal, 0.0f ); //TODO: use normal map as well.
                g_hitEmissive[ dispatchThreadId.xy ]          = uint4( g_emissiveTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).rgb * emissiveMul * 255.0f, 0 );
                g_hitAlbedo[ dispatchThreadId.xy ]            = uint4( g_albedoTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).rgb * albedoMul * 255.0f, 0 );
                g_hitMetalness[ dispatchThreadId.xy ]         = uint( g_metalnessTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * metalnessMul * 255.0f );   
                g_hitRoughness[ dispatchThreadId.xy ]         = uint( g_roughnessTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * roughnessMul * 255.0f );    
                g_hitIndexOfRefraction[ dispatchThreadId.xy ] = uint( g_indexOfRefractionTexture.SampleLevel( g_samplerState, hitTexCoords, 0.0f ).r * indexOfRefractionMul * 255.0f );   
            }
        }
    }
}



