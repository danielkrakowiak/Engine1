#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer 
{
	float posX;
	float posY;
	float width;
	float height;
};

struct VertexInputType 
{
	float4 position : POSITION;
	float4 normal   : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PixelInputType 
{
	float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

PixelInputType main( VertexInputType input ) {
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	output.position = input.position;

	output.position.x = posX + ( input.position.x * width );
	output.position.y = posY + ( input.position.y * height );

	output.position.x = ( output.position.x * 2.0f ) - 1.0f;
	output.position.y = ( output.position.y * 2.0f ) - 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	//TODO: should be joined into one matrix
	//float4x4 wvp = mul( mul( worldMatrix, viewMatrix ), projectionMatrix );

	/*output.position = mul( input.position, worldMatrix );
	output.position = mul( output.position, viewMatrix );
	output.position = mul( output.position, projectionMatrix );*/

	//output.position = mul( input.position, wvp );

	// Normal
	input.normal.w = 0.0f;
	//output.normal = mul( input.normal, worldMatrix );
	//output.normal = mul( output.normal, viewMatrix );

	output.texCoord = input.texCoord;


	return output;
}