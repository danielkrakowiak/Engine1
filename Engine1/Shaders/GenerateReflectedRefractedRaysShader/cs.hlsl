#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

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
};

// Input.
Texture2D<float4> g_depth         : register( t0 );
Texture2D<float2> g_surfaceNormal : register( t1 );

// Output.
RWTexture2D<float4> g_rayOrigin    : register( u0 );
RWTexture2D<float4> g_rayDirection : register( u1 );

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal );
float linearizeDepth( float depthSample );

// pixelPos in range (0,0; screen width, screen height) counting from the top-left corner of the viewport.
// Returns not normalized direction - vector from camera origin to imaginary viewport plane through the given pixel.
float3 getPrimaryRayVector( float2 pixelPos )
{
    // cameraPos - camera position in world space.
    // viewportCenter - viewport plane center in world space.
    // viewportUp - viewport up vector in world space. It's length equals half of height of the viewport plane.
    // viewportRight - viewport right vector in world space. It's length equals half of width of the viewport plane.
    // viewportSizeHalf - viewport dimensions in pixels divided by 2.

    const float2 pixelShift = (pixelPos - viewportSizeHalf + float2(0.5f, 0.5f)) / viewportSizeHalf; // In range (-1;1)

	const float3 pixelPosWorld = viewportCenter + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    return pixelPosWorld - cameraPos;
}

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

    const float3 primaryRayVec = getPrimaryRayVector( pixelPos ); 
    const float3 primaryRayDir = normalize( primaryRayVec );

    const float  depth = linearizeDepth( g_depth.Load( float3( pixelPos, 0.0f ) ).x ); // TODO: Maybe could use bilinear sampling if vieport sizes differs for primary and secondary rays?
    
    float3 surfaceNormal = float3( g_surfaceNormal[ pixelPos ], 0.0f );
    surfaceNormal.z = sqrt( 1.0f - surfaceNormal.x*surfaceNormal.x - surfaceNormal.y*surfaceNormal.y );

    const float3 secondaryRayOrigin = cameraPos + /*primaryRayVec +*/ primaryRayDir * depth;
    const float3 secondaryRayDir    = calcReflectedRay( primaryRayDir, surfaceNormal );

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

float linearizeDepth( float depthSample )
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / ( zFar + zNear - depthSample * ( zFar - zNear ) );

    return zLinear;
}


