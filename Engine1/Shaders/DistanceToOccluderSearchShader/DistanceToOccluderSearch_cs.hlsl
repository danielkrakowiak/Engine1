#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"
#include "Common\SampleWeighting.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    float3 lightPosition;
    float  pad2;
    float  lightConeMinDot;
    float3 pad3;
    float3 lightDirection;
    float  pad4;
    float  lightEmitterRadius;
    float3 pad5;
    float2 inputTextureSize;
    float2 pad6;
    float2 outputTextureSize;
    float2 pad7;
    float  positionThreshold;
    float3 pad8;
    float  g_searchRadius;
    float3 pad9;
    float  g_searchStep; // In pixels - distance between neighbor samples.
    float3 pad10;
};

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture : register( t0 );
Texture2D<float4> g_normalTexture   : register( t1 ); 
Texture2D<float>  g_distToOccluder  : register( t2 );

// Input / Output.
RWTexture2D<float> g_finalDistToOccluder : register( u0 );

//#define DEBUG

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
    #ifdef DEBUG ///////////////////////////////////////////////////////
    if ( dispatchThreadId.x != 512 || dispatchThreadId.y != 384 )
    {
        return;
    }
    #endif
    /////////////////////////////////////////////////////////////////

    //const float2 texcoords = dispatchThreadId.xy / outputTextureSize;
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    //const float2 outputTextureHalfPixelSize = 1.0f / outputTextureSize; // Should be illumination texture size?

    const float2 inputPixelSize = 1.0f / inputTextureSize; // Accounts for the selected mipmap size.

    const float3 surfacePosition     = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceNormal       = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = cameraPos - surfacePosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    const float3 vectorToLight       = lightPosition.xyz - surfacePosition;
    const float3 dirToLight          = normalize( vectorToLight );
    const float  distToLight         = length( vectorToLight );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( lightDirection, -dirToLight ) < lightConeMinDot ) {
        g_finalDistToOccluder[ dispatchThreadId.xy ] = 0.0f;
        return;
    }

    //const float  centerDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, texcoords, 0.0f );

    const float3 centerPosition       = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f ).xyz; 
    //const float  centerWeight         = saturate( 500.0f - centerDistToOccluder );

    // #TODO: Replace with call to getPixelSizeInWorldSpace. Check if it caused any errors. Use real FOV instead of hardcoded value.
    const float pixelSizeInWorldSpace = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);

    float2 texCoordShift = float2(0.0f, 0.0f); // x horizontal, y - vertical.
    
    float valueSum = 0.0f;//centerDistToOccluder * centerWeight;
    float weightSum = 0.00001f/* + centerWeight*/;

    // Modify search radius and search step depanding on distance from surface to camera.
    // Needed because when we zoom in the search area has to be larger in screen space (while staying similar in world space).
    // Step has to be increased to avoid taking too many samples when zoomed in - 
    // we don't need any extra precision in that case anyways - 
    // if sample sensity was fine for zoomed out, it will be fine when zoomed in.

	// #TODO: It should probably be re-enabled.
	// But beware - it destroys otherise great smoothnes on the result. Can we improve?
    const float mulFromDistToCamera = 4.0 / (0.1 + distToCamera);
    const float searchRadius = g_searchRadius /** mulFromDistToCamera*/;
    const float searchStep   = g_searchStep /** mulFromDistToCamera*/;

    // #TODO: Should we scale this search-radius based on view-angle?
    // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
    // to decrease search radius when looking at walls/floors at flat angle.
    //const float searchRadiusMul = saturate( abs( dot( surfaceNormal, dirToCamera ) ) );
    //searchRadius *= searchRadiusMul;

    const float searchRadiusSqared = searchRadius * searchRadius;

    for ( float y = -searchRadius; y <= searchRadius; y += searchStep )
    {
        for ( float x = -searchRadius; x <= searchRadius; x += searchStep )
        {
            const float radialWeight = max(0.0, 1.0 - ((x * x + y * y) / searchRadiusSqared));
            
            if ( radialWeight > 0.0 )
            {
				// Decrease sample weight the further it is from the center in screen-space. To achieve smooth blending.
                const float sampleWeight1 = pow(radialWeight, 2.0);

                const float2 sampleTexcoords = texcoords + float2(x * inputPixelSize.x, y * inputPixelSize.y);

                const float sampleDistToOccluder = g_distToOccluder.SampleLevel(g_pointSamplerState, sampleTexcoords, 0.0);

				#ifdef DEBUG ///////////////////////////////////////////////////////
				g_finalDistToOccluder[(int2) (sampleTexcoords * outputTextureSize)] = 0.2;
				#endif /////////////////////////////////////////////////////////////

				// Discarding samples which are fully lit - not even in partial shadow (huge dist-to-ccluder).
				const float sampleWeight2 = getSampleWeightLowerThan(sampleDistToOccluder, 999.0f);

				// Discard samples which are off-screen (zero dist-to-occluder).
				const float sampleWeight3 = getSampleWeightGreaterThan(sampleDistToOccluder, 0.0f);

				//const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
				//const float positionDiff = length( samplePosition - centerPosition );

				//const float sampleWeight3 = 1.0f;//pow( e, -positionDiff * positionDiff / positionThreshold );

				// TEST: The more sample differs from the central one, the less weight it gets.
				//const float distDiff = max(0.0f, sampleDistToOccluder - centerDistToOccluder);
				//const float distThreshold = 1.0f;
				//const float sampleWeight4 = 1.0f;//pow( e, -distDiff * distDiff / distThreshold );

				const float sampleWeight = sampleWeight1 * sampleWeight2 * sampleWeight3;

                valueSum += sampleDistToOccluder * sampleWeight;
                weightSum += sampleWeight;

				#ifdef DEBUG ///////////////////////////////////////////////////////
				g_finalDistToOccluder[(int2) (sampleTexcoords * outputTextureSize)] = max(0.2, sampleWeight1);
				#endif /////////////////////////////////////////////////////////////
            }
        }
    }

    const float distToOccluder = valueSum / weightSum;

    g_finalDistToOccluder[ dispatchThreadId.xy ] = distToOccluder;
}
