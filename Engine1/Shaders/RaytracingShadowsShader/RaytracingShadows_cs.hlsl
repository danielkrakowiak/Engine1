#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#define MAX_ACTOR_COUNT 1

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 localToWorldMatrix[ MAX_ACTOR_COUNT ]; // Transform from local to world space.
    float4x4 worldToLocalMatrix[ MAX_ACTOR_COUNT ]; // Transform from world to local space.
    float4   boundingBoxMin[ MAX_ACTOR_COUNT ]; // In local space (4th component is padding).
    float4   boundingBoxMax[ MAX_ACTOR_COUNT ]; // In local space (4th component is padding).
    float4   isOpaque[ MAX_ACTOR_COUNT ]; // 0 - semi transparent, else - fully opaque (2nd, 3rd, 4th components are padding).
    uint     actorCount;
    float3   pad1;
    float2   outputTextureSize;
    float2   pad2;
	float3   lightPosition;
	float    pad3;
    float    lightConeMinDot;
    float3   pad4;
    float3   lightDirection;
    float    pad5;
    float    lightEmitterRadius;
    float3   pad6;
    uint     isPreShadowAvailable; // 1 - available, 0 - not available.
    float3   pad7;
    float4x4 shadowMapViewMatrix;
    float4x4 shadowMapProjectionMatrix;
    float3   cameraPosition;
    float    pad8;
};

// Input.
Texture2D<float4> g_rayOrigins                          : register( t0 );
Texture2D<float4> g_surfaceNormal                       : register( t1 );
//Texture2D<float4> g_contributionTerm                  : register( t1 ); // How much of the ray color is visible by the camera. Used to avoid checking shadows for useless rays.
Texture2D<float>  g_preShadow                     : register( t2 );
ByteAddressBuffer g_meshVertices[ MAX_ACTOR_COUNT ]     : register( t3 );
ByteAddressBuffer g_meshTexcoords[ MAX_ACTOR_COUNT ]    : register( t4 );
ByteAddressBuffer g_meshTriangles[ MAX_ACTOR_COUNT ]    : register( t5 );
Buffer<uint2>     g_bvhNodes[ MAX_ACTOR_COUNT ]         : register( t6 );
Buffer<float3>    g_bvhNodesExtents[ MAX_ACTOR_COUNT ]  : register( t7 ); // min, max, min, max interleaved.

Texture2D         g_alphaTexture[ MAX_ACTOR_COUNT ]     : register( t8 );

SamplerState      g_pointSamplerState     : register( s0 );
SamplerState      g_linearSamplerState    : register( s1 );

// Input / Output.
RWTexture2D<float> g_distToOccluder   : register( u0 );
RWTexture2D<uint>  g_hardShadow       : register( u1 ); // 1 - there is no illumination, 0 - full illumination.
RWTexture2D<uint>  g_softShadow       : register( u2 );

