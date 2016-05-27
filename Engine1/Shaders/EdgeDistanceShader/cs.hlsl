#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    uint   passIndex;
    float3 pad1;      // Padding.
};

// Input.
Texture2D<uint>   g_distToEdgeSrc  : register( t0 );

// Output.
RWTexture2D<uint> g_distToEdgeDest : register( u0 );

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
    const uint currDistToEdge = g_distToEdgeSrc[ dispatchThreadId.xy ];

    // Optimization - don't recalculate pixels, which were calculated in the previous passes.
    if ( currDistToEdge < passIndex - 1 )
        return;
   
    uint distToNearestEdge = 254;

    distToNearestEdge = min( distToNearestEdge, g_distToEdgeSrc[ ( int2 )dispatchThreadId.xy + int2(  0, -1 ) ] ); // Top-neighbor.
    distToNearestEdge = min( distToNearestEdge, g_distToEdgeSrc[ ( int2 )dispatchThreadId.xy + int2( -1,  0 ) ] ); // Left-neighbor.
    distToNearestEdge = min( distToNearestEdge, g_distToEdgeSrc[ ( int2 )dispatchThreadId.xy + int2(  1,  0 ) ] ); // Right-neighbor.
    distToNearestEdge = min( distToNearestEdge, g_distToEdgeSrc[ ( int2 )dispatchThreadId.xy + int2(  0,  1 ) ] ); // Bottom-neighbor.

    // Note: Top-left, top-right, bottom-left and bottom-right comparisons are probably redundant.

    g_distToEdgeDest[ dispatchThreadId.xy ] = min( currDistToEdge, distToNearestEdge + 1 );
}

