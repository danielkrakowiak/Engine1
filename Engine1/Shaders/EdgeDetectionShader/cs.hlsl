#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<float4> g_surfacePosition : register( t0 );
Texture2D<float4> g_surfaceNormal   : register( t1 );

// Output.
RWTexture2D<uint> g_distToEdge : register( u0 );

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(32, 32, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    const float3 surfacePosition = g_surfacePosition[ dispatchThreadId.xy ].xyz;

    if ( !any( surfacePosition ) ) { // If all position components are zeros - there is no reflection.
        g_distToEdge[ dispatchThreadId.xy ] = 0;
        return;
    }

    const float3 surfaceNormal = g_surfaceNormal[ dispatchThreadId.xy ].xyz;

    { // Compare with right neighbor pixel.
        const float3 surfaceNormalRight = g_surfaceNormal[ dispatchThreadId.xy + uint2( 0, 1 ) ].xyz;
        const float  normalsDot = dot( surfaceNormal, surfaceNormalRight );
        if ( normalsDot < 0.95f ) {
            g_distToEdge[ dispatchThreadId.xy ] = 0;
            return;
        }
    }

    { // Compare with down neighbor pixel.
        const float3 surfaceNormalRight = g_surfaceNormal[ dispatchThreadId.xy + uint2( 1, 0 ) ].xyz;
        const float  normalsDot = dot( surfaceNormal, surfaceNormalRight );
        if ( normalsDot < 0.95f ) {
            g_distToEdge[ dispatchThreadId.xy ] = 0;
            return;
        }
    }

    { // Compare with right-down neighbor pixel.
        const float3 surfaceNormalRight = g_surfaceNormal[ dispatchThreadId.xy + uint2( 1, 1 ) ].xyz;
        const float  normalsDot = dot( surfaceNormal, surfaceNormalRight );
        if ( normalsDot < 0.95f ) {
            g_distToEdge[ dispatchThreadId.xy ] = 0;
            return;
        }
    }


    //[unroll]
    //for ( int y = 0; y <= 1; ++y ) 
    //{
    //    [unroll]
    //    for ( int x = 0; x <= 1; ++x ) 
    //    {
    //        const float normalsDot = dot( surfaceNormal, g_surfaceNormal[ dispatchThreadId.xy + int2( x, y ) ].xyz );
    //        if ( normalsDot < 0.99f )
    //        {
    //            g_distToEdge[ dispatchThreadId.xy ] = 0;
    //            return;
    //        }
    //    }
    //}
}

