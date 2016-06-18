struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
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
	output.albedo   = float4( 0.5f, 0.5f, 0.5f, 1.0f );

	output.normal   = float4( input.normal, 0.0f );

    output.metalness         = 0.9f;
    output.roughness         = 0.5f;
    output.indexOfRefraction = 0.3f;

	return output;
}