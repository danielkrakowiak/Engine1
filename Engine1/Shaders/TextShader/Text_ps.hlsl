Texture2D characterTexture;
SamplerState samplerState;

cbuffer ConstantBuffer : register( b0 )
{
    float4 color;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PixelOutputType {
    float2 normal   : SV_Target0;
	float4 albedo   : SV_Target1;
};

PixelOutputType main( PixelInputType input )
{
	PixelOutputType output;

	float4 textureColor = characterTexture.Sample( samplerState, input.texCoord );

	output.normal = float2( 0.0f, 0.0f );
    output.albedo = color * float4( textureColor.r, textureColor.r, textureColor.r, textureColor.r );

	return output;
}