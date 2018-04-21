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
	float3   lightCenterPosition;
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
    float4   alphaMul; // (2nd, 3rd, 4th components are padding).
    float3   cameraPos;
    float    pad8;
    float    shadowHardBlurRadiusStartThreshold;
    float3   pad9;
    float    shadowHardBlurRadiusEndThreshold;
    float3   pad10;
    float    shadowSoftBlurRadiusStartThreshold;
    float3   pad11;
    float    shadowSoftBlurRadiusEndThreshold;
    float3   pad12;
    float    shadowHardBlurRadiusTransitionWidth;
    float3   pad13;
    float    shadowSoftBlurRadiusTransitionWidth;
    float3   pad14;
    float    distToOccluderHardBlurRadiusStartThreshold;
    float3   pad15;
    float    distToOccluderHardBlurRadiusEndThreshold;
    float3   pad16;
    float    distToOccluderSoftBlurRadiusStartThreshold;
    float3   pad17;
    float    distToOccluderSoftBlurRadiusEndThreshold;
    float3   pad18;
    float    enableAlteringRayDirection; // If enabled, rays at different pixels aim at different parts of area light. 1 - enabled, 0 - disabled.
    float3   pad19;
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
RWTexture2D<float> g_distToOccluderHardShadow   : register( u0 );
RWTexture2D<float> g_distToOccluderMediumShadow : register( u1 );
RWTexture2D<float> g_distToOccluderSoftShadow   : register( u2 );
RWTexture2D<uint>  g_hardShadow                 : register( u3 ); // 1 - there is no illumination, 0 - full illumination.
RWTexture2D<uint>  g_mediumShadow               : register( u4 );
RWTexture2D<uint>  g_softShadow                 : register( u5 );

float    calculateShadowBlurRadius( const float lightEmitterRadius, const float distToOccluder, const float distLightToOccluder );
bool     rayMeshIntersect( const float3 rayOrigin, const float3 rayDir, const float maxAllowedHitDist, inout float nearestHitDist, inout float shadow );


static const float requiredContributionTerm = 0.35f; // Discard rays which color is visible in less than 5% by the camera.

