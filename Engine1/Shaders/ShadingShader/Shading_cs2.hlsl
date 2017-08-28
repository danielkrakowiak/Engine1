#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Shading.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float4 lightPosition;
    float4 lightColor;
    float  lightLinearAttenuationFactor;
    float3 pad1;
    float  lightQuadraticAttenuationFactor;
    float3 pad2;
    float2 outputTextureSize;
    float2 pad3;
};

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_rayOriginTexture         : register( t0 );
Texture2D<float4> g_positionTexture          : register( t1 );
Texture2D<float4> g_albedoTexture            : register( t2 );
Texture2D<float>  g_metalnessTexture         : register( t3 );
Texture2D<float>  g_roughnessTexture         : register( t4 );
Texture2D<float4> g_normalTexture            : register( t5 ); 
Texture2D<float>  g_shadowTexture      : register( t6 );

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

    const float3 rayOrigin                = g_rayOriginTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfacePosition          = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo            = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceAlpha             = g_albedoTexture[ dispatchThreadId.xy ].w;
    const float  surfaceMetalness         = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness         = g_roughnessTexture[ dispatchThreadId.xy ];
    const float3 surfaceNormal            = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = rayOrigin - surfacePosition;
    const float3 dirToCamera    = normalize( vectorToCamera );

    const float3 vectorToLight  = lightPosition.xyz - surfacePosition;
    const float3 dirToLight     = normalize( vectorToLight );
    const float  distToLight    = length( vectorToLight );

    float lightAttenuationFactor = 1 / (1 + lightLinearAttenuationFactor * distToLight + lightQuadraticAttenuationFactor * distToLight * distToLight );
    lightAttenuationFactor = ( lightAttenuationFactor - lightAttenuationFactorCutoff ) / ( 1.0 - lightAttenuationFactorCutoff );

    if (lightAttenuationFactor <= 0.0)
        return;

	const float surfaceIllumination = 1.0f - g_shadowTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f );
    
    if (surfaceIllumination <= 0.0)
        return;

    const float3 surfaceDiffuseColor     = calculateSurfaceDiffuseColor( surfaceAlpha, surfaceAlbedo, surfaceMetalness );
    const float3 surfaceBaseReflectivity = calculateSurfaceBaseReflectivity( surfaceAlpha, surfaceAlbedo, surfaceMetalness );

    float4 outputColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );

    outputColor.rgb += calculateSurfaceLighting( lightColor.rgb * lightAttenuationFactor * surfaceIllumination, surfaceNormal, dirToLight, dirToCamera,
                                                 surfaceDiffuseColor, surfaceBaseReflectivity, surfaceMetalness, surfaceRoughness );

    g_colorTexture[ dispatchThreadId.xy ] += outputColor;
}