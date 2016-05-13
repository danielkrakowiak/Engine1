Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
	float2 texCoord      : TEXCOORD2;
};

struct PixelOutputType 
{
	float4 normal   : SV_Target0;
    float4 position : SV_Target1;
    float4 albedo   : SV_Target2;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

    output.position = float4( input.positionWorld, 0.0f );
	output.albedo   = albedoTexture.Sample( samplerState, input.texCoord );
	output.normal   = float4( input.normal, 0.0f );

	return output;
}