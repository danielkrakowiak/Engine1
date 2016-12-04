#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

SamplerState g_samplerState;

// Input.
Texture2D<float> g_textureSrcMipmap : register( t0 );

// Output.
RWTexture2D<float> g_textureDstMipmap : register( u0 );

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
    const uint2 srcPixelPos = dispatchThreadId.xy * 2;
    // #TODO: Probably could be optimized using Gather method. But then it requies a sampler targeted at a specific mipmap level.
    const float value1 = g_textureSrcMipmap[ srcPixelPos ];
    const float value2 = g_textureSrcMipmap[ srcPixelPos + uint2( 1, 0 ) ];
    const float value3 = g_textureSrcMipmap[ srcPixelPos + uint2( 0, 1 ) ];
    const float value4 = g_textureSrcMipmap[ srcPixelPos + uint2( 1, 1 ) ];

    // #TODO: Something wrong with src/dst mipmap level? Writing/reading from wrong ones? All mipmaps end up black...

    g_textureDstMipmap[ dispatchThreadId.xy ] = 0.5f;//min( value4, min( value3, min( value2, value1 ) ) );
}
