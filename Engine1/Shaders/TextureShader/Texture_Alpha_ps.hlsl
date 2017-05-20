Texture2D colorTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
	float4 color = float4( colorTexture.Sample( samplerState, input.texCoord ).aaa, 1.0f );

	return color;
}