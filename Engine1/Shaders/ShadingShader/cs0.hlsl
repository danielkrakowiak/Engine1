#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

SamplerState g_linearSamplerState;

// Input.
Texture2D<float4> g_emissiveTexture : register( t0 );

// Output.
RWTexture2D<float4> g_colorTexture : register( u0 );

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
    const float3 surfaceEmissive = g_emissiveTexture[ dispatchThreadId.xy ].xyz;

    g_colorTexture[ dispatchThreadId.xy ] = float4( surfaceEmissive, 1.0f );
}
