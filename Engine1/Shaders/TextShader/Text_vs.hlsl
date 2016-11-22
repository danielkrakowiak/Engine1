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
	float2 texCoord : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul( input.position, worldMatrix );
	output.position = mul( output.position, viewMatrix );
	output.position = mul( output.position, projectionMatrix );
	
	output.texCoord = input.texCoord;

    return output;
}