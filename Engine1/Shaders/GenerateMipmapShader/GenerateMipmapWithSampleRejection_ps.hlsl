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
    float weightSum = 0.0f;

    // Equal to "if ( valueTopLeft <= maxAcceptableValue )"
    const float weightTopLeft = ( maxAcceptableValue - valueTopLeft ) * 1000.0f;
    valueSum  += valueTopLeft * weightTopLeft;
    weightSum += weightTopLeft;

    const float weightTopRight = ( maxAcceptableValue - valueTopRight ) * 1000.0f;
    valueSum  += valueTopRight * weightTopRight;
    weightSum += weightTopRight;

    const float weightBottomLeft = ( maxAcceptableValue - valueBottomLeft ) * 1000.0f;
    valueSum  += valueBottomLeft * weightBottomLeft;
    weightSum += weightBottomLeft;

    const float weightBottomRight = ( maxAcceptableValue - valueBottomRight ) * 1000.0f;
    valueSum  += valueBottomRight * weightBottomRight;
    weightSum += weightBottomRight;

    // #TODO: Can we simply return 0 in case we rejected all samples? That would remove that "if".
    if ( weightSum > 0.0f )
        return valueSum / weightSum;
    else
        return valueBottomRight;
}