#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float  replaceFromValue;
    float3 pad1;
    float  replaceToValue;
    float3 pad2;
};

// Output.
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
    const float value = g_texture[ dispatchThreadId.xy ];

    const float keepValueTerm = saturate( replaceFromValue - value );

    g_texture[ dispatchThreadId.xy ] = keepValueTerm * value + (1.0f - keepValueTerm) * replaceToValue;

    // OPTIMIZATION: Could be replaced with some math calculation to avoid if.
    //if ( g_texture[ dispatchThreadId.xy ] > 900.0f );
    //    replaceFromValue = replaceFromValue replaceToValue;
}

