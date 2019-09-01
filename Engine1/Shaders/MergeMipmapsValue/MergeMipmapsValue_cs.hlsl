#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float2 outputTextureSize;
    float2 pad1;
    float  firstMipmapLevel; // Should always be an integral number (not fractional).
    float3 pad2;
    float  lastMipmapLevel; // Should always be an integral number (not fractional).
    float3 pad3;
};

SamplerState g_samplerState;

// Input.
Texture2D<float4> g_baseTexture      : register(t0);
Texture2D<float4> g_mipmappedTexture : register( t1 );

// Input / Output.
RWTexture2D<float4> g_outputTexture : register( u0 );

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
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    float4 value = g_baseTexture.SampleLevel( g_samplerState, texcoords, 0 );

    // Note: We start shifted by 0.5 and jump 2 mipmap levels at once to use hardware interpolation between mipmaps.
    // That's also the reason to clamp mipmap level to the maximal desired mipmap level to avoid "over-jumping" it.
    for ( float mipmapLevel = firstMipmapLevel + 0.5f; mipmapLevel < lastMipmapLevel + 1.0f; mipmapLevel += 2.0f )
    {
        const float level = min( mipmapLevel, lastMipmapLevel );

        value += g_mipmappedTexture.SampleLevel( g_samplerState, texcoords, level );
    }

    g_outputTexture[ dispatchThreadId.xy ] = float4( value.rgb, 1.0f );
}

