#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Utils.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    float4 lightPosition;
    float4 lightColor;
    float2 outputTextureSize;
    float2 pad2;
};

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture           : register( t0 );
Texture2D<float4> g_albedoTexture             : register( t1 );
Texture2D<float>  g_metalnessTexture          : register( t2 );
Texture2D<float>  g_roughnessTexture          : register( t3 );
Texture2D<float4> g_normalTexture             : register( t4 ); 
Texture2D<float>  g_shadowTexture       : register( t5 ); 

// Input / Output.
RWTexture2D<float4> g_colorTexture : register( u0 );

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
    //const float2 outputTextureHalfPixelSize = 1.0f / outputTextureSize; // Should be illumination texture size?

    const float2 pixelSize0 = 1.0f / outputTextureSize;

    const float3 surfacePosition     = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo       = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceAlpha        = g_albedoTexture[ dispatchThreadId.xy ].w;
    const float  surfaceMetalness    = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness    = g_roughnessTexture[ dispatchThreadId.xy ];
    const float3 surfaceNormal       = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = cameraPos - surfacePosition;
    const float3 dirToCamera    = normalize( vectorToCamera );

    const float3 vectorToLight       = lightPosition.xyz - surfacePosition;
    const float3 dirToLight          = normalize( vectorToLight );

    const float surfaceIllumination = 1.0f - g_shadowTexture.SampleLevel( g_pointSamplerState, texcoords/* + pixelSize0*/, 0.0f ); // PROBLEM: Input texture is a bit shifted compared to other image textures...

    float3 surfaceDiffuseColor  = surfaceAlpha * (1.0f - surfaceMetalness) * surfaceAlbedo;
    float3 surfaceSpecularColor = surfaceAlpha * lerp( dielectricSpecularColor, surfaceAlbedo, surfaceMetalness );

    float4 outputColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );//float4( surfaceEmissive, 1.0f );

    outputColor.rgb += calculateDiffuseOutputColor( surfaceDiffuseColor, surfaceNormal, lightColor.rgb * surfaceIllumination, dirToLight );
    outputColor.rgb += calculateSpecularOutputColor( surfaceSpecularColor, surfaceRoughness, surfaceNormal, lightColor.rgb * surfaceIllumination, dirToLight, dirToCamera );

    g_colorTexture[ dispatchThreadId.xy ] += outputColor;
}