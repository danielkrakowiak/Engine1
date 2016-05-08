Texture2D characterTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PixelOutputType {
    float2 normal   : SV_Target0;
	float4 albedo   : SV_Target1;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

	float4 textureColor = characterTexture.Sample( samplerState, input.texCoord );

	output.normal = float2( 0.0f, 0.0f );
    output.albedo = float4( 1.0f, 1.0f, 1.0f, textureColor.r );

	return output;
}