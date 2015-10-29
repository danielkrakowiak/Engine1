Texture2D characterTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PixelOutputType {
	float4 albedo   : SV_Target0;
	float4 normal   : SV_Target1;
	float4 vertexId : SV_Target2;
};

PixelOutputType main( PixelInputType input ) : SV_TARGET
{
	PixelOutputType output;

	float4 textureColor = characterTexture.Sample( samplerState, input.texCoord );

	output.albedo = float4( 1.0f, 1.0f, 1.0f, textureColor.r );
	output.normal = float4( 0.0f, 0.0f, 1.0f, 1.0f );
	output.vertexId = 0.0f;

	return output;
}