#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float  skipPixelIfBelowValue;
    float3 pad1;
    float  minAcceptableValue;
    float3 pad2;
    int    spreadDistance; // Unused.
    float3 pad3;
    int    offset;         // Unused.
    float3 pad4;
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

    // Optimization - don't recalculate pixels, which were calculated in the previous passes.
    //if ( value < skipPixelIfBelowValue )
    //    return;

    // TODO: Try to sample furhter than one pixel away from center to decrease number of passes.
    // Can check if got any value before sampling further.
   
    float newValue = value;//2.0f * skipPixelIfBelowValue + 1.0f;

    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  0, -1 ) ] ); // Top-neighbor.
    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2( -1,  0 ) ] ); // Left-neighbor.
    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  1,  0 ) ] ); // Right-neighbor.
    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  0,  1 ) ] ); // Bottom-neighbor.

    // Note: Top-left, top-right, bottom-left and bottom-right comparisons are probably redundant.

    // Note: min-acceptable-value is used to avoid spreading very low values accross large areas.
    // Useful in the specific case of spreading blur-radius.
    //if ( newValue >= minAcceptableValue )
        g_texture[ dispatchThreadId.xy ] = max(value, newValue /*- 0.02f*/); // Note: Test - substract 1 to limit spreading values...
}



// Pretty good way to spread softer shadow blur-radius through hard edges.

//#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices
//
//cbuffer ConstantBuffer
//{
//    float  skipPixelIfBelowValue;
//    float3 pad1;
//    float  minAcceptableValue;
//    float3 pad2;
//};
//
//// Output.
//RWTexture2D<float> g_texture : register( u0 );
//
//// SV_GroupID - group id in the whole computation.
//// SV_GroupThreadID - thread id within its group.
//// SV_DispatchThreadID - thread id in the whole computation.
//// SV_GroupIndex - index of the group within the whole computation.
//[numthreads(8, 8, 1)]
//void main( uint3 groupId : SV_GroupID,
//           uint3 groupThreadId : SV_GroupThreadID,
//           uint3 dispatchThreadId : SV_DispatchThreadID,
//           uint  groupIndex : SV_GroupIndex )
//{
//    const float value = g_texture[ dispatchThreadId.xy ];
//
//    // Optimization - don't recalculate pixels, which were calculated in the previous passes.
//    //if ( value < skipPixelIfBelowValue )
//    //    return;
//
//    // TODO: Try to sample furhter than one pixel away from center to decrease number of passes.
//    // Can check if got any value before sampling further.
//   
//    float newValue = value;//2.0f * skipPixelIfBelowValue + 1.0f;
//
//    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  0, -1 ) ] ); // Top-neighbor.
//    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2( -1,  0 ) ] ); // Left-neighbor.
//    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  1,  0 ) ] ); // Right-neighbor.
//    newValue = max( newValue, g_texture[ ( int2 )dispatchThreadId.xy + int2(  0,  1 ) ] ); // Bottom-neighbor.
//
//    // Note: Top-left, top-right, bottom-left and bottom-right comparisons are probably redundant.
//
//    // Note: min-acceptable-value is used to avoid spreading very low values accross large areas.
//    // Useful in the specific case of spreading blur-radius.
//    //if ( newValue >= minAcceptableValue )
//        g_texture[ dispatchThreadId.xy ] = max(value, newValue - 0.5f); // Note: Test - substract 1 to limit spreading values...
//}

