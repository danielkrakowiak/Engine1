SamplerState g_samplerState  : register( s0 );

Texture2D<float> g_textureSrcMipmap : register( t0 );

cbuffer ConstantBuffer
{
    float2 srcPixelSizeInTexcoords;
    float2 pad1;
    float  srcMipmapLevel;
    float3 pad2;
    float  maxAcceptableValue;
    float3 pad3;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float main(PixelInputType input) : SV_Target
{
    // #TODO: Can it be optimized with gather?
    const float valueTopLeft     = g_textureSrcMipmap.SampleLevel( g_samplerState, input.texCoord + float2( -srcPixelSizeInTexcoords.x, -srcPixelSizeInTexcoords.y ), 0.0f );
    const float valueTopRight    = g_textureSrcMipmap.SampleLevel( g_samplerState, input.texCoord + float2(  srcPixelSizeInTexcoords.x, -srcPixelSizeInTexcoords.y ), 0.0f );
    const float valueBottomLeft  = g_textureSrcMipmap.SampleLevel( g_samplerState, input.texCoord + float2( -srcPixelSizeInTexcoords.x,  srcPixelSizeInTexcoords.y ), 0.0f );
    const float valueBottomRight = g_textureSrcMipmap.SampleLevel( g_samplerState, input.texCoord + float2(  srcPixelSizeInTexcoords.x,  srcPixelSizeInTexcoords.y ), 0.0f );

    float valueSum  = 0.0f;
    float weightSum = 0.000001f; // Note: Small value to avoid division by zero.

    // Equal to "if ( valueTopLeft <= maxAcceptableValue )"
    const float weightTopLeft = max( 0.0, maxAcceptableValue - valueTopLeft ) * 1000.0f;
    valueSum  += valueTopLeft * weightTopLeft;
    weightSum += weightTopLeft;

    const float weightTopRight = max(0.0, maxAcceptableValue - valueTopRight) * 1000.0f;
    valueSum  += valueTopRight * weightTopRight;
    weightSum += weightTopRight;

    const float weightBottomLeft = max(0.0, maxAcceptableValue - valueBottomLeft) * 1000.0f;
    valueSum  += valueBottomLeft * weightBottomLeft;
    weightSum += weightBottomLeft;

    const float weightBottomRight = max(0.0, maxAcceptableValue - valueBottomRight) * 1000.0f;
    valueSum  += valueBottomRight * weightBottomRight;
    weightSum += weightBottomRight;

	// If all samples were rejected - use the original value (instead of returning zero).
	// We also assume that all rejected samples have the same value. 
    const float valueIfAllRejected = max(0.0, 1.0 - weightSum) * valueTopLeft;

    return (valueSum / weightSum) + valueIfAllRejected;
}