float    calculateShadowBlurRadius( const float lightEmitterRadius, const float distToOccluder, const float distLightToOccluder, const float distToCamera );
bool     rayMeshIntersect( const float3 rayOrigin, const float3 rayDir, const int actorIdx, const float maxAllowedHitDist, inout float nearestHitDist, inout float shadow );
bool     rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float rayLength, const float3 boxMin, const float3 boxMax );
bool     rayBoxIntersect( const float3 rayOrigin, const float3 rayDir, const float3 boxMin, const float3 boxMax );
uint3    readTriangle( const uint actorIdx, const uint index );
float3x3 readVerticesPos( const uint actorIdx, const uint3 vertices_index );
float2x3 readVerticesTexCoords( const uint actorIdx, const uint3 vertices_index );
bool     rayTriangleIntersect( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float    calcDistToTriangle( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices );
float3   calcBarycentricCoordsInTriangle( const float3 p, const float3x3 vertices);
float3   calcInterpolatedVector(const float3 barycentricCoords, const float3x3 vectors);
float2   calcInterpolatedTexCoords( const float3 barycentricCoords, const float2x3 verticesTexCoords );

static const float requiredContributionTerm = 0.35f; // Discard rays which color is visible in less than 5% by the camera.

static const float minAllowedHitDist = 0.001f;

static const float Pi = 3.14159265f;

static const float maxShadowWorldSpaceBlurRadius = 1.0f;

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
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

	const float3 rayOrigin = g_rayOrigins.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
	const float3 rayDir    = normalize( lightPosition - rayOrigin );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( lightDirection, -rayDir ) < lightConeMinDot ) {
        // Preserve illumination from shadow map. 
        // #TODO: It may be needed to raytrace shadows slightly outiside of spot light cone to ensure proper blurring of soft shadows.
        //g_illumination[ dispatchThreadId.xy ] = (uint)(g_preIllumination[ dispatchThreadId.xy ] * 255.0f);
        return;
    }

    // TEMPORARILY RAY TRACE FOR ALL PIXELS - BECAUSE SHADOW MAP KNOWS ONLY THE DIST TO THE NEAREST TO LIGHT OCCLUDER.

    // Check if raytracing shadows is needed for this pixel.
    //const float preillumination = g_preIllumination.SampleLevel( g_linearSamplerState, texcoords, 3.0f );
    //if ( preillumination < 0.01f || preillumination > 0.99f )
    //{
        // This pixel and its neighbors are entirely in shadow or entirely lit - don't raytrace.

        //#TODO: This write should be avoided. There is not point to write the same copied value each time this shader is run...
        // This could be done in the first pass. Just copying values from preillumination texture. Or preillumination could be copied entirely using a copy drawcall.

        //g_illumination[ dispatchThreadId.xy ] = (uint)(g_preIllumination[ dispatchThreadId.xy ] * 255.0f);
        //return;
    //}
    //else
    //{
        // Raytrace!!!
        //g_illumination[ dispatchThreadId.xy ] = 128;
        //return;
    //}

    /////////////////////////////////////

	const float3 surfaceNormal  = g_surfaceNormal.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
    const float  normalLightDot = dot( surfaceNormal, rayDir );

	// #TODO: Discard ray with zero contribution.

    //const uint illuminationUint = g_illumination[ dispatchThreadId.xy ];

    // IMPORTANT: Cannot quit early when pixel is already in shadow, because we need to know if a mesh changed blur radius at that pixel!!!!
    // Or maybe we can rely on blur radius from shadow map?
    
	// If all position components are zeros - ignore. If face is backfacing the light - ignore (shading will take care of that case). Already in shadow - ignore.
    if ( !any( rayOrigin ) || normalLightDot < 0.0f /*|| illuminationUint == 0*//*|| dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm*/ ) 
    { 
        g_hardShadow[ dispatchThreadId.xy ] = 255;
        g_softShadow[ dispatchThreadId.xy ] = 255;
        return;
    }

    const float rayMaxLength = length( lightPosition - rayOrigin );

    // #TODO: Reading unsigned char from UAV is supported only on DirectX 11.3. 
	float shadow = 0.0f;//(float)illuminationUint / 255.0f;
    
    // We only care about intersections closer to ray origin than the light source. #TODO: Or all of them? - alpha etc, illumination?
    float nearestHitDist = 10000.0f;

    // #OPTIMIZATION: Could be more effective to trace ray from light to surface - because we care about the nearest to light intersection. Easier to skip other objects. 


    

    if ( rayMeshIntersect( rayOrigin, rayDir, 0, rayMaxLength, nearestHitDist, shadow ) )
    {
        const float surfaceDistToLight    = rayMaxLength;
        const float surfaceDistToOccluder = nearestHitDist;
        const float occluderDistToLight   = surfaceDistToLight - surfaceDistToOccluder;
        const float surfaceDistToCamera   = length( rayOrigin - cameraPosition );
        
        const float blurRadius = calculateShadowBlurRadius( lightEmitterRadius, surfaceDistToOccluder, occluderDistToLight, surfaceDistToCamera );

        const float prevDistToOccluder = g_distToOccluder[ dispatchThreadId.xy ];

        const float prevHardShadow = (float)g_hardShadow[ dispatchThreadId.xy ] / 255.0f;
        const float prevSoftShadow = (float)g_softShadow[ dispatchThreadId.xy ] / 255.0f;

        const float shadowSoftness = min(1.0f, (blurRadius / 1.0f));
        const float shadowHardness = 1.0f - shadowSoftness;

        shadow = min( 1.0f, shadow ); // Needed? Can shadow go over 1 after many intersections?

        g_distToOccluder[ dispatchThreadId.xy ] = min( prevDistToOccluder, surfaceDistToOccluder );
        g_hardShadow[ dispatchThreadId.xy ]     = (uint)( min(1.0f, prevHardShadow + shadow * shadowHardness ) * 255.0f );
        g_softShadow[ dispatchThreadId.xy ]     = (uint)( min(1.0f, prevSoftShadow + shadow * shadowSoftness ) * 255.0f );
    }
}

