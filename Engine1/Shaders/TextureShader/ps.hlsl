Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
    //return float4( 1.0f, 0.0f, 0.0f, 1.0f );
	float4 textureColor = albedoTexture.Sample( samplerState, input.texCoord );

	//return input.normal;
	return textureColor;
}