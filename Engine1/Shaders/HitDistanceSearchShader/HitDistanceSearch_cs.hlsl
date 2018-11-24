#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"
#include "Common\SampleWeighting.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    float2 outputTextureSize;
    float2 pad2;
    float2 inputTextureSize;
    float2 pad3;
    float  positionThreshold;
    float3 pad4;
    float  positionDiffMul;
    float3 pad5;
    float  normalDiffMul;
    float3 pad6;
    float  positionNormalThreshold;
    float3 pad7;
    float  minSampleWeightBasedOnDistance;
    float3 pad8;
    float  decreaseBlurForSmallValues; // 0 - no, 1 - yes.
    float3 pad9;
    float  maxHitDistForDecreasedBlur;
    float3 pad10;
};

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture     : register( t0 );
Texture2D<float4> g_normalTexture       : register( t1 ); 
Texture2D<float>  g_hitDistanceTexture  : register( t2 );

// Input / Output.
RWTexture2D<float> g_finalDistToOccluder : register( u0 );

static const float maxHitDistance = 50.0f; // Should be less than the initial ray length. 

void sampleWeightedHitDistance( 
    const float2 texcoords, const float mipmap, 
    const float2 centerTexcoords, const float centerSampleValue, 
    out float sampleValue, out float sampleWeight 
);

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

    const float2 inputPixelSize0 = 1.0f / inputTextureSize;

    const float3 centerPosition    = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0 ).xyz;
	const float  centerHitDistance = min( maxHitDistance, g_hitDistanceTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f) );

    const float3 vectorToCamera = cameraPos - centerPosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    float mipmap            = 2.0f;//2.0
    float mipmapInt         = 0.0f;
    float hitDistance       = 0.0f;
    float sampleWeightSum   = 0.0001f;
    float searchRadiusSqr   = 0.0f;
    float searchRadius      = 10.0;// / max(1.0, distToCamera);
    float maxSearchRadius   = 100.0f;
    float searchStep        = 1.0f;// / max(1.0, distToCamera);
    float searchRadiusStep  = 0.5f;

    // #TODO: The problem with quality/visible artifacts in final-hit-dist is when only samples along one line 
    // (from center to outer circle) give meaningfull results. 
    // The number of gathered samples is small and so high variance appears in form of artifacts. 
    // One solution: Step non-linearly on x - more slowly on x near -searchRadius/+searchRadius, because it causes quick jumps in y value at this points.
    // Use some mapping of linear scale to non-linear scale. link: https://www.cg.tuwien.ac.at/research/theses/matkovic/node36.html
    // Or check 1 / smoothstep etc.

    // #TODO: Blur radius is too high. Things are too blurred. Or maybe too blurred where they don't need to.
	        
    //[unroll(126)]
    float2 samplePixel;
    for ( samplePixel.y = -searchRadius; samplePixel.y <= searchRadius; samplePixel.y += searchStep )
    {
        for ( samplePixel.x = -searchRadius; samplePixel.x <= searchRadius; samplePixel.x += searchStep )
        {
            mipmapInt = floor( mipmap );

            // #TODO: If only mipmap increases by one - could be optimized through multiplication by 2 instead of calculating power.
            const float2 inputPixelSize = inputPixelSize0 * pow( 2.0f, mipmapInt );

            // Lower the weight of samples, which are far from center in screen-space 
            // (to fucus on near samples if they are available, or on far samples if they are the only ones available).
            const float sampleDistFromCenter = saturate(length(samplePixel) / maxSearchRadius);
            const float gaussThreshold = 0.005;
			const float sampleDistFromCenterSqr = sampleDistFromCenter * sampleDistFromCenter;
            const float weightPower = -sampleDistFromCenterSqr / gaussThreshold;
            const float weightFromRadius = clamp( pow( e, weightPower ), 0.00001, 1.0 );

            float2 sampleTexcoords = texcoords + float2( samplePixel * inputPixelSize );

            float sampleHitDistance = 0.0f;
            float sampleWeight      = 0.0f;

            sampleWeightedHitDistance( 
                sampleTexcoords, mipmapInt, 
                texcoords, centerHitDistance, 
                sampleHitDistance, sampleWeight 
            );

            hitDistance     += min( maxHitDistance, sampleHitDistance ) * sampleWeight * weightFromRadius;
            sampleWeightSum += sampleWeight * weightFromRadius;
        }
    }

    // When blurring hit-distance, near-zero values usually get over blurred,
    // because of how easily they can be dominated by larger values. This multiplier helps reducing that effect
    // and maintaining sharp reflections where object touches reflective surface.
    const float blurMult = max( 1.0 - decreaseBlurForSmallValues, min( 1.0, centerHitDistance / maxHitDistForDecreasedBlur ) );

    g_finalDistToOccluder[ dispatchThreadId.xy ] = lerp( centerHitDistance, hitDistance / sampleWeightSum, blurMult );
}

void sampleWeightedHitDistance( 
    const float2 texcoords, const float mipmap, 
    const float2 centerTexcoords, const float centerSampleValue, 
    out float sampleValue, out float sampleWeight )
{
    sampleValue = min( maxHitDistance, g_hitDistanceTexture.SampleLevel( g_pointSamplerState, texcoords, mipmap ) );

    const float samplesHitDistDiff = abs(sampleValue - centerSampleValue);

    // To avoid blurring small values (into larger values) - needed when object touches the reflective surface.
    const float minSampleWeightBasedOnDistance2 = lerp( 0.0000001, minSampleWeightBasedOnDistance, saturate(centerSampleValue) );

    // Weight diminishing importance of samples hitting the sky if any other samples are available.
    const float sampleWeight2 = clamp( maxHitDistance - sampleValue, minSampleWeightBasedOnDistance2, 1.0 );

    // Discard samples which are off-screen (zero dist-to-occluder).
    const float sampleWeight3 = getSampleWeightGreaterThan( sampleValue, 0.0 );

    sampleWeight = sampleWeight2 * sampleWeight3;
}