float calculateShadowBlurRadius( const float lightEmitterRadius, const float distToOccluder, const float distLightToOccluder, const float distToCamera )
{
    const float blurRadiusInWorldSpace = min( maxShadowWorldSpaceBlurRadius, lightEmitterRadius * ( distToOccluder / distLightToOccluder ) );
    //const float blurRadius     = baseBlurRadius / log2( distToCamera + 1.0f );

    //#TODO: Blur radius may have different resolution in the future? Using outputTextureSize may not be safe.
    //const float pixelSizeInWorldSpace = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);
    //const float blurRadiusInScreenSpace = blurRadiusInWorldSpace / pixelSizeInWorldSpace;

    return blurRadiusInWorldSpace;
}

bool rayMeshIntersect( const float3 rayOrigin, const float3 rayDir, const int actorIdx, const float maxAllowedHitDist, inout float nearestHitDist, inout float shadow )
{
    // Offset to avoid self-collision. 
    // Note: Is it useful? We still need to check against the "origin triangle" and ignore by intersection distance... Or not?
    const float3 rayOriginOffset = rayDir * 0.001f; 

    // Transform the ray from world to local space.
	const float4 rayOriginLocal  = mul( float4( rayOrigin + rayOriginOffset, 1.0f ), worldToLocalMatrix[ actorIdx ] );
	const float4 rayDirLocal     = mul( float4( rayOrigin + rayDir, 1.0f ), worldToLocalMatrix[ actorIdx ] ) - rayOriginLocal;

     //#TODO: Should find all intersections - even those further from ray origin - to determine alpha...

    bool intersected = false;

	// Test the ray against the bounding box
	if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, maxAllowedHitDist, boundingBoxMin[ actorIdx ].xyz, boundingBoxMax[ actorIdx ].xyz ) ) 
	{
		// TODO: Size of the stack could be passed as argument in constant buffer?
		const uint BVH_STACK_SIZE = 32; 

		// Stack of BVH nodes (as indices) which were visited or will be visited.
		uint bvhStack[ BVH_STACK_SIZE ];
		uint bvhStackIndex = 0;

		// Push root node on the stack.
		bvhStack[ bvhStackIndex++ ] = 0; 

		// While the stack is not empty.
		while ( bvhStackIndex > 0 ) 
        {
			// Pop a node from the stack.
			int bvhNodeIndex = bvhStack[ --bvhStackIndex ];

			uint2 bvhNodeData = g_bvhNodes[ actorIdx ][ bvhNodeIndex ];

			// Determine if BVH node is an inner node or a leaf node by checking the highest bit.
			// Inner node if highest bit is 0, leaf node if 1.
			if ( !(bvhNodeData.x & 0x80000000) ) 
			{ // Inner node.
				// If ray intersects inner node, push indices of left and right child nodes on the stack.
				if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, g_bvhNodesExtents[ actorIdx ][ bvhNodeIndex * 2 ], g_bvhNodesExtents[ actorIdx ][ bvhNodeIndex * 2 + 1 ] )) {
				
					bvhStack[ bvhStackIndex++ ] = bvhNodeData.x; // Left child node index.
					bvhStack[ bvhStackIndex++ ] = bvhNodeData.y; // Right child node index.
				
					// Return if stack size is exceeded. 
					//TODO: Maybe we can remove that check and check it before running the shader.
					if ( bvhStackIndex > BVH_STACK_SIZE )
						return intersected; 
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
					const uint3    trianglee   = readTriangle( actorIdx, triangleIdx );
					const float3x3 verticesPos = readVerticesPos( actorIdx, trianglee );

					if ( rayTriangleIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos ) )
					{
						const float dist = calcDistToTriangle( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos );

						//#TODO: Maybe could ignore calculating distance - it's only to avoid self-collision (or not.. - triangles behind the ray origin).
						if ( dist > minAllowedHitDist && dist < maxAllowedHitDist )
						{
                            intersected = true;
                            nearestHitDist = min( nearestHitDist, dist );

                            if ( isOpaque[ actorIdx ].x == 0.0f )
                            {
                                const float3 hitPos               = rayOriginLocal.xyz + rayDirLocal.xyz * dist;
                                const float3 hitBarycentricCoords = calcBarycentricCoordsInTriangle( hitPos, verticesPos );

							    const float2x3 verticesTexCoords = readVerticesTexCoords( actorIdx, trianglee );
							    const float2   hitTexCoords      = calcInterpolatedTexCoords( hitBarycentricCoords, verticesTexCoords );

                                const float alpha = g_alphaTexture[ actorIdx ].SampleLevel( g_linearSamplerState, hitTexCoords, 0.0f ).r;

                                shadow += alpha;

                                // Note: We don't stop tracing rays now, because we need to find the closest intersection for soft shadow calculation.

                                // Stop tracing shadow rays if the pixel is already fully shadowed.
                                //if ( illumination < 0.001f )
                                //    break;
                            }
                            else
                            {
                                shadow = 1.0f;
                                //break;
							    //illumination -= lightAmountPerSample;

                                // Note: We don't stop tracing rays now, because we need to find the closest intersection for soft shadow calculation.
                                //break;
                            }
						}
					}
				}
			}
		}
	}

    return intersected;
}

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

    return ( tmax > 0.0f && tmax >= tmin && tmin < rayLength );
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

    return ( tmax > 0.0f && tmax >= tmin );
}

