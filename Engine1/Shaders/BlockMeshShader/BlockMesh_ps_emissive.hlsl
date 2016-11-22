struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
};

struct PixelOutputType {
    //float4 normal            : SV_Target0;
	//float4 position          : SV_Target1;
    //float  metalness         : SV_Target2;
    //float  roughness         : SV_Target3;
    //float  indexOfRefraction : SV_Target4;
    float4 emissive          : SV_Target5;
    //float4 albedo            : SV_Target6;
};

// Note: This is a special shader used to mark objects with an emissive glow.
// It's not supposed to write to any render target except for emmisive - to not change raytracing behavior.

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;
    
    output.emissive = float4( 1.0f, 0.8f, 0.0f, 0.35f );

	return output;
}