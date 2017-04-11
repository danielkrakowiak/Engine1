#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"
#include "Common\SampleWeighting.hlsl"

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

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture     : register( t0 );
Texture2D<float4> g_normalTexture       : register( t1 ); 
Texture2D<float>  g_hitDistanceTexture  : register( t2 );

// Input / Output.
RWTexture2D<float> g_finalDistToOccluder : register( u0 );

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
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    const float2 pixelSize0 = 1.0f / outputTextureSize;

    const float3 centerPosition     = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 centerNormal       = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = cameraPos - centerPosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    //#TODO: Select mipmap based on central hitDistance - maybe a log of what we need - the rest through avaraging.
    // Why reflection doesn't blur through edges?
    // Check linear/point sampling.
    // Avarage pixels from some region.

    const float  mipmap      = 2.0f;
    const float2 pixelSize   = pixelSize0 * pow( 2.0f, mipmap );

    float sampleWeightSum = 0.0f;

    float searchRadius = 45.0f;

    float hitDistance = 0.0f;

    const float centerHitDistance = min( maxHitDistance, g_hitDistanceTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f/*mipmap*/ ) );

    for ( float y = -searchRadius; y <= searchRadius; y += 1.0f )
    {
        for ( float x = -searchRadius; x <= searchRadius; x += 1.0f )
        {
            const float2 sampleTexcoords = texcoords + float2( x * pixelSize.x, y * pixelSize.y );

            const float sampleHitDistance = min( centerHitDistance/*maxHitDistance*/, g_hitDistanceTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, mipmap ) );

            // Weight depanding on difference in position/normal between center and the sample.
            const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
            const float3 sampleNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 


            const float  positionDiff   = length( samplePosition - centerPosition );
            const float  normalDiff     = 1.0f - dot(centerNormal, sampleNormal);
            //const float  power          = -( positionDiffMul * positionDiff * positionDiff + normalDiffMul * normalDiff ) / positionNormalThreshold;
            //const float  sampleWeight2  = pow( e, power );
            const float samplesHitDistDiff = max( 0.0f, sampleHitDistance - centerHitDistance );

            //const float samplesPosNormDiff = positionDiffMul * positionDiff * positionDiff + normalDiffMul * normalDiff;
            const float samplesPosNormDiff = positionDiffMul * samplesHitDistDiff * samplesHitDistDiff /*+ normalDiffMul * normalDiff*/;
            const float sampleWeight1 = 1.0f - positionDiffMul * ( sampleHitDistance / centerHitDistance );//max(0.0f, 1.0f - (positionDiffMul * samplesHitDistDiff));//getSampleWeightSimilarSmooth( samplesPosNormDiff, positionNormalThreshold );

            // Weight diminishing importance of samples hitting the sky if any other samples are available.
            const float sampleWeight2 = 1.0f;//1.0f - saturate(sampleHitDistance / 200.0f);
			//if ( sampleHitDistance > maxHitDistance)
			//    sampleWeight1 = 0.0f;

            // Discard samples which are off-screen (zero dist-to-occluder).
            const float sampleWeight3 = getSampleWeightGreaterThan( sampleHitDistance, 0.0f );

            hitDistance += sampleHitDistance * sampleWeight1 * sampleWeight2 * sampleWeight3;
            sampleWeightSum += sampleWeight1 * sampleWeight2 * sampleWeight3;
        }
    }

    hitDistance /= sampleWeightSum;

    g_finalDistToOccluder[ dispatchThreadId.xy ] = hitDistance;
}
