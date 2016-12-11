#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

SamplerState g_linearSamplerState : register( s0 );

cbuffer ConstantBuffer
{
    float2 outputTextureSize;
    float2 pad1;
};

// Input.
Texture2D<float4> g_raysDirection    : register( t0 );
Texture2D<float4> g_surfacePosition  : register( t1 );
Texture2D<float4> g_surfaceNormal    : register( t2 );
Texture2D<float>  g_surfaceRoughness : register( t3 ); // Used to decide whether to generate reflected/refracted ray. If roughness > 0.999, ray is not generated.
Texture2D<float4> g_contributionTerm : register( t4 ); // How much of the ray color is visible by the camera. Used to avoid generating useless rays.

// Output.
RWTexture2D<float4> g_rayOrigin    : register( u0 );
RWTexture2D<float4> g_rayDirection : register( u1 );

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

static const float requiredContributionTerm = 0.35f; // Discard rays which color is visible in less than 5% by the camera.

float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal );

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

    const float3 surfacePosition  = g_surfacePosition.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
    const float  surfaceRoughness = g_surfaceRoughness.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    const float3 contributionTerm = g_contributionTerm.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    // TODO: Could be otpimized to check only roughness (not position). Roughness buffer needs to be filled with maximal value at the beginning of each frame.
    if ( !any( surfacePosition ) || dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm || g_surfaceRoughness[ dispatchThreadId.xy ] > 0.999f ) { // If all position components are zeros or roughness is maximal - there is no reflected ray.
        // Deactivate the ray.
        g_rayDirection[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
        return;
    }

    const float3 rayDir = g_raysDirection.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    float3 surfaceNormal = g_surfaceNormal.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    const bool frontHit = dot( rayDir, surfaceNormal ) < 0.0f;

    if ( !frontHit ) {
        surfaceNormal = -surfaceNormal;
    }

    const float3 secondaryRayDir    = calcReflectedRay( rayDir, surfaceNormal );
    const float3 secondaryRayOrigin = surfacePosition + secondaryRayDir * 0.01f; // Modify ray origin to avoid self-collisions.

    g_rayOrigin[ dispatchThreadId.xy ]    = float4( secondaryRayOrigin, 0.0f );
    g_rayDirection[ dispatchThreadId.xy ] = float4( secondaryRayDir, 0.0f );
}

// Reflects the vector which represents an incident ray hitting a surface.
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal )
{
    return incidentRay - 2.0f * surfaceNormal * dot( surfaceNormal, incidentRay );
}

//refractiveIndex = refractiveIndex1(incident) / refractiveIndex2(refracted)
//return false if there is no refraction

// TODO: use struct to return bool + float3
// bool calcRefractedRay(const Vec3& incidentRay, const Vec3& surfaceNormal, float refractiveIndex, Vec3* refractedRay);

