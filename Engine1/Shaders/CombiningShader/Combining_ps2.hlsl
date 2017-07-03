
#include "Common\Constants.hlsl"
#include "Common\Utils.hlsl"
#include "Common\SampleWeighting.hlsl"

Texture2D<float4> g_colorTexture          : register( t0 );
Texture2D<float4> g_contributionTermRoughnessTexture : register( t1 );
Texture2D<float4> g_normalTexture         : register( t2 );
Texture2D<float4> g_positionTexture       : register( t3 );
Texture2D<float>  g_prevHitDistance       : register( t4 );
Texture2D<float>  g_hitDistanceTexture    : register( t5 );
Texture2D<float4> g_rayOriginTexture      : register( t6 );

SamplerState g_pointSamplerState  : register( s0 );
SamplerState g_linearSamplerState : register( s1 );

cbuffer ConstantBuffer
{
    float  normalThreshold;
    float3 pad1;
    float2 imageSize;
    float2 pad2;
    float2 contributionTextureFillSize;
    float2 pad3;
    float2 srcTextureFillSize;
    float2 pad4;
    float  positionDiffMul;
    float3 pad5;
    float  normalDiffMul;
    float3 pad6;
    float  positionNormalThreshold;
    float3 pad7;
    float  roughnessMul;
    float3 pad8;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

static const float maxDepth = 200.0f;
static const float maxHitDistance = 200.0f; // Should be less than the initial ray length. 

float4 main(PixelInputType input) : SV_Target
{
    // #TODO: Should it be accumulated distance to camera instead?
    const float prevHitDistance = g_prevHitDistance.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f );

    if ( prevHitDistance > maxDepth )
        return float4( 0.0f, 0.0f, 0.0f, 0.0f );

    const float  hitDistance               = g_hitDistanceTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f );
    const float4 contributionTermRoughness = g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f );
    
    const float roughness  = roughnessMul * contributionTermRoughness.a;
    float       blurRadius = max( 0.0f, log2( hitDistance + 1.0f ) * roughness / log2( prevHitDistance + 1.0f ) );

    const float3 centerPosition = g_positionTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz; 
    const float3 centerNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz;

    { // Scaling blur-kernel based on view-angle - disabled to improve performance - not too important for higher layers - could be anabled usign some define.
        // Note: eeded only for kernel-scaling based on view-angle - could be ignored for performance.
        //float3 cameraPosition = g_rayOriginTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;

        //const float3 dirToCamera = normalize( cameraPosition - centerPosition );

        // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
        // to decrease search radius when looking at walls/floors at flat angle.
        // #TODO: This should probably flatten blur-kernel separately in vertical and horizontal directions?
        //const float blurRadiusMul = saturate( abs( dot( centerNormal, dirToCamera ) ) );
        //blurRadius *= blurRadiusMul;
    }

    const float maxMipmapLevel  = 6.0f; // To avoid sampling overly blocky, aliased mipmaps - limits maximal roughness.
    const float baseMipmapLevel = min( maxMipmapLevel, log2( blurRadius ) );

    float4 reflectionColor = float4( 0.0f, 0.0f, 0.0f, 1.0f );

    const float samplingStep = 1.0f;

    // This code decreases the mipmap level and calculates how many 
    // samples have to ba taken to achieve the same result (but with smoother filtering, rejecting etc).
    const float mipmapLevel         = baseMipmapLevel * 0.666f;
    const float mipmapLevelDecrease = baseMipmapLevel - mipmapLevel;
    const float samplingRadius      = pow( 2.0f, mipmapLevelDecrease );
    ////

    float2 pixelSize0 = ( 1.0f / imageSize );
    float2 pixelSize  = pixelSize0 * (float)pow( 2, mipmapLevel );

    float  sampleWeightSum = 0.0001;
    float3 sampleSum       = 0.0;

    for ( float y = -samplingRadius; y <= samplingRadius; y += samplingStep ) 
    {
        for ( float x = -samplingRadius; x <= samplingRadius; x += samplingStep ) 
        {
            const float2 sampleTexcoords = input.texCoord + float2( x * pixelSize.x, y * pixelSize.y ); 

            // Skip central sample? As it contains influence of other pixels because of mipmapping
            // and cannot be filtered out, because it's position is too similar to the central pixel from 0 mipmap.
            //(IDEA: Or increase drastically thresholds to be accepted for central sample)
            //PROBLEM: Other samples also contain influence of other pixels because of mipmapping...
            if ( x == 0.0 && y == 0.0 )
                continue;

            // Skip pixels which are off-screen.
            if (any(saturate(sampleTexcoords) != sampleTexcoords))
                continue;

            const float3 sampleValue = g_colorTexture.SampleLevel( g_linearSamplerState, sampleTexcoords/** srcTextureFillSize / imageSize*/, mipmapLevel ).rgb;

            const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
            const float3 sampleNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 

            const float  positionDiff       = length( samplePosition - centerPosition );
            const float  normalDiff         = 1.0f - max( 0.0, dot( sampleNormal, centerNormal ) );  
            const float  samplesPosNormDiff = positionDiffMul * positionDiff + normalDiffMul * normalDiff;

            const float  sampleWeight1 = getSampleWeightSimilarSmooth( samplesPosNormDiff, positionNormalThreshold );

            const float sampleWeight = sampleWeight1;

            sampleSum       += sampleValue * sampleWeight;
            sampleWeightSum += sampleWeight;
        }
    }

    reflectionColor.rgb = sampleSum / sampleWeightSum;

    const float3 outputColor = contributionTermRoughness.rgb * reflectionColor.rgb;

	return float4( outputColor, 1.0f );
}

