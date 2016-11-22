#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#define MAX_BONES_COUNT 255
#define MAX_BONES_PER_VERTEX_COUNT 4

cbuffer ConstantBuffer
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4x4 boneBindPose[ MAX_BONES_COUNT ];
	float4x4 boneBindPoseInv[ MAX_BONES_COUNT ];
	float4x4 bonePose[ MAX_BONES_COUNT ];
	uint     bonesPerVertex;
};

struct VertexInputType
{
    float4 position      : POSITION;
	uint4  boneIndices   : BLENDINDICES;
	float4 vertexWeights : BLENDWEIGHT;
	float4 normal        : NORMAL;
};

struct PixelInputType
{
    float4 position      : SV_POSITION;
    float3 positionWorld : TEXCOORD0;
	float4 normal        : TEXCOORD1;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

	input.position.w = 1.0f;
	input.normal.w = 0.0f;

	output.position = float4( 0.0f, 0.0f, 0.0f, 1.0f );
	output.normal = float4( 0.0f, 0.0f, 0.0f, 0.0f );

	[ unroll( MAX_BONES_PER_VERTEX_COUNT ) ] // Unroll is required for the shader to compile - it removes dynamic indexing of input attributes.
	for ( uint i = 0; i < bonesPerVertex; ++i )
	{
		if ( input.boneIndices[ i ] != 0 && input.vertexWeights[ i ] > 0.0f ) {
			const uint boneIndex = input.boneIndices[ i ] - 1;
			float4 position;
			position = mul( input.position, boneBindPose[ boneIndex ] ); //move to bone's coordinate system
			position = mul( position, mul( bonePose[ boneIndex ], boneBindPose[ boneIndex ] ) ); //move vertex along with the bone
			position = mul( position, boneBindPoseInv[ boneIndex ] ); //move to mesh's coordinate system

			output.position += position * input.vertexWeights[ i ];

			float4 normal;
			normal = mul( input.normal, boneBindPose[ boneIndex ] ); //move to bone's coordinate system
			normal = mul( normal, mul( bonePose[ boneIndex ], boneBindPose[ boneIndex ] ) ); //move vertex along with the bone
			normal = mul( normal, boneBindPoseInv[ boneIndex ] ); //move to mesh's coordinate system

			output.normal += normal * input.vertexWeights[ i ];
		}
	}

	output.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul( output.position, worldMatrix );

    output.positionWorld = output.position.xyz;

    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Normal
	output.normal = normalize( output.normal );
	output.normal = mul( output.normal, worldMatrix );

    return output;
}