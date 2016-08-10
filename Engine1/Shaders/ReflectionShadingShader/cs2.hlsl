#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<float4> g_rayOriginTexture                   : register( t0 );
Texture2D<float4> g_positionTexture                    : register( t1 );
Texture2D<float4> g_normalTexture                      : register( t2 ); 
Texture2D<float4> g_albedoTexture                      : register( t3 );
Texture2D<float>  g_metalnessTexture                   : register( t4 );
Texture2D<float>  g_roughnessTexture                   : register( t5 );
Texture2D<float4> g_prevContributionTermRoughnessTexture : register( t6 );

// Output.
RWTexture2D<float4> g_contributionTermRoughnessTexture : register( u0 ); 


float3 calculateReflectionTerm( float3 surfaceSpecularColor, float3 surfaceNormal, float3 dirToCamera );
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal );

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
    //const float  surfaceAlpha     = g_albedoTexture[ dispatchThreadId.xy ].w;
    const float  surfaceMetalness = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness = g_roughnessTexture[ dispatchThreadId.xy ];
    
    float3 surfaceSpecularColor = lerp( dielectricSpecularColor, surfaceAlbedo, surfaceMetalness );

    const float3 dirToCamera = normalize( rayOrigin - surfacePosition );

    // Invert normal if surface was hit in the backface.
    if ( dot( dirToCamera, surfaceNormal ) < 0.0f )
        surfaceNormal = -surfaceNormal;

    const float3 dirToReflectionSource = normalize( calcReflectedRay( -dirToCamera, surfaceNormal ) );

    // Calculate how much of the incoming reflection light is visible from the ray origin (at current light bounce level).
    const float3 reflectionTerm = calculateReflectionTerm( surfaceSpecularColor, surfaceNormal, dirToCamera );

    const float4 prevContributionTermRoughness = g_prevContributionTermRoughnessTexture[ dispatchThreadId.xy ];

    // Calculate how much of the incoming reflection light is visible at the camera (after all the light bounces).
    g_contributionTermRoughnessTexture[ dispatchThreadId.xy ] = float4( prevContributionTermRoughness.rgb * reflectionTerm, min( 1.0f, prevContributionTermRoughness.a + surfaceRoughness ) );
}

float3 calculateReflectionTerm( float3 surfaceSpecularColor, float3 surfaceNormal, float3 dirToCamera )
{
    const float normalViewDot = dot( surfaceNormal, dirToCamera );

    // Fresnel term. Schlick Approximation.
    const float3 fresnel = surfaceSpecularColor + (( 1.0f - surfaceSpecularColor ) * pow( 1.0f - max( 0.0f, normalViewDot), 5.0f ));

    // We normalize specular color because it doesn't influence the amount of reflection directly (only through fresnel), but rather the tint of it.
    // We lerp, bacause at glancing angles, the light color is unaltered in reflection, but at normal incidence the light is modulated by the color of the metal.
    // #TODO: Should the same theory be applied to specular shading?
    const float3 tintColor = lerp( float3( 1.0f, 1.0f, 1.0f ), normalize(surfaceSpecularColor), normalViewDot );
    
    return fresnel * tintColor;
}

// Reflects the vector which represents an incident ray hitting a surface.
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal )
{
    return incidentRay - 2.0f * surfaceNormal * dot( surfaceNormal, incidentRay );
}