Texture2D alphaTexture;
Texture2D emissiveTexture;
Texture2D albedoTexture;
Texture2D normalTexture;
Texture2D metalnessTexture;
Texture2D roughnessTexture;
Texture2D indexOfRefractionTexture;

SamplerState samplerState;

cbuffer ConstantBuffer
{
    float  alphaMul;
    float3 pad1;
    float3 emissiveMul;
    float  pad2;
    float3 albedoMul;
    float  pad3;
    float3 normalMul;
    float  pad4;
    float  metalnessMul;
    float3 pad5;
    float  roughnessMul;
    float3 pad6;
    float  indexOfRefractionMul;
    float3 pad7;
    float4 extraEmissive;
};

struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
    float3 bitangent     : TEXCOORD2;
    float3 tangent       : TEXCOORD3;
	float2 texCoord      : TEXCOORD4;
};

struct PixelOutputType 
{
	float4 normal            : SV_Target0;
    float4 position          : SV_Target1;
    float  metalness         : SV_Target2;
    float  roughness         : SV_Target3;
    float  indexOfRefraction : SV_Target4;
    float4 emissive          : SV_Target5;
    float4 albedoAlpha       : SV_Target6;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

    output.position        = float4( input.positionWorld, 0.0f );
    output.emissive.rgb    = emissiveTexture.Sample( samplerState, input.texCoord ).rgb * emissiveMul + extraEmissive.rgb;
    output.emissive.a      = 0.0f;
	output.albedoAlpha.rgb = albedoTexture.Sample( samplerState, input.texCoord ).rgb * albedoMul;
    output.albedoAlpha.a   = alphaTexture.Sample( samplerState, input.texCoord ).r * alphaMul;

    float3x3 tangentToWorldMatrix = float3x3( 
        normalize( input.tangent ),
        normalize( input.bitangent ),
        normalize( input.normal )
    );
			
	const float3 normalFromMap = ( normalTexture.Sample( samplerState, input.texCoord ).rgb * normalMul - 0.5f ) * 2.0f;
	output.normal = float4( normalize( mul( normalFromMap, tangentToWorldMatrix ) ), 0.0f );

    output.metalness         = metalnessTexture.Sample( samplerState, input.texCoord ).r * metalnessMul;
    output.roughness         = roughnessTexture.Sample( samplerState, input.texCoord ).r * roughnessMul;
    output.indexOfRefraction = indexOfRefractionTexture.Sample( samplerState, input.texCoord ).r * indexOfRefractionMul;

	return output;
}