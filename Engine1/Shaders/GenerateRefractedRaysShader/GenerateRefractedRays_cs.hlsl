#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Utils.hlsl"

SamplerState g_linearSamplerState : register( s0 );

cbuffer ConstantBuffer
{
    uint   refractionLevel;
    float3 pad1;
    float2 outputTextureSize;
    float2 pad2;
};

// Input.
Texture2D<float4> g_raysDirection          : register( t0 );
Texture2D<float4> g_surfacePosition        : register( t1 );
Texture2D<float4> g_surfaceNormal          : register( t2 );
Texture2D<float>  g_surfaceRoughness       : register( t3 ); // Used to decide whether to generate reflected/refracted ray. If roughness > 0.999, ray is not generated.
Texture2D<float>  g_surfaceRefractiveIndex : register( t4 );
Texture2D<float4> g_contributionTerm       : register( t5 ); // How much of the ray color is visible by the camera. Used to avoid generating useless rays.
Texture2D<float>  g_prevRefractiveIndex    : register( t6 ); // Refractive index of the previous incident rays.
Texture2D<float>  g_currentRefractiveIndex : register( t7 ); // Refractive index of the incident rays.

// Output.
RWTexture2D<float4> g_rayOrigin           : register( u0 );
RWTexture2D<float4> g_rayDirection        : register( u1 );
RWTexture2D<float>  g_nextRefractiveIndex : register( u2 ); // Refractive index of the generated rays.

static const float requiredContributionTerm = 0.05f; // Discard rays which color is visible in less than 5% by the camera.
static const float refractiveIndexMul = 2.0f;
static const float rayOriginOffset = 0.0001f; // Used to move refracted ray origin along the ray direction to avoid self-collisions.

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(32, 32, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    const float3 surfacePosition            = g_surfacePosition.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
    const float  surfaceRoughness           = g_surfaceRoughness.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    const float  surfaceRefractiveIndexNorm = g_surfaceRefractiveIndex.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    const float  surfaceRefractiveIndex     = 1.0f + surfaceRefractiveIndexNorm * refractiveIndexMul;
    const float3 contributionTerm           = g_contributionTerm.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    // TODO: Could be otpimized to check only roughness (not position). Roughness buffer needs to be filled with maximal value at the beginning of each frame.
    if ( !any( surfacePosition ) || dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm || g_surfaceRoughness[ dispatchThreadId.xy ] > 0.999f ) { // If all position components are zeros or roughness is maximal - there is no reflected ray.
        // Deactivate the ray.
        g_rayDirection[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
        return;
    }

    const float3 rayDir = g_raysDirection[ dispatchThreadId.xy ].xyz;

    float3 surfaceNormal = g_surfaceNormal[ dispatchThreadId.xy ].xyz;

    const bool frontHit = dot(rayDir, surfaceNormal) < 0.0f;
    
    const float currentRefractiveIndex = 1.0f + g_currentRefractiveIndex[ dispatchThreadId.xy ] * refractiveIndexMul;
    float refractiveIndex = 1.0f;

    if ( frontHit ) {
        g_nextRefractiveIndex[ dispatchThreadId.xy ] = surfaceRefractiveIndexNorm;

        refractiveIndex = currentRefractiveIndex / surfaceRefractiveIndex;

    } else {
        const float prevRefractiveIndexNorm = g_prevRefractiveIndex[ dispatchThreadId.xy ];
        const float prevRefractiveIndex = 1.0f + prevRefractiveIndexNorm * refractiveIndexMul;

        g_nextRefractiveIndex[ dispatchThreadId.xy ] = prevRefractiveIndexNorm; //prevRefractiveIndex; // Temporary. From lack of better idea.

        refractiveIndex = currentRefractiveIndex / prevRefractiveIndex;

        surfaceNormal = -surfaceNormal; 
    }

    const float3 secondaryRayDir    = calcRefractedRay( rayDir, surfaceNormal, refractiveIndex );
    const float3 secondaryRayOrigin = surfacePosition + secondaryRayDir * rayOriginOffset; // Modify ray origin to avoid self-collisions.

    g_rayOrigin[ dispatchThreadId.xy ]    = float4( secondaryRayOrigin, 0.0f );
    g_rayDirection[ dispatchThreadId.xy ] = float4( secondaryRayDir, 0.0f );
}

