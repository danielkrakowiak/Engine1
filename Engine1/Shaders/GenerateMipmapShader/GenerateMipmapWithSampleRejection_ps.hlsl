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
    const float valueTopLeft     = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord + float2( -srcPixelSizeInTexcoords.x, -srcPixelSizeInTexcoords.y ) );
    const float valueTopRight    = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord + float2(  srcPixelSizeInTexcoords.x, -srcPixelSizeInTexcoords.y ) );
    const float valueBottomLeft  = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord + float2( -srcPixelSizeInTexcoords.x,  srcPixelSizeInTexcoords.y ) );
    const float valueBottomRight = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord + float2(  srcPixelSizeInTexcoords.x,  srcPixelSizeInTexcoords.y ) );

    float valueSum  = 0.0f;
    float weightSum = 0.0f;

    if ( valueTopLeft <= maxAcceptableValue ){
        valueSum += valueTopLeft;
        weightSum += 1.0f;
    }

    if ( valueTopRight <= maxAcceptableValue ){
        valueSum += valueTopRight;
        weightSum += 1.0f;
    }

    if ( valueBottomLeft <= maxAcceptableValue ){
        valueSum += valueBottomLeft;
        weightSum += 1.0f;
    }

    if ( valueBottomRight <= maxAcceptableValue ){
        valueSum += valueBottomRight;
        weightSum += 1.0f;
    }

    if ( weightSum > 0.0f )
        return valueSum / weightSum;
    else
        return valueBottomRight;
}