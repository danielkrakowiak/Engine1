#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Utils.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
};

// Input.
Texture2D<float4> g_positionTexture                    : register( t0 );
Texture2D<float4> g_normalTexture                      : register( t1 ); 
Texture2D<float4> g_albedoTexture                      : register( t2 );
Texture2D<float>  g_metalnessTexture                   : register( t3 );
Texture2D<float>  g_roughnessTexture                   : register( t4 );

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
    const float3 surfacePosition  = g_positionTexture[ dispatchThreadId.xy ].xyz;
    float3       surfaceNormal    = g_normalTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo    = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    //const float  surfaceAlpha     = g_albedoTexture[ dispatchThreadId.xy ].w; // #TODO: Is alpha important here? Does it influence how much reflection is going?
    const float  surfaceMetalness = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness = g_roughnessTexture[ dispatchThreadId.xy ];
    
    float3 surfaceSpecularColor = lerp( dielectricSpecularColor, surfaceAlbedo, surfaceMetalness );

    const float3 dirToCamera = normalize( cameraPos - surfacePosition );

    // Invert normal if surface was hit in the backface.
    if ( dot( dirToCamera, surfaceNormal ) < 0.0f )
        surfaceNormal = -surfaceNormal;

    const float3 dirToReflectionSource = normalize( calcReflectedRay( -dirToCamera, surfaceNormal ) );

    // Calculate how much of the incoming reflection light is visible from the ray origin (at current light bounce level).
    const float3 reflectionTerm = calculateReflectionTerm( surfaceSpecularColor, surfaceNormal, dirToCamera );

    // Calculate how much of the incoming reflection light is visible at the camera (after all the light bounces).
    g_contributionTermRoughnessTexture[ dispatchThreadId.xy ] = (uint4)(float4( reflectionTerm, min( 1.0f, surfaceRoughness ) ) * 255.0);
}