#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"
#include "Common\SampleWeighting.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 g_cameraPos;
    float  pad1;
    float3 g_lightPosition;
    float  pad2;
    float  g_lightConeMinDot;
    float3 pad3;
    float3 g_lightDirection;
    float  pad4;
    float  g_lightEmitterRadius;
    float3 pad5;
    float2 g_outputTextureSize;
    float2 pad6;
    float  g_positionThreshold;
    float3 pad7;
    float  g_normalThreshold;
    float3 pad8;
    float  g_positionSampleMipmapLevel; // Can be used to improve performance by sampling higher level mipmaps.
    float3 pad9;
    float  g_normalSampleMipmapLevel;
    float3 pad10;
    float  g_shadowSampleMipmapLevel; // Can be used to improve performance by sampling higher level mipmaps.
    float3 pad11;
};

//#define DEBUG
//#define DEBUG2

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture            : register( t0 );
Texture2D<float4> g_normalTexture              : register( t1 ); 
Texture2D<float>  g_shadowTexture              : register( t2 ); 
Texture2D<float>  g_distToOccluderTexture      : register( t3 );
Texture2D<float>  g_finalDistToOccluderTexture : register( t4 );

// Input / Output.
RWTexture2D<uint> g_blurredShadowTexture : register( u0 );

// Blur kernel should cover a single shadow pattern area.
static const float samplingRadiusBase = 12.0;//8.0