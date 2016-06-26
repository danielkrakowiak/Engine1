#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#define MAX_POINT_LIGHT_COUNT 50

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    uint   pointLightCount;
    float3 pad2;
    float4 pointLightPositions[ MAX_POINT_LIGHT_COUNT ];
    float4 pointLightColors[ MAX_POINT_LIGHT_COUNT ];
};

// Input.
Texture2D<float4> g_positionTexture          : register( t0 );
Texture2D<float4> g_emissiveTexture          : register( t1 );
Texture2D<float4> g_albedoTexture            : register( t2 );
Texture2D<float>  g_metalnessTexture         : register( t3 );
Texture2D<float>  g_roughnessTexture         : register( t4 );
Texture2D<float4> g_normalTexture            : register( t5 ); 
Texture2D<float>  g_indexOfRefractionTexture : register( t6 );

// Output.
RWTexture2D<float4> g_colorTexture : register( u0 );

float3 calculateDiffuseOutputColor( float3 surfaceDiffuseColor, float3 surfaceNormal, float3 lightColor, float3 dirToLight );
float3 calculateSpecularOutputColor( float3 surfaceSpecularColor, float3 dirToLight, float3 dirToCamera, float3 surfaceNormal );

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
    const float3 surfacePosition          = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceEmissive          = g_emissiveTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceAlbedo            = g_albedoTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceMetalness         = g_metalnessTexture[ dispatchThreadId.xy ];
    const float  surfaceRoughness         = g_roughnessTexture[ dispatchThreadId.xy ];
    const float3 surfaceNormal            = g_normalTexture[ dispatchThreadId.xy ].xyz;
    const float  surfaceIndexOfRefraction = g_indexOfRefractionTexture[ dispatchThreadId.xy ];
    
    float3 surfaceDiffuseColor  = (1.0f - surfaceMetalness) * surfaceAlbedo;
    float3 surfaceSpecularColor = surfaceMetalness * surfaceAlbedo;

    float4 outputColor = float4( surfaceEmissive, 1.0f );

    for ( uint i = 0; i < pointLightCount; ++i )
    {
        const float3 dirToLight  = normalize( pointLightPositions[ i ].xyz - surfacePosition );
        const float3 dirToCamera = normalize( cameraPos - surfacePosition );

        outputColor.rgb += calculateDiffuseOutputColor( surfaceDiffuseColor, surfaceNormal, pointLightColors[ i ].rgb, dirToLight );
        outputColor.rgb += calculateSpecularOutputColor( surfaceSpecularColor, surfaceNormal, dirToLight, dirToCamera );
    }

    g_colorTexture[ dispatchThreadId.xy ] = outputColor;
}

float3 calculateDiffuseOutputColor( float3 surfaceDiffuseColor, float3 surfaceNormal, float3 lightColor, float3 dirToLight )
{
    return max( 0.0f, dot( dirToLight, surfaceNormal ) ) * surfaceDiffuseColor * lightColor;
}

// Schlick Approximation.
float3 calculateFresnel( float3 specularColor, float3 dirToLight, float3 halfDir )
{
    return specularColor + (1.0f - specularColor) * pow(1.0f - dot(dirToLight, halfDir), 5.0f);
}

float3 calculateSpecularOutputColor( float3 surfaceSpecularColor, float3 surfaceNormal, float3 dirToLight, float3 dirToCamera )
{
    float3 halfDir = normalize( dirToLight + dirToCamera );

    float3 fresnel = calculateFresnel( surfaceSpecularColor, dirToLight, halfDir );

    return max( 0.0f, dot( halfDir, surfaceNormal ) ) * surfaceSpecularColor * fresnel;
}
