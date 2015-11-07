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
	float4 albedo   : SV_Target0;
	float4 normal   : SV_Target1;
	float4 vertexId : SV_Target2;
};

PixelOutputType main( PixelInputType input ) : SV_TARGET
{
	PixelOutputType output;

	output.albedo = albedoTexture.Sample( samplerState, input.texCoord );
	output.normal = input.normal;
	output.vertexId = input.vertexId / 10000.0f;

	return output;
}