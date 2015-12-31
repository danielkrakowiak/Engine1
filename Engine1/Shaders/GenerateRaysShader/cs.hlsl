#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float3 cameraPos;
    float  pad1;
    float3 viewportTopLeft;
    float  pad2;
    float3 viewportUp;
    float  pad3;
    float3 viewportRight;
    float  pad4;
    float2 viewportSize;
    float2 pad5;
};

RWTexture2D<float4> computeTarget : register( u0 );

// pixelPos in range (0,0; screen width, screen height) counting from the top-left corner of the viewport.
float3 getPrimaryRayDirection(float2 pixelPos)
{
    // cameraPos - camera position in world space.
    // viewportBottomLeft - viewport plane bottom-left corner in world space.
    // viewportUp - viewport up vector in world space. It's length equals height of the viewport plane.
    // viewportRight - viewport right vector in world space. It's length equals width of the viewport plane.
    // viewportSize - in pixels.

    const float2 pixelShift = (pixelPos + float2(0.5f, 0.5f)) / viewportSize; // In range (0;1)

	const float3 pixelPosWorld = viewportTopLeft + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    return pixelPosWorld - cameraPos;
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
    //const float fov = 90.0f;

    //const float2 viewportSize       = float2( 1024.0f, 768.0f );
    //const float3 cameraPos          = float3( 0.0f, 0.0f, 0.0f );
    //const float3 viewportUp         = float3( 0.0f, tan(fov), 0.0f );
    //const float3 viewportRight      = float3( (viewportSize.x / viewportSize.y) * length(viewportUp), 0.0f, 0.0f );
    //const float3 viewportBottomLeft = float3( -viewportRight.x * 0.5f, -viewportUp.y * 0.5f, 1.0f ); // z different than in camera pos!

    const float2 pixelPos = (float2)dispatchThreadId.xy;

    const float3 rayDir = getPrimaryRayDirection( pixelPos );

    computeTarget[ dispatchThreadId.xy ] = float4( rayDir, 1.0f );

    //float3 groupIdColor = ((float3)groupId / 32.0f);
    //computeTarget[ dispatchThreadId.xy ] = float4( groupIdColor.x, groupIdColor.y, groupIdColor.z, 1.0f );
}



