SamplerState g_samplerState  : register( s0 );

Texture2D<float4> g_textureSrcMipmap : register( t0 );

cbuffer ConstantBuffer
{
    float2 srcPixelSizeInTexcoords;
    float2 pad1;
    float  srcMipmapLevel;
    float3 pad2;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
	const float4 values = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord );
    
    return values;
}