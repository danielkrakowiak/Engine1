#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float3 cameraPos;
    float  pad1;
    float2 outputTextureSize;
    float2 pad2;
};

SamplerState g_samplerState;

// Input.
Texture2D< float4 > g_positionTexture : register( t0 );

// Input / Output.
RWTexture2D< float > g_texture : register( u0 );

static const float Pi = 3.14159265f;

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
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    // #TODO: Which is better? linear or point? And from which mipmap? 3rd to avarage some region? Add some param?
    const float3 surfacePosition     = g_positionTexture.SampleLevel( g_samplerState, texcoords, 0.0f ).xyz;
    const float  surfaceDistToCamera = length( cameraPos - surfacePosition );

    const float distInScreenSpace = g_texture[ dispatchThreadId.xy ];

    // #TODO: Take FOV tangens as input argument.
    const float pixelSizeInWorldSpace = (surfaceDistToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);
    const float distInWorldSpace     = distInScreenSpace * pixelSizeInWorldSpace;

    g_texture[ dispatchThreadId.xy ] = distInWorldSpace;
}

