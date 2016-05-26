#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<uint>   g_distToEdgeSrc  : register( t0 );

// Output.
RWTexture2D<uint> g_distToEdgeDest : register( u0 );

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
    const uint currDistToEdge = g_distToEdgeSrc[ dispatchThreadId.xy ];
   
     uint distToNearestEdge = 254;

    //[unroll]
    for ( int x = -1; x <= 1; ++x ) {
        //[unroll]
        for ( int y = -1; y <= 1; ++y ) {
            distToNearestEdge = min( distToNearestEdge, g_distToEdgeSrc[ ( int2 )dispatchThreadId.xy + int2( x, y ) ] );
        }
    }

    g_distToEdgeDest[ dispatchThreadId.xy ] = min( currDistToEdge, distToNearestEdge + 1 );

    //

    //if ( currDistToEdge == 0 )
        //g_distToEdge[ dispatchThreadId.xy ] = currDistToEdge + 100;

    // Distance to nearest edge + 1. Limited to maximum of 255.
    //g_distToEdge[ dispatchThreadId.xy ] = min(255, currDistToEdge + 100);//min( currDistToEdge, distToNearestEdge + 10 ); //min( currDistToEdge, min( 255, distToNearestEdge + 10 ) );
}

