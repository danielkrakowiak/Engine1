#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\RaytracingUtils.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 localToWorldMatrix; // Transform from local to world space.
    float4x4 worldToLocalMatrix; // Transform from world to local space.
    float4   boundingBoxMin; // In local space (4th component is padding).
    float4   boundingBoxMax; // In local space (4th component is padding).
    float4   isOpaque; // 0 - semi transparent, else - fully opaque (2nd, 3rd, 4th components are padding).
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
    float4   alphaMul; // (2nd, 3rd, 4th components are padding).
};

// Input.
Texture2D<float4> g_rayOrigins       : register( t0 );
Texture2D<float4> g_surfaceNormal    : register( t1 );
//Texture2D<float4> g_contributionTerm                  : register( t1 ); // How much of the ray color is visible by the camera. Used to avoid checking shadows for useless rays.
Texture2D<float>  g_preShadow        : register( t2 );
ByteAddressBuffer g_meshVertices     : register( t3 );
ByteAddressBuffer g_meshTexcoords    : register( t4 );
ByteAddressBuffer g_meshTriangles    : register( t5 );
Buffer<uint2>     g_bvhNodes         : register( t6 );
Buffer<float3>    g_bvhNodesExtents  : register( t7 ); // min, max, min, max interleaved.

Texture2D         g_alphaTexture     : register( t8 );

SamplerState      g_pointSamplerState     : register( s0 );
SamplerState      g_linearSamplerState    : register( s1 );

// Input / Output.
RWTexture2D<float> g_distToOccluder   : register( u0 );
RWTexture2D<uint>  g_hardShadow       : register( u1 ); // 1 - there is no illumination, 0 - full illumination.
RWTexture2D<uint>  g_softShadow       : register( u2 );

float    calculateShadowBlurRadius( const float lightEmitterRadius, const float distToOccluder, const float distLightToOccluder, const float distToCamera );
bool     rayMeshIntersect( const float3 rayOrigin, const float3 rayDir, const float maxAllowedHitDist, inout float nearestHitDist, inout float shadow );


static const float requiredContributionTerm = 0.35f; // Discard rays which color is visible in less than 5% by the camera.

static const float minAllowedHitDist = 0.001f;

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


    

    if ( rayMeshIntersect( rayOrigin, rayDir, rayMaxLength, nearestHitDist, shadow ) )
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

bool rayMeshIntersect( const float3 rayOrigin, const float3 rayDir, const float maxAllowedHitDist, inout float nearestHitDist, inout float shadow )
{
    // Offset to avoid self-collision. 
    // Note: Is it useful? We still need to check against the "origin triangle" and ignore by intersection distance... Or not?
    const float3 rayOriginOffset = rayDir * 0.001f; 

    // Transform the ray from world to local space.
	const float4 rayOriginLocal  = mul( float4( rayOrigin + rayOriginOffset, 1.0f ), worldToLocalMatrix );
	const float4 rayDirLocal     = mul( float4( rayOrigin + rayDir, 1.0f ), worldToLocalMatrix ) - rayOriginLocal;

     //#TODO: Should find all intersections - even those further from ray origin - to determine alpha...

    bool intersected = false;

	// Test the ray against the bounding box
	if ( rayBoxIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, maxAllowedHitDist, boundingBoxMin.xyz, boundingBoxMax.xyz ) ) 
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
					const uint3    trianglee   = readTriangle( triangleIdx, g_meshTriangles );
					const float3x3 verticesPos = readVerticesPos( trianglee, g_meshVertices );

					if ( rayTriangleIntersect( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos ) )
					{
						const float dist = calcDistToTriangle( rayOriginLocal.xyz, rayDirLocal.xyz, verticesPos );

						//#TODO: Maybe could ignore calculating distance - it's only to avoid self-collision (or not.. - triangles behind the ray origin).
						if ( dist > minAllowedHitDist && dist < maxAllowedHitDist )
						{
                            intersected = true;
                            nearestHitDist = min( nearestHitDist, dist );

                            if ( isOpaque.x == 0.0f )
                            {
                                const float3 hitPos               = rayOriginLocal.xyz + rayDirLocal.xyz * dist;
                                const float3 hitBarycentricCoords = calcBarycentricCoordsInTriangle( hitPos, verticesPos );

							    const float2x3 verticesTexCoords = readVerticesTexCoords( trianglee, g_meshTexcoords );
							    const float2   hitTexCoords      = calcInterpolatedTexCoords( hitBarycentricCoords, verticesTexCoords );

                                const float alpha = g_alphaTexture.SampleLevel( g_linearSamplerState, hitTexCoords, 0.0f ).r * alphaMul.r;

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