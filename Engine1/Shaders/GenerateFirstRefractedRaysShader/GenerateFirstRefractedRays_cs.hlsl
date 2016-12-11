#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

SamplerState g_linearSamplerState : register( s0 );

cbuffer ConstantBuffer
{
    float3 cameraPos;
    float  pad1;
    float3 viewportCenter;
    float  pad2;
    float3 viewportUp;
    float  pad3;
    float3 viewportRight;
    float  pad4;
    float2 viewportSizeHalf;
    float2 pad5;
    float2 outputTextureSize;
    float2 pad6;
};

// Input.
Texture2D<float4> g_surfacePosition                           : register( t0 );
Texture2D<float4> g_surfaceNormal                             : register( t1 );
Texture2D<float>  g_surfaceRoughness                          : register( t2 ); // Used to decide whether to generate reflected/refracted ray. If roughness > 0.999, ray is not generated.
Texture2D<float>  g_surfaceRefractiveIndex                    : register( t3 );
Texture2D<float4> g_contributionTerm                          : register( t4 ); // How much of the ray color is visible by the camera. Used to avoid generating useless rays.

// Output.
RWTexture2D<float4> g_rayOrigin           : register( u0 );
RWTexture2D<float4> g_rayDirection        : register( u1 );
RWTexture2D<float>  g_nextRefractiveIndex : register( u2 ); // Refractive index of the generated rays.

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

static const float requiredContributionTerm = 0.05f; // Discard rays which color is visible in less than 5% by the camera.
static const float refractiveIndexMul = 2.0f;

float3 getPrimaryRayDirection( float2 pixelPos );
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
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    const float2 pixelPos = (float2)dispatchThreadId.xy;

    const float3 surfacePosition            = g_surfacePosition.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;
    const float  surfaceRoughness           = g_surfaceRoughness.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    const float  surfaceRefractiveIndexNorm = g_surfaceRefractiveIndex.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    const float  surfaceRefractiveIndex     = 1.0f + surfaceRefractiveIndexNorm * refractiveIndexMul;
    const float3 contributionTerm           = g_contributionTerm.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    // TODO: Could be otpimized to check only roughness (not position). Roughness buffer needs to be filled with maximal value at the beginning of each frame.
    if ( !any( surfacePosition ) || dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm || surfaceRoughness > 0.999f ) { // If all position components are zeros or roughness is maximal - there is no reflected ray.
        // Deactivate the ray.
        g_rayDirection[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
        return;
    }

    const float3 primaryRayDir = getPrimaryRayDirection( pixelPos );

    float3 surfaceNormal = g_surfaceNormal.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    const bool frontHit = dot( primaryRayDir, surfaceNormal ) < 0.0f;

    const float prevRefractiveIndex = 1.0f; // Temporary. Should be passed in constant buffer.
    const float prevRefractiveIndexNorm = 0.0f;

    if ( frontHit ) {
        g_nextRefractiveIndex[ dispatchThreadId.xy ] = surfaceRefractiveIndexNorm;
    } else {
        g_nextRefractiveIndex[ dispatchThreadId.xy ] = prevRefractiveIndexNorm; // Temporary. From lack of better idea.
        
        surfaceNormal = -surfaceNormal;
    }

    const float3 secondaryRayDir    = calcRefractedRay( primaryRayDir, surfaceNormal, prevRefractiveIndex / surfaceRefractiveIndex );
    const float3 secondaryRayOrigin = surfacePosition + secondaryRayDir * 0.01f; // Modify ray origin to avoid self-collisions.

    g_rayOrigin[ dispatchThreadId.xy ]    = float4( secondaryRayOrigin, 0.0f );
    g_rayDirection[ dispatchThreadId.xy ] = float4( secondaryRayDir, 0.0f );
}

// pixelPos in range (0,0; screen width, screen height) counting from the top-left corner of the viewport.
float3 getPrimaryRayDirection( float2 pixelPos )
{
    // cameraPos - camera position in world space.
    // viewportCenter - viewport plane center in world space.
    // viewportUp - viewport up vector in world space. It's length equals half of height of the viewport plane.
    // viewportRight - viewport right vector in world space. It's length equals half of width of the viewport plane.
    // viewportSizeHalf - viewport dimensions in pixels divided by 2.

    const float2 pixelShift = (pixelPos - viewportSizeHalf + float2(0.5f, 0.5f)) / viewportSizeHalf; // In range (-1;1)

	const float3 pixelPosWorld = viewportCenter + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    return normalize( pixelPosWorld - cameraPos );
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

