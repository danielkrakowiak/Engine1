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
	float4 textureColor = albedoTexture.Sample( samplerState, input.texCoord );

    //////////////////////////////////////////////////////////////////
    // Debug: Display single-channel textures darkened - because they probably contain distance data (high values).
    if (textureColor.g + textureColor.b < 0.0001f)
        textureColor /= 5.0f;
    //////////////////////////////////////////////////////////////////

	return textureColor;
}