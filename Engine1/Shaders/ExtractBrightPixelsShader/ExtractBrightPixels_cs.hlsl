#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float  minBrightness;
    float3 pad1;
};

// Input.
Texture2D<float4> g_colorTexture : register( t0 );

// Output.
RWTexture2D<float4> g_destTexture : register( u0 );

static const float3 brightnessMult = float3( 0.2126f, 0.7152f, 0.0722f );

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
    const float3 color = g_colorTexture[ dispatchThreadId.xy ].rgb;

    const float brightness = dot( color, brightnessMult );

    if ( brightness > minBrightness )
        g_destTexture[ dispatchThreadId.xy ] = float4( color, 1.0f );
    else
        g_destTexture[ dispatchThreadId.xy ] = float4( 0.0f, 0.0f, 0.0f, 1.0f );
}