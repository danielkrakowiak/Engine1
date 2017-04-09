#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Utils\SampleWeighting.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    float2 outputTextureSize;
    float2 pad2;
    float  positionThreshold;
    float3 pad3;
    float  positionDiffMul;
    float3 pad4;
    float  normalDiffMul;
    float3 pad5;
    float  positionNormalThreshold;
    float3 pad6;
};

//#define DEBUG
//#define DEBUG2

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture : register( t0 );
Texture2D<float4> g_normalTexture   : register( t1 ); 
Texture2D<float>  g_hitDistanceTexture  : register( t2 );

// Input / Output.
RWTexture2D<float> g_finalDistToOccluder : register( u0 );

static const float Pi = 3.14159265f;
//static const float e = 2.71828f;
static const float positionThresholdFalloff = 0.4f;
static const float maxHitDistance = 50.0f; // Should be less than the initial ray length. 

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
        return;
    #endif
    /////////////////////////////////////////////////////////////////

    //const float2 texcoords = dispatchThreadId.xy / outputTextureSize;
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    //const float2 outputTextureHalfPixelSize = 1.0f / outputTextureSize; // Should be illumination texture size?

    const float2 pixelSize0 = 1.0f / outputTextureSize;

    const float3 centerPosition     = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 centerNormal       = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = cameraPos - centerPosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    //const float3 vectorToLight       = lightPosition.xyz - surfacePosition;
    //const float3 dirToLight          = normalize( vectorToLight );
    //const float  distToLight         = length( vectorToLight );

    // If pixel is outside of spot light's cone - ignore.
    //if ( dot( lightDirection, -dirToLight ) < lightConeMinDot ) {
    //    g_finalDistToOccluder[ dispatchThreadId.xy ] = 0.0f;
    //    return;
    //}

    //#TODO: Select mipmap based on central hitDistance - maybe a log of what we need - the rest through avaraging.
    // Why reflection doesn't blur through edges?
    // Check linear/point sampling.
    // Avarage pixels from some region.

    const float  mipmap      = 2.0f;
    const float2 pixelSize   = pixelSize0 * pow( 2.0f, mipmap );

    float sampleWeightSum = 0.0f;

    float searchRadius = 2.0f;

    float hitDistance = 0.0f;

    for ( float y = -searchRadius; y <= searchRadius; y += 1.0f )
    {
        for ( float x = -searchRadius; x <= searchRadius; x += 1.0f )
        {
            const float2 sampleTexcoords = texcoords + float2( x * pixelSize.x, y * pixelSize.y );

            const float sampleHitDistance = min( maxHitDistance, g_hitDistanceTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, mipmap ) );

            // Weight decreasing importance of samples further from the pixel.
            const float sampleWeight1 = 1.0f;//pow(1.0f - (length(searchRadius) / sqrt(searchRadius*searchRadius * 2.0f)), 4.0f);

            // Weight depanding of difference in position between center and the sample.
            const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
            const float3 sampleNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
            const float  positionDiff   = length( samplePosition - centerPosition );
            const float  normalDiff     = 1.0f - dot(centerNormal, sampleNormal);
            const float  power          = -( positionDiffMul * positionDiff * positionDiff + normalDiffMul * normalDiff ) / positionNormalThreshold;
            const float  sampleWeight2  = pow( e, power );

            // Weight diminishing importance of samples hitting the sky if any other samples are available.
            const float sampleWeight3 = 1.0f;//1.0f - saturate(sampleHitDistance / 200.0f);
			//if ( sampleHitDistance > maxHitDistance)
			//    sampleWeight1 = 0.0f;

            // Discard samples which are off-screen (zero dist-to-occluder).
            const float sampleWeight4 = getSampleWeightGreaterThan( sampleHitDistance, 0.0f );

            hitDistance += sampleHitDistance * sampleWeight1 * sampleWeight2 * sampleWeight3 * sampleWeight4;
            sampleWeightSum += sampleWeight1 * sampleWeight2 * sampleWeight3 * sampleWeight4;
        }
    }

    hitDistance /= sampleWeightSum;

    g_finalDistToOccluder[ dispatchThreadId.xy ] = hitDistance;
}
