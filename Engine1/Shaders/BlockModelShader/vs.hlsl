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
    float4 tangent  : TANGENT;
	float2 texCoord : TEXCOORD0;
};

struct PixelInputType 
{
	float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float3 normal        : TEXCOORD1;
    float3 bitangent     : TEXCOORD2; // Perpendicular to normal and tangent.
    float3 tangent       : TEXCOORD3; // Perpendicular to normal and binormal.
	float2 texCoord      : TEXCOORD4;
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

    // For normal mapping.
	output.tangent   = input.tangent.xyz;
	output.bitangent = cross( output.normal, output.tangent ); // TODO: Normalize needed?

	// Texcoord
	output.texCoord = input.texCoord;

	return output;
}