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

// Input.
Texture2D<float4> rayDirections : register( t0 );
ByteAddressBuffer meshVertices  : register( t1 );
//Buffer<float3>    meshNormals   : register( t2 );
//Buffer<float2>    meshTexcoords : register( t3 );
ByteAddressBuffer meshTriangles : register( t2 );

// Output.
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
	float4 rayOriginLocal = mul( float4( rayOrigin, 1.0f ), worldMatrixInv ); //#TODO: ray origin could be passed in local space to avoid this calculation.
	float4 rayDirLocal    = mul( float4( rayOrigin + rayDir, 1.0f ), worldMatrixInv ) - rayOriginLocal;

    // Test the ray against the bounding box
	float tmin = -15000.0f;
	float tmax =  15000.0f;
 
	float3 t1 = ( boundingBoxMin - rayOriginLocal.xyz ) / rayDirLocal.xyz;
	float3 t2 = ( boundingBoxMax - rayOriginLocal.xyz ) / rayDirLocal.xyz;
 
	tmin = max(tmin, min(t1.x, t2.x));
	tmax = min(tmax, max(t1.x, t2.x));
	tmin = max(tmin, min(t1.y, t2.y));
	tmax = min(tmax, max(t1.y, t2.y));
	tmin = max(tmin, min(t1.z, t2.z));
	tmax = min(tmax, max(t1.z, t2.z));

    float4 output = float4( 0.2f, 0.2f, 0.2f, 1.0f );

    if ( tmax >= tmin && tmax > 0.0f ) {

        output = float4( 0.0f, 0.5f, 0.2f, 1.0f );

        for ( uint i = 0; i < 4800; ++i )
        {
            const uint addr = i * 4 * 3;

            const uint3 triangleIndex = uint3(
                asuint( meshTriangles.Load( addr ) ),
                asuint( meshTriangles.Load( addr + 4 ) ),
                asuint( meshTriangles.Load( addr + 8 ) ) 
            );

            const uint3 vertexAddr = triangleIndex * 4 * 3;

		    const float3 vertex1 = float3(
                asfloat( meshVertices.Load( vertexAddr.x ) ),
                asfloat( meshVertices.Load( vertexAddr.x + 4 ) ),
                asfloat( meshVertices.Load( vertexAddr.x + 8 ) )
            );

            const float3 vertex2 = float3(
                asfloat( meshVertices.Load( vertexAddr.y ) ),
                asfloat( meshVertices.Load( vertexAddr.y + 4 ) ),
                asfloat( meshVertices.Load( vertexAddr.y + 8 ) )
            );

            const float3 vertex3 = float3(
                asfloat( meshVertices.Load( vertexAddr.z ) ),
                asfloat( meshVertices.Load( vertexAddr.z + 4 ) ),
                asfloat( meshVertices.Load( vertexAddr.z + 8 ) )
            );
		    
            const float dot1 = dot( rayDirLocal.xyz, cross( vertex1 - rayOriginLocal.xyz, vertex2 - rayOriginLocal.xyz ));
			const float dot2 = dot( rayDirLocal.xyz, cross( vertex2 - rayOriginLocal.xyz, vertex3 - rayOriginLocal.xyz ));
			const float dot3 = dot( rayDirLocal.xyz, cross( vertex3 - rayOriginLocal.xyz, vertex1 - rayOriginLocal.xyz ));

            if ( ( dot1 < 0 && dot2 < 0 && dot3 < 0 ) || ( dot1 > 0 && dot2 > 0 && dot3 > 0 ) ) {
                output = float4( 1.0f, 0.2f, 0.2f, 1.0f );
                break;
            }
        }
    }

    computeTarget[ dispatchThreadId.xy ] = output;

    
}



