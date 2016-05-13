#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType 
{
	float4 position : POSITION;
	float4 normal   : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PixelInputType 
{
	float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
	float2 texCoord      : TEXCOORD2;
};

PixelInputType main( VertexInputType input ) {
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul( input.position, worldMatrix );

    output.positionWorld = output.position.xyz;

	output.position = mul( output.position, viewMatrix ); 
	output.position = mul( output.position, projectionMatrix );

	// Normal
	input.normal.w = 0.0f;
	output.normal = mul( input.normal, worldMatrix ).xyz;
	//output.normal = mul( output.normal, viewMatrix );

	// Texcoord
	output.texCoord = input.texCoord;

	return output;
}