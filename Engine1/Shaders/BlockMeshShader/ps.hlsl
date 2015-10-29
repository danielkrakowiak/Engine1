struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float  vertexId : TEXCOORD1;
};

struct PixelOutputType {
	float4 albedo   : SV_Target0;
	float4 normal   : SV_Target1;
	float4 vertexId : SV_Target2;
};

PixelOutputType main( PixelInputType input ) : SV_TARGET
{
	PixelOutputType output;

	output.albedo   = float4( 0.0f, 0.0f, 0.0f, 1.0f );
	output.normal   = input.normal;
	output.vertexId = input.vertexId / 100.0f;

	return output;
}