uint3 readTriangle( const uint actorIdx, const uint index ) 
{
    const uint address = index * 12; // 12 = 3 components * 4 bytes.

    return uint3(
        asuint( g_meshTriangles[ actorIdx ].Load( address ) ),
        asuint( g_meshTriangles[ actorIdx ].Load( address + 4 ) ),
        asuint( g_meshTriangles[ actorIdx ].Load( address + 8 ) ) 
    );
}

float3x3 readVerticesPos( const uint actorIdx, const uint3 vertices_index ) 
{
    const uint3 address = vertices_index * 12; // 12 = 3 components * 4 bytes.

    return float3x3(
        // Vertex 1.
        asfloat( g_meshVertices[ actorIdx ].Load( address.x ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.x + 4 ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.x + 8 ) ),
        // Vertex 2.
        asfloat( g_meshVertices[ actorIdx ].Load( address.y ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.y + 4 ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.y + 8 ) ),
        // Vertex 3.
        asfloat( g_meshVertices[ actorIdx ].Load( address.z ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.z + 4 ) ),
        asfloat( g_meshVertices[ actorIdx ].Load( address.z + 8 ) )
    );
}

float2x3 readVerticesTexCoords( const uint actorIdx, const uint3 vertices_index )
{
    const uint3 address = vertices_index * 8; // 8 = 2 components * 4 bytes.

    return float2x3(
        // row 0 - U coord - vertex 1, 2, 3.
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.x) ),
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.y) ),
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.z) ),
        // row 1 - V coord - vertex 1, 2, 3.
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.x + 4) ),
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.y + 4) ),
        asfloat( g_meshTexcoords[ actorIdx ].Load(address.z + 4) )
    );
}

bool rayTriangleIntersect( const float3 rayOrigin, const float3 rayDir, const float3x3 vertices )
{
    const float dot1 = dot( rayDir, cross( vertices[0] - rayOrigin, vertices[1] - rayOrigin ));
	const float dot2 = dot( rayDir, cross( vertices[1] - rayOrigin, vertices[2] - rayOrigin ));
	const float dot3 = dot( rayDir, cross( vertices[2] - rayOrigin, vertices[0] - rayOrigin ));

    // TODO OPTIMIZATION: Disabling backface culling is required only for meshes which have holes or are flat surfaces 
    // (even flat surfaces can have two quads in the same place). Maybe there should be a flag for that or a special shader version?
    // Or maybe such meshes should be forbidden..
    // But it doesn't make a difference when profiling... Maybe because we are memory bound. Worth checking from time to time..

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



