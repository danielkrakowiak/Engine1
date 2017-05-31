#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Utils.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    int    level;
    float3 pad2;
};

// Input.
Texture2D<float4> g_rayOriginTexture                     : register( t0 );
Texture2D<float4> g_positionTexture                      : register( t1 );
Texture2D<float4> g_normalTexture                        : register( t2 ); 
Texture2D<float4> g_albedoTexture                        : register( t3 );
Texture2D<float>  g_metalnessTexture                     : register( t4 );
Texture2D<float>  g_roughnessTexture                     : register( t5 );
Texture2D<float4> g_prevContributionTermRoughnessTexture : register( t6 );

// Output.
RWTexture2D<uint4> g_contributionTermRoughnessTexture : register( u0 ); 

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
    const float3 rayOrigin        = g_rayOriginTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfacePosition  = g_positionTexture[ dispatchThreadId.xy ].xyz;
    float3       surfaceNormal    = g_normalTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo    = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceAlpha     = g_albedoTexture[ dispatchThreadId.xy ].w;
    const float  surfaceMetalness = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness = g_roughnessTexture[ dispatchThreadId.xy ];

    // Completaly opaque surface - no contribution from refracted rays.
    if ( surfaceAlpha >= 0.999f ) {
        g_contributionTermRoughnessTexture[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 1.0f );
        return;
    }

    const float3 dirToCamera = normalize( rayOrigin - surfacePosition );

    // Invert normal if surface was hit in the backface.
    if ( dot( dirToCamera, surfaceNormal ) < 0.0f )
        surfaceNormal = -surfaceNormal;

    // Calculate how much of the incoming reflection light is visible from the ray origin (at current light bounce level).
    const float refractionTerm = ( 1.0f - surfaceAlpha ) * ( 1.0f - calculateRefractionTerm( -dirToCamera, surfaceNormal, 1.0f, 1.3f ) );

    const float4 prevContributionTermRoughness = g_prevContributionTermRoughnessTexture[ dispatchThreadId.xy ];

    // Calculate how much of the incoming reflection light is visible at the camera (after all the light bounces).
    g_contributionTermRoughnessTexture[ dispatchThreadId.xy ] = (uint4)(float4( prevContributionTermRoughness.rgb * refractionTerm, min( 1.0f, prevContributionTermRoughness.a + surfaceRoughness ) ) * 255.0);
}