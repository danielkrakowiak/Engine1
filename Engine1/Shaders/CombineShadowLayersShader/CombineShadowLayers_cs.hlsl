#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

// Input.
Texture2D<float> g_hardShadow   : register( t0 ); // Input textures are assumed to be in UNORM format.
Texture2D<float> g_mediumShadow : register( t1 );
Texture2D<float> g_softShadow   : register( t2 );

// Output.
RWTexture2D<uint> g_finalShadow : register( u0 );

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
    float hardShadow   = g_hardShadow[ dispatchThreadId.xy ];
    float mediumShadow = g_mediumShadow[ dispatchThreadId.xy ];
    float softShadow   = g_softShadow[ dispatchThreadId.xy ];

    // Note: This is very important! If removed, there would be too much shadow where shadow from different layers overlap.
    // And this would cause "shadow bubbles", shadow shape changes on camera movement etc. It is also crucial, because
    // it allows shadows from different layers to overlap (doesn't matter on which layer a given shadow is, the sum it correct), 
    // which protects us against light leaks at layer trnasitions.
    mediumShadow = max(0.0, mediumShadow - hardShadow);
    softShadow   = max(0.0, softShadow - mediumShadow - hardShadow);

    g_finalShadow[ dispatchThreadId.xy ] = min( 1.0, hardShadow + mediumShadow + softShadow ) * 255.0;
}

