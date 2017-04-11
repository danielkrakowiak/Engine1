Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
	float4 textureColor = float4( albedoTexture.Sample( samplerState, input.texCoord ).aaa, 1.0f );

	return textureColor;
}