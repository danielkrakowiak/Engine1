#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
    float  skipPixelIfBelowValue;
    float3 pad1;
    float  minAcceptableValue;
    float3 pad2;
    int    spreadDistance; // In pixels.
    float3 pad3;
    int    offset;         // In pixels (same along x and y).
    float3 pad4;
    float2 textureSize;
    float2 pad5;
    float3 cameraPos;
    float  pad6;
    float  totalSpread;
    float3 pad7;
};

SamplerState g_samplerState;

// Input.
Texture2D< float4 > g_positionTexture : register( t0 );

// Output.
RWTexture2D<float> g_texture : register( u0 );

void sampleConditionally( inout float valueSum, inout float weightSum, in int2 texcoordsInt, in float minAcceptableValue, in float maxAcceptableValue,
                          in float3 centerPositionInWorldSpace, in float maxDistFromCenterToSampleInWorldSpace );

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
    const int2 texcoordInt = (int2)dispatchThreadId.xy * spreadDistance + offset.xx;
    const float2 texcoords = ((float2)texcoordInt + 0.5f) / textureSize;

    const float value = g_texture[ texcoordInt ];

    // Optimization - don't recalculate pixels, which were calculated in the previous passes.
    if ( value < skipPixelIfBelowValue )
        return;

    float valueSum  = 0.0f;
    float weightSum = 0.0f;

    // #TODO: OPTIMIZATION: Could use gather to optimize sampling.

    // Sampling shape:
    //      # # #
    //      # * #
    //      # # #

    const float3 centerSurfacePosition = g_positionTexture.SampleLevel( g_samplerState, texcoords, 0.0f ).xyz;

    const float  centerDistToCamera = length( cameraPos - centerSurfacePosition );

    // #TODO: Take FOV tangens as input argument.
    const float pixelSizeInWorldSpace = (centerDistToCamera * tan( Pi / 8.0f )) / (textureSize.y * 0.5f);

    const float minAcceptableValue                    = pixelSizeInWorldSpace * totalSpread;
    const float maxDistFromCenterToSampleInWorldSpace = pixelSizeInWorldSpace * spreadDistance;

    sampleConditionally( valueSum, weightSum, texcoordInt + int2( -spreadDistance, -spreadDistance ), minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  0,              -spreadDistance ), minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  spreadDistance, -spreadDistance ), minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2( -spreadDistance, 0 ),               minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  0,              0 ),               minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  spreadDistance, 0 ),               minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2( -spreadDistance, spreadDistance ),  minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  0,              spreadDistance ),  minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );
    sampleConditionally( valueSum, weightSum, texcoordInt + int2(  spreadDistance, spreadDistance ),  minAcceptableValue, skipPixelIfBelowValue, centerSurfacePosition, maxDistFromCenterToSampleInWorldSpace );

    // Check if any samples were accepted - if so, avarage them.
    if ( weightSum > 0.0f ) 
    {
        g_texture[ texcoordInt ] = valueSum / weightSum;
    }
    
    // Note: min-acceptable-value is used to avoid spreading very low values accross large areas.
    // Useful in the specific case of spreading blur-radius.
    //if ( newValue >= minAcceptableValue )
    //    g_texture[ dispatchThreadId.xy ] = min(value, newValue /*- 0.2f*/); // Note: Test - substract 1 to limit spreading values...
}

void sampleConditionally( inout float valueSum, inout float weightSum, in int2 texcoordsInt, in float minAcceptableValue, in float maxAcceptableValue,
                          in float3 centerPositionInWorldSpace, in float maxDistFromCenterToSampleInWorldSpace )
{
    const float value = g_texture[ texcoordsInt ];

    // Equals zero if sample is greater than threshold.
    const float acceptMul = saturate( maxAcceptableValue - value );

    // Equals zero if sample is zero - we reject zeros because they mean samples outside of screen area.
    // And zeros don't really have to spread or be part of the avarage.
    const float acceptMul2 = min( 1.0f, value * 1000.0f ); 

    const float2 sampleTexcoords = ((float2)texcoordsInt + 0.5f) / textureSize;
    const float3 samplePositionInWorldSpace = g_positionTexture.SampleLevel( g_samplerState, sampleTexcoords, 0.0f ).xyz;

    // Accept sample if it's not too far from the center in world space.
    // Limits spreading small values and avoids spreading blur-radius from foreground to background objects.
    // #TODO: Could be optimized using squared maxDistFromCenterToSampleInWorldSpace to avoid calling "length".
    const float acceptMul3 = length( centerPositionInWorldSpace - samplePositionInWorldSpace ) < maxDistFromCenterToSampleInWorldSpace ? 1.0f : 0.0f;

    const float acceptMul4 = saturate( ( value - minAcceptableValue ) * 1000.0f );

    valueSum  += acceptMul * acceptMul3 * acceptMul4 * value;
    weightSum += acceptMul * acceptMul2 * acceptMul3 * acceptMul4; // Zero or one.
}