static const float minAllowedHitDist = 0.0005f;

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

	const float3 rayOrigin                = g_rayOrigins.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
	const float3 rayDirTowardsLightCenter = normalize( lightCenterPosition - rayOrigin );

    const float3 worldUpDir      = float3(0.0, 1.0, 0.0);
    const float3 lightForwardDir = -rayDirTowardsLightCenter;
    const float3 lightSideDir    = cross( lightForwardDir, worldUpDir );
    const float3 lightUpDir      = cross( lightSideDir, lightForwardDir );

    float3 lightPosition = lightCenterPosition;

    if (enableAlteringRayDirection > 0.5)
    {
        const float threadHorzSeed = (float)dispatchThreadId.x + (float)dispatchThreadId.y * 0.5;
        const float threadVertSeed = (float)dispatchThreadId.x * 0.5 + (float)dispatchThreadId.y;

        // Note: More samples are better, because the transition between full shadow and full light is smoother,
        // but larger sample count means that shadow fluctuations appear over larger distances - making the pattern more visible.
        // TODO: Would be best to adjust sample count based on dist-to-occluder - larger when expecting a thick shadow edge?
        const float seedMult                 = 12.0; // Useful, when taking many light samples, to avoid regular shadow patterns.
        const float lightSideSampleCount     = 40.0;
        const float lightSideSampleCountHalf = lightSideSampleCount * 0.5;

        const float  lightPosSideOffsetRatio = (fmod(threadHorzSeed * seedMult, lightSideSampleCount) - lightSideSampleCountHalf) / lightSideSampleCountHalf;
        const float  lightPosUpOffsetRatio = (fmod(threadVertSeed * seedMult, lightSideSampleCount) - lightSideSampleCountHalf) / lightSideSampleCountHalf;

        const float3 lightSideOffset     = enableAlteringRayDirection * (lightEmitterRadius * lightSideDir * lightPosSideOffsetRatio);
        const float3 lightUpOffset       = enableAlteringRayDirection * (lightEmitterRadius * lightUpDir * lightPosUpOffsetRatio);
        
        // Alter light position.
        lightPosition += lightSideOffset + lightUpOffset;
    }

    const float3 rayDir = normalize( lightPosition - rayOrigin );

    const float3 vectorToCamera = cameraPos - rayOrigin;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( lightDirection, -rayDir ) < lightConeMinDot ) {
        // Preserve illumination from shadow map. 
        // #TODO: It may be needed to raytrace shadows slightly outiside of spot light cone to ensure proper blurring of soft shadows.
        //g_illumination[ dispatchThreadId.xy ] = (uint)(g_preIllumination[ dispatchThreadId.xy ] * 255.0f);
        return;
    }

	const float3 surfaceNormal  = g_surfaceNormal.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
    const float  normalLightDot = dot( surfaceNormal, rayDir );

	// #TODO: Discard ray with zero contribution.

    // IMPORTANT: Cannot quit early when pixel is already in shadow, because we need to know if a mesh changed blur radius at that pixel!!!!
    // Or maybe we can rely on blur radius from shadow map?
    
	// If all position components are zeros or face is backfacing the light - set as in shadow.
    // It is very important to shadow these areas, as non-shadowed pixels on normalmapped surfaces 
    // would otherwise brighten shadowed areas after blurring (causing serious light leaks).
    if ( !any( rayOrigin ) || normalLightDot < 0.0f /*|| illuminationUint == 0*//*|| dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm*/ ) 
    { 
        g_hardShadow[ dispatchThreadId.xy ]   = 255;
        g_mediumShadow[ dispatchThreadId.xy ] = 255;
        g_softShadow[ dispatchThreadId.xy ]   = 255;
        return;
    }

    const float rayMaxLength = length( lightPosition - rayOrigin );

    // We only care about intersections closer to ray origin than the light source. #TODO: Or all of them? - alpha etc, illumination?
    float nearestHitDist = 10000.0f;

    // #OPTIMIZATION: Could be more effective to trace ray from light to surface - because we care about the nearest to light intersection. Easier to skip other objects. 

    float shadow = 0.0f;

    const float pixelSizeInWorldSpace = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);

    if ( rayMeshIntersect( rayOrigin, rayDir, rayMaxLength, nearestHitDist, shadow ) )
    {
        const float surfaceDistToLight    = rayMaxLength;
        const float surfaceDistToOccluder = nearestHitDist;
        const float occluderDistToLight   = surfaceDistToLight - surfaceDistToOccluder;
        
        const float blurRadiusInWorldSpace  = calculateShadowBlurRadius( lightEmitterRadius, surfaceDistToOccluder, occluderDistToLight );
        const float blurRadiusInScreenSpace = blurRadiusInWorldSpace / pixelSizeInWorldSpace;

        // #TODO: just in case, to test if there is a bug where shadow has larger value.
        //shadow = min(1.0, shadow);

        // Write shadow data to texture.
        if (blurRadiusInScreenSpace <= shadowHardBlurRadiusEndThreshold)
        {
            const float prevHardShadowInt = (float)g_hardShadow[ dispatchThreadId.xy ] /*/ 255.0f*/;
            const float prevMediumShadowInt = (float)g_mediumShadow[ dispatchThreadId.xy ] /*/ 255.0f*/;

            // How much light should go to harder/softer shadow layer.
            const float shadowRatio = 0.0;//min(1.0, max(0.0, blurRadiusInScreenSpace - shadowHardBlurRadiusStartThreshold) / shadowHardBlurRadiusTransitionWidth);
            
            const float extraShadow       = ceil(shadow * 255.0);
            const float extraHardShadow   = round((1.0 - shadowRatio) * extraShadow);
            //const float extraMediumShadow = extraShadow - extraHardShadow;

            const float hardShadowInt   = prevHardShadowInt + extraHardShadow;
            //const float mediumShadowInt = prevMediumShadowInt + extraMediumShadow;

            g_hardShadow[ dispatchThreadId.xy ]   = (uint)( min(255.0f, ceil(hardShadowInt) ) );
            //g_mediumShadow[ dispatchThreadId.xy ] = (uint)( min(255.0f, ceil(mediumShadowInt) ) );

            // Mark some pixels as "untouchable" during blur - cannot be used as samples or as gather center.
            //g_distToOccluderMediumShadow[ dispatchThreadId.xy ] = 10000.0;
            //g_distToOccluderSoftShadow[ dispatchThreadId.xy ]   = 10000.0;
        }

        if (blurRadiusInScreenSpace > shadowHardBlurRadiusStartThreshold && blurRadiusInScreenSpace <= shadowSoftBlurRadiusEndThreshold)
        {
            const float prevMediumShadowInt = (float)g_mediumShadow[ dispatchThreadId.xy ] /*/ 255.0f*/;
            const float prevSoftShadowInt   = (float)g_softShadow[ dispatchThreadId.xy ] /*/ 255.0f*/;

            // How much light should go to harder/softer shadow layer.
            const float shadowRatio = 0.0;//min(1.0, max(0.0, blurRadiusInScreenSpace - shadowSoftBlurRadiusStartThreshold) / shadowSoftBlurRadiusTransitionWidth);
            
            const float extraShadow       = ceil(shadow * 255.0);
            const float extraMediumShadow = round((1.0 - shadowRatio) * extraShadow);
            //const float extraSoftShadow   = extraShadow - extraMediumShadow;

            const float mediumShadow = prevMediumShadowInt + extraMediumShadow;
            //const float softShadow   = prevSoftShadowInt + extraSoftShadow;

            g_mediumShadow[ dispatchThreadId.xy ] = (uint)( min(255.0f, ceil(mediumShadow) ) );
            //g_softShadow[ dispatchThreadId.xy ]   = (uint)( min(255.0f, ceil(softShadow) ) );

            // Mark some pixels as "untouchable" during blur - cannot be used as samples or as gather center.
            //g_distToOccluderHardShadow[ dispatchThreadId.xy ] = 10000.0;
            //g_distToOccluderSoftShadow[ dispatchThreadId.xy ] = 10000.0;
        }
        
        if (blurRadiusInScreenSpace > shadowSoftBlurRadiusStartThreshold)
        {
            const float prevSoftShadowInt = (float)g_softShadow[ dispatchThreadId.xy ] /*/ 255.0f*/;
            const float softShadow     = prevSoftShadowInt + ceil(shadow * 255.0);

            g_softShadow[ dispatchThreadId.xy ] = (uint)( min(255.0f, ceil(softShadow) ) );

            // Mark some pixels as "untouchable" during blur - cannot be used as samples or as gather center.
            //g_distToOccluderMediumShadow[ dispatchThreadId.xy ] = 10000.0;
            //g_distToOccluderHardShadow[ dispatchThreadId.xy ]   = 10000.0;
        }

        //// Write shadow data to texture.
        //if (blurRadiusInScreenSpace <= hardShadowBlurRadiusThreshold)
        //{
        //    const float prevHardShadow = (float)g_hardShadow[ dispatchThreadId.xy ] / 255.0f;
        //    const float hardShadow     = prevHardShadow + shadow;

        //    g_hardShadow[ dispatchThreadId.xy ] = (uint)( round( min(1.0f, hardShadow ) * 255.0f ) );
        //}
        //else if (blurRadiusInScreenSpace <= softShadowBlurRadiusThreshold)
        //{
        //    const float prevMediumShadow = (float)g_mediumShadow[ dispatchThreadId.xy ] / 255.0f;
        //    const float mediumShadow     = prevMediumShadow + shadow;

        //    g_mediumShadow[ dispatchThreadId.xy ] = (uint)( round( min(1.0f, mediumShadow ) * 255.0f ) );
        //}
        //else
        //{
        //    const float prevSoftShadow = (float)g_softShadow[ dispatchThreadId.xy ] / 255.0f;
        //    const float softShadow     = prevSoftShadow + shadow;

        //    g_softShadow[ dispatchThreadId.xy ] = (uint)( round( min(1.0f, softShadow ) * 255.0f ) );
        //}

        // Write dist-to-occluder data to texture.
        if (blurRadiusInScreenSpace <= distToOccluderHardBlurRadiusEndThreshold)
        {
            const float prevDistToOccluderHardShadow = g_distToOccluderHardShadow[ dispatchThreadId.xy ];
            g_distToOccluderHardShadow[ dispatchThreadId.xy ] = min( prevDistToOccluderHardShadow, surfaceDistToOccluder );
        }

        if (blurRadiusInScreenSpace > distToOccluderHardBlurRadiusStartThreshold && 
            blurRadiusInScreenSpace <= distToOccluderSoftBlurRadiusEndThreshold)
        {
            const float prevDistToOccluderMediumShadow = g_distToOccluderMediumShadow[ dispatchThreadId.xy ];
            g_distToOccluderMediumShadow[ dispatchThreadId.xy ] = min( prevDistToOccluderMediumShadow, surfaceDistToOccluder );
        }

        if (blurRadiusInScreenSpace > distToOccluderSoftBlurRadiusStartThreshold)
        {
            const float prevDistToOccluderSoftShadow = g_distToOccluderSoftShadow[ dispatchThreadId.xy ];
            g_distToOccluderSoftShadow[ dispatchThreadId.xy ] = min( prevDistToOccluderSoftShadow, surfaceDistToOccluder );
        }
    }
}

// Returns blur radius in world space.
float calculateShadowBlurRadius( const float lightEmitterRadius, const float distToOccluder, const float distLightToOccluder )
{
    const float blurRadiusInWorldSpace = min( maxShadowWorldSpaceBlurRadius, lightEmitterRadius * ( distToOccluder / distLightToOccluder ) );

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