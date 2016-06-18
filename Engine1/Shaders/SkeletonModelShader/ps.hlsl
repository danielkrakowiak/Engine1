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
	float4 normal        : TEXCOORD1;
    float4 bitangent     : TEXCOORD2;
    float4 tangent       : TEXCOORD3;
	float2 texCoord      : TEXCOORD4;
};

struct PixelOutputType {
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


    float3x3 tangentToWorldMatrix = float3x3( 
        normalize( input.tangent.xyz ),
        normalize( input.bitangent.xyz ),
        normalize( input.normal.xyz )
    );
			
	const float3 normalFromMap = ( normalTexture.Sample( samplerState, input.texCoord ).rgb - 0.5f ) * 2.0f;
	output.normal = float4( normalize( mul( normalFromMap, tangentToWorldMatrix ) ), 0.0f );

    output.metalness         = metalnessTexture.Sample( samplerState, input.texCoord ).r;
    output.roughness         = roughnessTexture.Sample( samplerState, input.texCoord ).r;
    output.indexOfRefraction = indexOfRefractionTexture.Sample( samplerState, input.texCoord ).r;

	return output;
}