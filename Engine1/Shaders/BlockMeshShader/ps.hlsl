struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float  vertexId : TEXCOORD1;
};

struct PixelOutputType {
    float2 normal   : SV_Target0;
	float4 albedo   : SV_Target1;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

	output.albedo   = float4( 0.0f, 0.0f, 0.0f, 1.0f );
	output.normal   = input.normal.xy;

	return output;
}