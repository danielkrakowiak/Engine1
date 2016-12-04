SamplerState g_samplerState  : register( s0 );

Texture2D<float> g_textureSrcMipmap : register( t0 );

//cbuffer ConstantBuffer
//{};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float main(PixelInputType input) : SV_Target
{
	const float4 values = g_textureSrcMipmap.Gather( g_samplerState, input.texCoord );
    
    // Return min of four pixels.
    return min( values.w, min( values.z, min( values.x, values.y ) ) );
}