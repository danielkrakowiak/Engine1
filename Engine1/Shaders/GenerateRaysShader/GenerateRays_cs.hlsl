#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float3 cameraPos;
    float  pad1;
    float3 viewportCenter;
    float  pad2;
    float3 viewportUp;
    float  pad3;
    float3 viewportRight;
    float  pad4;
    float2 viewportSizeHalf;
    float2 pad5;
};

RWTexture2D<float4> computeTarget : register( u0 );

// pixelPos in range (0,0; screen width, screen height) counting from the top-left corner of the viewport.
float3 getPrimaryRayDirection( float2 pixelPos )
{
    // cameraPos - camera position in world space.
    // viewportCenter - viewport plane center in world space.
    // viewportUp - viewport up vector in world space. It's length equals half of height of the viewport plane.
    // viewportRight - viewport right vector in world space. It's length equals half of width of the viewport plane.
    // viewportSizeHalf - viewport dimensions in pixels divided by 2.

    const float2 pixelShift = (pixelPos - viewportSizeHalf + float2(0.5f, 0.5f)) / viewportSizeHalf; // In range (-1;1)

	const float3 pixelPosWorld = viewportCenter + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    return normalize( pixelPosWorld - cameraPos );
}

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
    const float2 pixelPos = (float2)dispatchThreadId.xy;

    const float3 rayDir = getPrimaryRayDirection( pixelPos );

    computeTarget[ dispatchThreadId.xy ] = float4( rayDir, 1.0f );
}



