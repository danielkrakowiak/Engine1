#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

struct VertexInputType 
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PixelInputType 
{
	float4 position : SV_POSITION;
	float4 normal : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

PixelInputType main( VertexInputType input ) {
	PixelInputType output;


	output.position = input.position;

	output.position.x = ( output.position.x * 2.0f ) - 1.0f;
	output.position.y = ( output.position.y * 2.0f ) - 1.0f;

    output.normal = input.normal;
	output.texCoord = input.texCoord;

	return output;
}