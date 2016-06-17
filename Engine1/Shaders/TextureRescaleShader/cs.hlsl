#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

SamplerState samplerState;

cbuffer ConstantBuffer
{
    float2 destTexturePixelSize; // In texcoords.
    float2 pad1;                 // Padding.
    uint   srcMipmapLevel;
};

// Input.
Texture2D<float4>   g_srcTexture  : register( t0 );

// Output.
RWTexture2D<float4> g_DestTexture : register( u0 );

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
    const float2 texCoords = ((float2)dispatchThreadId.xy + float2( 0.5f, 0.5f )) * destTexturePixelSize;

    // Sample 0 level, because passed SRV already points to a specific mipmap.
    const float4 srcColor = g_srcTexture.SampleLevel( samplerState, texCoords, 0.0f/*(float)srcMipmapLevel*/ ); 

    g_DestTexture[ dispatchThreadId.xy ] = srcColor;
}

