Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
	float  vertexId : TEXCOORD2;
};

struct PixelOutputType {
    float2 normal   : SV_Target0;
	float4 albedo   : SV_Target1;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

	output.albedo = albedoTexture.Sample( samplerState, input.texCoord );
	output.normal = input.normal.xy;

	return output;
}