SamplerState sampleType;

struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float4 normal        : TEXCOORD1;
};

struct PixelOutputType {
    float4 normal   : SV_Target0;
    float4 position : SV_Target1;
	float4 albedo   : SV_Target2;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

    output.position = float4( input.positionWorld, 0.0f );
	output.albedo   = float4( 0.5f, 0.5f, 0.5f, 1.0f );
	output.normal   = float4( input.normal.xyz, 0.0f );

	return output;
}