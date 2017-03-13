#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float  exposure;
    float3 pad1;
};

// Input / Output.
RWTexture2D<float4> g_texture : register( u0 );

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
    const float3 color = g_texture[ dispatchThreadId.xy ].rgb;

    const float3 outputColor  = color * pow( 2.0f, exposure );

    g_texture[ dispatchThreadId.xy ] = float4( outputColor, 1.0f );
}