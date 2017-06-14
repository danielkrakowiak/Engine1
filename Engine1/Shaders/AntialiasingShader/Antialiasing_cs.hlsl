#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12

#include "FXAA.hlsl"

// fxaaQualitySubpix - Only used on FXAA Quality.
// Choose the amount of sub-pixel aliasing removal.
// This can effect sharpness.
//   1.00 - upper limit (softer)
//   0.75 - default amount of filtering
//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
//   0.25 - almost off
//   0.00 - completely off

// fxaaQualityEdgeThreshold - Only used on FXAA Quality.
// The minimum amount of local contrast required to apply algorithm.
//   0.333 - too little (faster)
//   0.250 - low quality
//   0.166 - default
//   0.125 - high quality 
//   0.063 - overkill (slower)

// fxaaQualityEdgeThresholdMin - Only used on FXAA Quality.
// Trims the algorithm from processing darks.
//   0.0833 - upper limit (default, the start of visible unfiltered edges)
//   0.0625 - high quality (faster)
//   0.0312 - visible limit (slower)
// Special notes when using FXAA_GREEN_AS_LUMA,
//   Likely want to set this to zero.
//   As colors that are mostly not-green
//   will appear very dark in the green channel!
//   Tune by looking at mostly non-green content,
//   then start at zero and increase until aliasing is a problem.

cbuffer ConstantBuffer : register( b0 )
{
    float2 pixelSizeInTexcoords;
    float2 pad1;
    float  fxaaQualitySubpix;
    float3 pad2;
    float  fxaaQualityEdgeThreshold;
    float3 pad3;
    float  fxaaQualityEdgeThresholdMin;
    float3 pad4;
};

Texture2D colorTexture;
SamplerState samplerState;

// Input
Texture2D<float4> g_srcTexture : register( t0 );

// Output.
RWTexture2D<uint4> g_dstTexture : register( u0 );

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
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) * pixelSizeInTexcoords;

    FxaaTex fxaaTex;
    fxaaTex.smpl = samplerState;
    fxaaTex.tex  = g_srcTexture;

    float4 outputColor = FxaaPixelShader(
        //
        // Use noperspective interpolation here (turn off perspective interpolation).
        // {xy} = center of pixel
        texcoords,
        fxaaTex,
        pixelSizeInTexcoords,
        fxaaQualitySubpix,
        fxaaQualityEdgeThreshold,
        fxaaQualityEdgeThresholdMin
    );

    g_dstTexture[ dispatchThreadId.xy ] = uint4( outputColor * 255.0 );
}