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
	float4 color = colorTexture.Sample( samplerState, input.texCoord );

    //////////////////////////////////////////////////////////////////
    // Debug: Display single-channel textures darkened - because they probably contain distance data (high values).
    /*if (color.g + color.b < 0.0001f)
        color /= 5.0f;*/
    //////////////////////////////////////////////////////////////////

	return color;
}