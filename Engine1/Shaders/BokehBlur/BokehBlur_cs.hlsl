#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"
#include "Common\Utils.hlsl"
#include "Common\SampleWeighting.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float2 g_textureSize;
    float2 pad1;
    float  g_apertureDiameter;
    float3 pad2;
    float  g_cameraFocusDist;
    float3 pad3;
    float  g_focalLength; // The distance from lens at which refracted rays cross (towards the scene).
    float3 pad4;
    float  g_coCMul; // Depends on how many pixels there are in a meter of image (we assume 72 dpi here).
    float3 pad5;
    float  g_maxCoC; // Radius in pixels.
    float3 pad6;
    float  g_relativeDepthThreshold; // How much Bokeh from further pixels (from the camera) can influence closer pixels.
    float3 pad7;                     // Can be interpreted as acceptable relative difference between center-depth and sample-depth.
};

SamplerState g_samplerState;

// Input.
Texture2D<float4> g_inputTexture : register( t0 );
Texture2D<float>  g_depthTexture : register( t1 );

// Output.
RWTexture2D<float4> g_outputTexture : register( u0 );

float calculateCircleOfConfusionRadius(
    in const float apertureDiameter, 
    in const float cameraFocusDist,
    in const float distToObject,
    in const float focalLength );

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(8, 8, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    // Note: Calculate texcoords for the pixel center.
    // #TODO: Could be optimized by passing pixelSizeInTexcoords through constant buffer.
    const float2 texcoords                = ((float2)dispatchThreadId.xy + 0.5f) / g_textureSize;
    const float2 halfPixelSizeInTexcoords = 0.5 / g_textureSize;
    const float2 pixelSizeInTexcoords     = 1.0 / g_textureSize;

    const float maxCoCSqr = g_maxCoC * g_maxCoC;

    // Calculate spread-radius for each sample withing max-radius.
    // if central pixel is within that spread-radius - add sample to weight.
    // This way the bright points should spread light to pixels equally, 
    // even when some of those pixels are much closer to camera.

    const float centerDepth = linearizeDepth( g_depthTexture.SampleLevel( g_samplerState, texcoords, 0.0f ), zNear, zFar );
    const float centerCoC = g_coCMul * calculateCircleOfConfusionRadius( g_apertureDiameter, g_cameraFocusDist, centerDepth, g_focalLength );

    float4 colorSum = 0.0;
    float weightSum = 0.0;

    for (float y = -g_maxCoC; y <= g_maxCoC; ++y)
    {
        for (float x = -g_maxCoC; x <= g_maxCoC; ++x)
        {
            const float sampleDistSqr = x*x + y*y;

            if (sampleDistSqr <= maxCoCSqr)
            {
                const float2 sampleTexcoords = texcoords + float2(x, y) * pixelSizeInTexcoords;

                const float  sampleDepth = linearizeDepth( g_depthTexture.SampleLevel( g_samplerState, sampleTexcoords, 0.0f ), zNear, zFar );
                const float4 sampleColor = g_inputTexture.SampleLevel( g_samplerState, sampleTexcoords, 0.0f );

                float coC = g_coCMul * calculateCircleOfConfusionRadius( g_apertureDiameter, g_cameraFocusDist, sampleDepth, g_focalLength );

                coC = min( g_maxCoC, coC );

                // The sample has less impact if it scatters over a larger bokeh area.
                // Note: use real CoC value here, before clamping based on depth.
                const float bokehArea = Pi * coC * coC;
                const float bokehSizeWeight = min(1.0, 1.0 / bokehArea);

                // This is very important - it avoids spilling color from further objects onto closer objects,
                // but at the same time it doesn't cause ringing/visible edges. It works much better than weighting 
                // samples using depth difference between center and samples.
                if (sampleDepth > centerDepth)
                {
                    // #TODO_OPTIMIZATION: This "if" could be replaced with multiplication/lerp etc.
                    coC = min( centerCoC, coC );
                }

                const float coCSqr = coC * coC;

                if (sampleDistSqr <= coCSqr)
                {
                    const float weight = bokehSizeWeight;

                    colorSum  += (sampleColor * weight);
                    weightSum += weight;
                }
            }
        }
    }

    const float4 finalColor = colorSum / weightSum;

    g_outputTexture[ dispatchThreadId.xy ] = finalColor;
}

float calculateCircleOfConfusionRadius(
    in const float apertureDiameter, 
    in const float cameraFocusDist,
    in const float distToObject,
    in const float focalLength ) 
{
    // Source: https://en.wikipedia.org/wiki/Circle_of_confusion

    float lensMagnification = focalLength / (cameraFocusDist - focalLength);

    // Note: we assume that dist-to-object is larger than camera-focus-dist.
    float blurCirlceDiameter = apertureDiameter * max(0.0, distToObject - cameraFocusDist) / distToObject;

    float circleOfConfusionDiameter = blurCirlceDiameter * lensMagnification;

    return circleOfConfusionDiameter * 0.5; // We have the diameter, but we return radius.
}

