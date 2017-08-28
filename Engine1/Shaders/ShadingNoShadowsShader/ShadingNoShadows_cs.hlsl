#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Shading.hlsl"

#define MAX_POINT_LIGHT_COUNT 50

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    uint   pointLightCount;
    float3 pad2;
    float4 pointLightPositions[ MAX_POINT_LIGHT_COUNT ];
    float4 pointLightColors[ MAX_POINT_LIGHT_COUNT ];
    float4 lightLinearAttenuationFactor[ MAX_POINT_LIGHT_COUNT ];    // YZW components are ignored.
    float4 lightQuadraticAttenuationFactor[ MAX_POINT_LIGHT_COUNT ]; // YZW components are ignored.
};

SamplerState g_linearSamplerState;

// Input.
Texture2D<float4> g_positionTexture  : register( t0 );
Texture2D<float4> g_albedoTexture    : register( t1 );
Texture2D<float>  g_metalnessTexture : register( t2 );
Texture2D<float>  g_roughnessTexture : register( t3 );
Texture2D<float4> g_normalTexture    : register( t4 ); 

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
    const float3 surfacePosition  = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo    = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceAlpha     = g_albedoTexture[ dispatchThreadId.xy ].w;
    const float  surfaceMetalness = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness = g_roughnessTexture[ dispatchThreadId.xy ];
    const float3 surfaceNormal    = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 surfaceDiffuseColor     = calculateSurfaceDiffuseColor( surfaceAlpha, surfaceAlbedo, surfaceMetalness );
    const float3 surfaceBaseReflectivity = calculateSurfaceBaseReflectivity( surfaceAlpha, surfaceAlbedo, surfaceMetalness );

    float4 outputColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );

    const float3 dirToCamera = normalize( cameraPos - surfacePosition );

    for ( uint i = 0; i < pointLightCount; ++i )
    {
        const float3 vectorToLight  = pointLightPositions[ i ].xyz - surfacePosition;
        const float3 dirToLight     = normalize( vectorToLight );
        const float  distToLight    = length( vectorToLight );

        float lightAttenuationFactor = 1 / (1 + lightLinearAttenuationFactor[ i ] * distToLight + lightQuadraticAttenuationFactor[ i ] * distToLight * distToLight );
        lightAttenuationFactor = ( lightAttenuationFactor - lightAttenuationFactorCutoff ) / ( 1.0 - lightAttenuationFactorCutoff );

        if (lightAttenuationFactor <= 0.0)
            continue;

        outputColor.rgb += calculateSurfaceLighting( pointLightColors[ i ].rgb * lightAttenuationFactor, surfaceNormal, dirToLight, dirToCamera,
                                                     surfaceDiffuseColor, surfaceBaseReflectivity, surfaceMetalness, surfaceRoughness );
    }

    g_colorTexture[ dispatchThreadId.xy ] += outputColor;
}