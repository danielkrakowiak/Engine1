#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<float> g_texture2 : register( t0 );

// Input / Output.
RWTexture2D<float> g_texture : register( u0 );

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(8, 8, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    const float minValue = min( g_texture[ dispatchThreadId.xy ], g_texture2[ dispatchThreadId.xy ] );

    g_texture[ dispatchThreadId.xy ] = minValue;
}

