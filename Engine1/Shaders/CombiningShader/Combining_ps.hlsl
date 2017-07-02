
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

    const float hitDistance = g_hitDistanceTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f );

    const float roughness     = roughnessMul * g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f ).a;/*g_roughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f )*/;
    float       blurRadius    = max( 0.0f, log2( hitDistance + 1.0f ) * roughness / log2( depth + 1.0f ) );// * tan( roughness * PiHalf );

    const float3 centerPosition = g_positionTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz; 
    const float3 centerNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz;

    const float3 dirToCamera = normalize( cameraPosition - centerPosition );

    // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
    // to decrease search radius when looking at walls/floors at flat angle.
    // #TODO: This should probably flatten blur-kernel separately in vertical and horizontal directions?
    const float blurRadiusMul = saturate( abs( dot( centerNormal, dirToCamera ) ) );
    blurRadius *= blurRadiusMul;

    float samplingRadius      = min( 1.0f, blurRadius ) * 0.5f;
    float samplingMipmapLevel = log2( blurRadius );

    const float3 contributionTerm = g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f ).rgb;
    
    float4 reflectionColor = float4( 0.0f, 0.0f, 0.0f, 1.0f );

    const int2 texCoordsInt = (int2)( imageSize * input.texCoord );

    if ( samplingRadius <= 0.0001f )
    {
        reflectionColor.rgb = g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord * srcTextureFillSize / imageSize, 0.0f ).rgb;
    }
    else
    {
        const float samplingStep = 1.0f;

        const float3 centerNormal   = g_normalTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz;
        const float3 centerPosition = g_positionTexture.SampleLevel( g_pointSamplerState, input.texCoord, 0.0f ).xyz;

        // FOR TEST - TO INCREASE QUALITY:
        samplingMipmapLevel /= 4.0;//2.0;
        //samplingStep /= 2.0;
        samplingRadius *= 8.0;//4.0
        ///////////////////////////////////

        float2 pixelSize0 = ( 1.0f / imageSize );
        float2 pixelSize = pixelSize0 * (float)pow( 2, samplingMipmapLevel );

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

                const float3 sampleValue = g_colorTexture.SampleLevel( g_linearSamplerState, sampleTexcoords/** srcTextureFillSize / imageSize*/, samplingMipmapLevel ).rgb;

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
    }

    const float3 outputColor = contributionTerm * reflectionColor.rgb;

	return float4( outputColor, 1.0f );
}