// Position threshold needs to be larger when we sample with larger radius (oversampling ratio). Same for normal threshold.
// Some object id buffer would be useful to reject samples from different objects. It's impossible now to tell whether we hit the same object when sampling radius is big.

// Wanted sampling level should decrease with distance to the pixel - because roughness in screen-space decreaes (and sampling radius).

// Idea: Could check which level normal differs from highest level normal to learn how far from the edge center pixel is.
// IDEA: Could use depth to decide the maximum sampling level. It's because when we are very close to the object, there is more flat surface on the screen.
// IDEA: When depth is below 1 - skip taking log2 from it. log2 is too steep below 1. Use depth directly or something else.

// IMPORTANT: Only normals and position of the reflecting surface matter (because they influence the direction and shape of ray cone) when accepting/rejecting samples. What's seen by rays is irrelevant.

// IDEA: Select reflection mipmap based on ray distance. If distance is high, we don't care if reflection blurs over the edges? Probably we still care...

// IDEA: Always first check ray distance of neighboring pixels to decide the sampling radius. But the problem is that sampling radius could be different in each direction.

// IDEA: Maybe instead of box shaped sampling, use circle shaped sampling with different radius in each direction and different number of samples in each direction. Quite costly, but may give nice quality.

// IDEA: Sample low-res mipmap of ray distance to check if there is any object around, despite center ray distance being so high.
//       That should cause enough blurring near the edges to forget about the problem.
//       Tested - doesn't work, because difference in depth are so big that edges stay sharp as knife anyways. Plus normal mapping causes some depth values to be really high even in the middle of objects.

// If I sample only the highest reflection mipmap there is no color bleeding over the edges. If we're checking normals and positions...

// We can actually sample low level reflection mipmaps and it still looks good. We should maybe blur them a bit first in a separate pass. Or use 3x3 sampling here.

// How come reflections are not bleeding through the edges even when sampling low-level mipmaps? They actually do for rougher surfaces.

// Size of the object and reflected object on screen depends linearly on camera to object distance and reflective surface to object distance.

// Using smaller sampling radius works only to compansate for camera moving away from surface. It's because ( 1 / depth ) is not a linear function and it gets more linear only for larger depth values. 
// It has more impact for low depth values, rather than high depth values. Wait, but pixel sizes on screen also don't change linearly with depth...
// Maybe it's because properly blurred "mipmap" would look sharper at closer distance, but mipmaps don't look sharper, because they were made to be seen from distance. Artifacts of linear sampling etc?

// CURRENT ISSUES:
// - perceived blur changed when moving camera closer/farther to the reflective surface. Should be the same. Only the distance from the reflective surface to the object should changed perceived blur.