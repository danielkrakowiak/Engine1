
#include "Common\Constants.hlsl"
#include "Common\Utils.hlsl"
#include "Common\SampleWeighting.hlsl"

Texture2D<float4> g_colorTexture          : register( t0 );
Texture2D<float4> g_contributionTermRoughnessTexture : register( t1 );
Texture2D<float4> g_normalTexture         : register( t2 );
Texture2D<float4> g_positionTexture       : register( t3 );
Texture2D<float>  g_depthTexture          : register( t4 );
Texture2D<float>  g_hitDistanceTexture    : register( t5 );

SamplerState g_pointSamplerState  : register( s0 );
SamplerState g_linearSamplerState : register( s1 );

cbuffer ConstantBuffer
{
    float  normalThreshold;
    float3 pad1;
    float3 cameraPosition;
    float  pad2;
    float2 imageSize;
    float2 pad3;
    float2 contributionTextureFillSize;
    float2 pad4;
    float2 srcTextureFillSize;
    float2 pad5;
    float  positionDiffMul;
    float3 pad6;
    float  normalDiffMul;
    float3 pad7;
    float  positionNormalThreshold;
    float3 pad8;
    float  roughnessMul;
    float3 pad9;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

static const float maxDepth = 200.0f;
static const float maxHitDistance = 50.0f; // Should be less than the initial ray length. 

float4 main(PixelInputType input) : SV_Target
{
    const float depth = linearizeDepth( g_depthTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ), zNear, zFar ); // Have to account for fov tanges?

    if ( depth > maxDepth )
        return float4( 0.0f, 0.0f, 0.0f, 0.0f );

    const float  hitDistance               = g_hitDistanceTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f );
    const float4 contributionTermRoughness = g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f );

    const float roughness = roughnessMul * contributionTermRoughness.a;

    // Note: clamp min to 1 to ensure correct behavior of log2 further down the code.
    float blurRadius = max( 1.0f, log2( hitDistance + 1.0f ) * roughness / log2( depth + 1.0f ) );// * tan( roughness * PiHalf );

    const float3 centerPosition = g_positionTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz; 
    const float3 centerNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz;

    const float3 vectorToCamera = cameraPosition - centerPosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
    // to decrease search radius when looking at walls/floors at flat angle.
    // #TODO: This has to flat blur-kernel separately in vertical and horizontal directions - otherwise it introduces visible artefacts.
    //const float blurRadiusMul = saturate( abs( dot( centerNormal, dirToCamera ) ) );
    //blurRadius *= blurRadiusMul;

    const float maxMipmapLevel  = 6.0f; // To avoid sampling overly blocky, aliased mipmaps - limits maximal roughness.
    const float baseMipmapLevel = min( maxMipmapLevel, log2( blurRadius ) );

    float4 reflectionColor = float4( 0.0f, 0.0f, 0.0f, 1.0f );

    const float samplingStepInt = 1.0f;

    // This code decreases the mipmap level and calculates how many 
    // samples have to ba taken to achieve the same result (but with smoother filtering, rejecting etc).
    // Note: baseMipmapLevel may be fractional so mipmapLevelDecrease may also be fractional.
    const float mipmapLevel         = floor( baseMipmapLevel * 0.666 );
    const float mipmapLevelDecrease = baseMipmapLevel - mipmapLevel; 
    const float samplingRadius      = pow( 2.0, mipmapLevelDecrease );
    const float samplingRadiusInt   = ceil( samplingRadius );
    ////

    float2 pixelSize0   = ( 1.0f / imageSize );
    float  pixelSizeMul = pow(2.0, mipmapLevel);
    float2 pixelSize    = pixelSize0 * pixelSizeMul;

    float  sampleWeightSum = 0.0001;
    float3 sampleSum       = 0.0;

    // #TODO: Use real FOV instead of hardcoded value.
    const float pixelSizeInWorldSpace = pixelSizeMul * getPixelSizeInWorldSpace( distToCamera, Pi / 4.0f, imageSize.y );

    // Center sample gets special treatment - it's very important for low roughness (low level mipmaps) 
    // and should be ignored for high roughness (high level mipmaps) as it contains influences from neighbors (artefacts).
    // Decrease its weight based on mipmap level.
    { 
        // #TODO: Could use point sampler? Should not make a difference as we sample exact pixel center.
        const float3 centerSample = g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord/** srcTextureFillSize / imageSize*/, mipmapLevel ).rgb;
        const float  centerWeight = lerp(1.0, 0.0, mipmapLevel / 3.0); // Zero its weight for mipmap level >= 3.
        
        sampleSum       += centerSample * centerWeight;
        sampleWeightSum += centerWeight;
    }

    for ( float yInt = -samplingRadiusInt; yInt <= samplingRadiusInt; yInt += samplingStepInt )
    {
        const float y = clamp( yInt, -samplingRadius, samplingRadius );
        
        const float sampleWeightY = 1.0 - abs( yInt - y );

        for ( float xInt = -samplingRadiusInt; xInt <= samplingRadiusInt; xInt += samplingStepInt )
        {
            const float x = clamp( xInt, -samplingRadius, samplingRadius );

            const float sampleWeightX = 1.0 - abs( xInt - x );

            // Used to decrease weight of the outmost samples which are taken 
            // not exactly at pixel centers (non integer x, y relative to center).
            // Their weight has to be decreased, because they are too close to other samples
            // and sometimes could cause error from sampling twice at almost the same place.
            const float sampleWeightXY = sampleWeightX * sampleWeightY;

            // Note: This test will only work if loop iterates over integer x, y.
            // Skip central pixel as it's been already accounted for.
            if ( abs(x) < 0.001 && abs(y) < 0.001 )
                continue;

            const float2 sampleTexcoords = input.texCoord + float2( x * pixelSize.x, y * pixelSize.y ); 

            // Skip pixels which are off-screen.
            if (any(saturate(sampleTexcoords) != sampleTexcoords))
                continue;

            const float3 sampleValue   = g_colorTexture.SampleLevel( g_linearSamplerState, sampleTexcoords/** srcTextureFillSize / imageSize*/, mipmapLevel ).rgb;
            const float3 samplPosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz;
            const float3 sampleNormal  = g_normalTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz;
            
            // #TODO: Maybe I should calculate the expected difference in positions between center and sample. 
            // And then use ratio of positon-diff to expected-position-diff as weight?
            // Because right now we dim outer pixels just because they are naturally further away. 
            // And it's fine, but it's mixed with filtering pixel in the background.
            // These two processes should be clearly separated and tweakable. So two kinds of weights calculated...

            // Calculate expected distance in world space between sample and center 
            // assuming they are on flat surface facing the camera.
            const float distanceInPixels     = sqrt(x * x + y * y);

            const float expectedPositionDiff = distanceInPixels * pixelSizeInWorldSpace;
            const float positionDiff         = length( samplPosition - centerPosition );
            const float normalDiff           = 1.0f - max( 0.0, dot( sampleNormal, centerNormal ) );  

            const float positionDiffErrorRatio = max( 0.0, positionDiff - expectedPositionDiff) / expectedPositionDiff;

            const float samplesPosNormDiff = positionDiffMul * positionDiff * positionDiff + normalDiffMul * normalDiff * normalDiff;

            const float  sampleWeight1 = lerp( 1.0, 0.0, saturate( positionDiffErrorRatio ) );
            const float  sampleWeight2 = getSampleWeightSimilarSmooth( samplesPosNormDiff, positionNormalThreshold );

            const float sampleWeight = sampleWeightXY * sampleWeight1 * sampleWeight2;

            sampleSum       += sampleValue * sampleWeight;
            sampleWeightSum += sampleWeight;
        }
    }

    reflectionColor.rgb = sampleSum / sampleWeightSum;

    const float3 outputColor = contributionTermRoughness.rgb * reflectionColor.rgb;

	return float4( outputColor, 1.0f );
}