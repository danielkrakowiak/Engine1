Texture2D albedoTexture;
Texture2D normalTexture;
Texture2D metalnessTexture;
Texture2D roughnessTexture;
Texture2D indexOfRefractionTexture;

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
	float4 normal            : SV_Target0;
    float4 position          : SV_Target1;
    float  metalness         : SV_Target2;
    float  roughness         : SV_Target3;
    float  indexOfRefraction : SV_Target4;
    float4 albedo            : SV_Target5;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

    output.position = float4( input.positionWorld, 0.0f );
	output.albedo   = albedoTexture.Sample( samplerState, input.texCoord );
	output.normal   = float4( input.normal, 0.0f );

    output.metalness         = metalnessTexture.Sample( samplerState, input.texCoord ).r;
    output.roughness         = roughnessTexture.Sample( samplerState, input.texCoord ).r;
    output.indexOfRefraction = indexOfRefractionTexture.Sample( samplerState, input.texCoord ).r;

	return output;
}