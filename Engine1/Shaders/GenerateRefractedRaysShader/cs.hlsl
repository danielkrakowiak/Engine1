#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<float4> g_raysDirection    : register( t0 );
Texture2D<float4> g_surfacePosition  : register( t1 );
Texture2D<float4> g_surfaceNormal    : register( t2 );
Texture2D<float>  g_surfaceRoughness : register( t3 ); // Used to decide whether to generate reflected/refracted ray. If roughness > 0.999, ray is not generated.
Texture2D<float4> g_reflectionTerm   : register( t4 ); // How much of the ray color is visible by the camera. Used to avoid generating useless rays.

// Output.
RWTexture2D<float4> g_rayOrigin    : register( u0 );
RWTexture2D<float4> g_rayDirection : register( u1 );

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

static const float requiredReflectionTerm = 0.05f; // Discard rays which color is visible in less than 5% by the camera.

float3 calcRefractedRay( float3 incidentRay, float3 surfaceNormal, float refractiveIndex );

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
    const float2 pixelPos = (float2)dispatchThreadId.xy;

    const float3 surfacePosition  = g_surfacePosition[ dispatchThreadId.xy ].xyz;
    const float  surfaceRoughness = g_surfaceRoughness[ dispatchThreadId.xy ];
    const float3 reflectionTerm   = g_reflectionTerm[ dispatchThreadId.xy ].xyz;

    // TODO: Could be otpimized to check only roughness (not position). Roughness buffer needs to be filled with maximal value at the beginning of each frame.
    if ( !any( surfacePosition ) || dot( float3( 1.0f, 1.0f, 1.0f ), reflectionTerm ) < requiredReflectionTerm || g_surfaceRoughness[ dispatchThreadId.xy ] > 0.999f ) { // If all position components are zeros or roughness is maximal - there is no reflected ray.
        // Deactivate the ray.
        g_rayDirection[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
        return;
    }

    const float3 rayDir = g_raysDirection[ dispatchThreadId.xy ].xyz;

    float3 surfaceNormal = g_surfaceNormal[ dispatchThreadId.xy ].xyz;

    if ( dot(rayDir, surfaceNormal) > 0.0f )
        surfaceNormal = -surfaceNormal;

    const float3 secondaryRayDir    = calcRefractedRay( rayDir, surfaceNormal, 0.90f );
    const float3 secondaryRayOrigin = surfacePosition + secondaryRayDir * 0.01f; // Modify ray origin to avoid self-collisions.

    g_rayOrigin[ dispatchThreadId.xy ]    = float4( secondaryRayOrigin, 0.0f );
    g_rayDirection[ dispatchThreadId.xy ] = float4( secondaryRayDir, 0.0f );
}

// Returns refracted ray or zero vector if there is no refraction.
// refractiveIndex = refractiveIndex1(incident) / refractiveIndex2(refracted)
float3 calcRefractedRay( float3 incidentRay, float3 surfaceNormal, float refractiveIndex )
{
	const float cosI  = -dot( surfaceNormal, incidentRay );
	const float sinT2 = ( refractiveIndex * refractiveIndex ) * ( 1.0f - cosI * cosI );
	
    if ( sinT2 > 1.0f ) 
        return float3( 0.0f, 0.0f, 0.0f ); // Deactivate the ray.

	const float cosT = sqrt( 1.0f - sinT2 );
	
	return refractiveIndex * incidentRay + ( refractiveIndex * cosI - cosT ) * surfaceNormal;
}

