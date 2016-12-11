#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

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
Texture2D<float>  g_illuminationTexture       : register( t5 ); 

// Input / Output.
RWTexture2D<float4> g_colorTexture : register( u0 );

float  readDistToOccluder( float2 texcoords );
float3 calculateDiffuseOutputColor( float3 surfaceDiffuseColor, float3 surfaceNormal, float3 lightColor, float3 dirToLight );
float3 calculateSpecularOutputColor( float3 surfaceSpecularColor, float surfaceRoughness, float3 surfaceNormal, float3 lightColor, float3 dirToLight, float3 dirToCamera );

static const float Pi = 3.14159265f;

static const float3 dielectricSpecularColor = float3( 0.04f, 0.04f, 0.04f );

static const float maxDistToOccluder = 999.0f; // Every distance-to-occluder sampled from texture, which is greater than that is not a real value - rather a missing value.

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
    //const float2 texcoords = dispatchThreadId.xy / outputTextureSize;
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

    const float surfaceIllumination = g_illuminationTexture.SampleLevel( g_pointSamplerState, texcoords/* + pixelSize0*/, 0.0f ); // PROBLEM: Input texture is a bit shifted compared to other image textures...

    float3 surfaceDiffuseColor  = surfaceAlpha * (1.0f - surfaceMetalness) * surfaceAlbedo;
    float3 surfaceSpecularColor = surfaceAlpha * lerp( dielectricSpecularColor, surfaceAlbedo, surfaceMetalness );

    float4 outputColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );//float4( surfaceEmissive, 1.0f );

    outputColor.rgb += calculateDiffuseOutputColor( surfaceDiffuseColor, surfaceNormal, lightColor.rgb * surfaceIllumination, dirToLight );
    outputColor.rgb += calculateSpecularOutputColor( surfaceSpecularColor, surfaceRoughness, surfaceNormal, lightColor.rgb * surfaceIllumination, dirToLight, dirToCamera );

    g_colorTexture[ dispatchThreadId.xy ] += outputColor;
}

float3 calculateDiffuseOutputColor( float3 surfaceDiffuseColor, float3 surfaceNormal, float3 lightColor, float3 dirToLight )
{
    return max( 0.0f, dot( dirToLight, surfaceNormal ) ) * surfaceDiffuseColor * lightColor;
}

// Schlick Approximation.
float3 calculateFresnel( float3 specularColor, float3 dirToLight, float3 halfDir )
{
    return specularColor + (( 1.0f - specularColor ) * pow( 1.0f - max( 0.0f, dot( dirToLight, halfDir )), 5.0f ));
}

float3 calculateSpecularOutputColor( float3 surfaceSpecularColor, float surfaceRoughness, float3 surfaceNormal, float3 lightColor, float3 dirToLight, float3 dirToCamera )
{
    // Source: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

    float3 halfDir = normalize( dirToLight + dirToCamera );

    // Normal distribution function term.
    const float a = surfaceRoughness * surfaceRoughness;
    const float aSqr = a * a;
    const float normalHalfDot = saturate( dot( surfaceNormal, halfDir ) );
    const float b = ( normalHalfDot * normalHalfDot * ( aSqr - 1.0f ) + 1.0f );
    const float normalDistribution = aSqr / ( /*Pi **/ b * b );

    // Specular geometric attenuation term.
    const float k = (( surfaceRoughness + 1.0f ) * ( surfaceRoughness + 1.0f )) / 8.0f;
    const float normalViewDot = saturate( dot( surfaceNormal, dirToCamera ) );
    const float gv = normalViewDot / ( normalViewDot * ( 1.0f - k ) + k );
    const float normalLightDot = saturate( dot( surfaceNormal, dirToLight ) );
    const float gl = normalLightDot / ( normalLightDot * ( 1.0f - k ) + k );
    const float geometricAttenuation = gv * gl;

    // Fresnel term. Schlick Approximation.
    const float3 fresnel = surfaceSpecularColor + (( 1.0f - surfaceSpecularColor ) * pow( 1.0f - max( 0.0f, dot( dirToCamera, halfDir )), 5.0f ));

    const float3 specular = ( normalDistribution * fresnel * geometricAttenuation ) / ( 4.0f * ( normalLightDot * normalViewDot ) );

    return max( 0.0f, specular * normalLightDot * surfaceSpecularColor * lightColor );
}
