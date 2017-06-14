#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input / Output.
RWTexture2D<uint4> g_texture : register( u0 );

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
    const float3 color     = (float3)g_texture[ dispatchThreadId.xy ].rgb / 255.0;
    const float  luminance = dot( color.rgb, float3( 0.299, 0.587, 0.114 ) );

    g_texture[ dispatchThreadId.xy ] = uint4( color * 255.0, luminance * 255.0 );
}
