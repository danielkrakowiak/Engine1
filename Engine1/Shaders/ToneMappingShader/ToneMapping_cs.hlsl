#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float  exposure;
    float3 pad1;
};

// Input
Texture2D<float4> g_srcTexture : register( t0 );

// Output.
RWTexture2D<uint4> g_dstTexture : register( u0 );

float3 tonemap(float3 col);

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
    const float3 color = g_srcTexture[ dispatchThreadId.xy ].rgb;

    const float3 outputColor  = tonemap( color * exposure );

    g_dstTexture[ dispatchThreadId.xy ] = uint4( outputColor * 255.0, 255 );
}

float3 tonemap(float3 color)
{
    // Jim Hejl and Richard Burgess-Dawson tonemapping.
    // Source: http://filmicworlds.com/blog/filmic-tonemapping-operators/

	const float3 x = max( 0.0, color - 0.004 );

	return (x * (6.2 * x + 0.5)) / ( x * (6.2 * x + 1.7) + 0.06);
}