#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float  blurRadius;
    float3 pad1;
    float2 outputTextureSize;
    float2 pad2;
    float2 inputTextureSize;
    float2 pad3;
};

SamplerState g_samplerState;

// Input.
Texture2D<float4> g_inputTexture : register( t0 );

// Output.
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
    // #TODO: Could be optimized by passing pixelSizeInTexcoords through constant buffer.
    const float2 texcoords            = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;
    const float2 halfPixelSizeInTexcoords = 0.5f / inputTextureSize;

    // We sample at the center of pixel and at it's corners to achieve small Gaussian blur.
    const float4 sampleLeftTop     = g_inputTexture.SampleLevel( g_samplerState, texcoords - halfPixelSizeInTexcoords, 0.0f );
    const float4 sampleRightTop    = g_inputTexture.SampleLevel( g_samplerState, texcoords + float2( halfPixelSizeInTexcoords.x, -halfPixelSizeInTexcoords.y), 0.0f );
    const float4 sampleCenter      = g_inputTexture.SampleLevel( g_samplerState, texcoords, 0.0f );
    const float4 sampleLeftBottom  = g_inputTexture.SampleLevel( g_samplerState, texcoords + float2( -halfPixelSizeInTexcoords.x, halfPixelSizeInTexcoords.y), 0.0f );
    const float4 sampleRightBottom = g_inputTexture.SampleLevel( g_samplerState, texcoords + halfPixelSizeInTexcoords, 0.0f );

    g_outputTexture[ dispatchThreadId.xy ] = ( sampleLeftTop + sampleRightTop + sampleCenter + sampleLeftBottom + sampleRightBottom ) / 5.0f;
}

