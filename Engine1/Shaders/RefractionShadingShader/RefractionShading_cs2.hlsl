#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

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
RWTexture2D<float4> g_contributionTermRoughnessTexture : register( u0 ); 

float calculateReflectionTerm( float3 incidentRay, float3 surfaceNormal, float refractionIndex1, float refractionIndex2 );

static const float3 dielectricSpecularColor = float3( 0.04f, 0.04f, 0.04f );

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
    const float refractionTerm = ( 1.0f - surfaceAlpha ) * ( 1.0f - calculateReflectionTerm( -dirToCamera, surfaceNormal, 1.0f, 1.3f ) );

    const float4 prevContributionTermRoughness = g_prevContributionTermRoughnessTexture[ dispatchThreadId.xy ];

    // Calculate how much of the incoming reflection light is visible at the camera (after all the light bounces).
    g_contributionTermRoughnessTexture[ dispatchThreadId.xy ] = float4( prevContributionTermRoughness.rgb * refractionTerm, min( 1.0f, prevContributionTermRoughness.a + surfaceRoughness ) );
}

// Schlick Approximation.
float calculateReflectionTerm( float3 incidentRay, float3 surfaceNormal, float refractionIndex1, float refractionIndex2 )
{
    float r0 = (refractionIndex1 - refractionIndex2) / (refractionIndex1 + refractionIndex2);
    r0 *= r0;
    float cosX = -dot( surfaceNormal, incidentRay );
    if ( refractionIndex1 > refractionIndex2 )
    {
        const float n = refractionIndex1 / refractionIndex2;
        const float sinT2 = n * n * ( 1.0f - cosX * cosX );

        if ( sinT2 > 1.0f )
            return 1.0f; // Total internal reflection.

        cosX = sqrt( 1.0f - sinT2 );
    }

    const float x = 1.0f - cosX;

    return r0 + (1.0f - r0) * x * x * x * x * x;
}