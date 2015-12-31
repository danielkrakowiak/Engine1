#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float3   rayOrigin;      // Ray origin in world space.
    float    pad1;
    float4x4 worldMatrixInv; // Transform from world to local space.
    float3   boundingBoxMin; // In local space.
    float    pad2;
    float3   boundingBoxMax; // In local space.
    float    pad3;
};

Texture2D<float4>   rayDirections : register( t0 );
RWTexture2D<float4> computeTarget : register( u0 );

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
    float3 rayDir = rayDirections.Load( int3( dispatchThreadId.xy, 0 ) ).xyz;

    // Transform the ray from world to local space.
	float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldMatrixInv );
	float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldMatrixInv ) - rayOriginLocal;

    // Test the ray against the bounding box
	float tmin = -15000.0f;
	float tmax =  15000.0f;
 
	float3 t1 = ( boundingBoxMin - rayOriginLocal ) / rayDirLocal;
	float3 t2 = ( boundingBoxMax - rayOriginLocal ) / rayDirLocal;
 
	tmin = max(tmin, min(t1.x, t2.x));
	tmax = min(tmax, max(t1.x, t2.x));
	tmin = max(tmin, min(t1.y, t2.y));
	tmax = min(tmax, max(t1.y, t2.y));
	tmin = max(tmin, min(t1.z, t2.z));
	tmax = min(tmax, max(t1.z, t2.z));

    float4 result = float4( 0.2f, 0.2f, 0.2f, 1.0f );
	
    if( tmax >= tmin && tmax > 0.0f ) {
        result.x = 1.0f;
    }

    computeTarget[ dispatchThreadId.xy ] = result;
}



