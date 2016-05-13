Texture2D srcTexture;

SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
	float4 textureColor = srcTexture.Sample( samplerState, input.texCoord );

    textureColor.a = 0.5f;

	return textureColor;
}