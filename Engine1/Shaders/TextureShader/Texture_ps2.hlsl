Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

//static const float zNear = 0.1f;
//static const float zFar  = 1000.0f;

float linearizeDepth( float depthSample );

float4 main(PixelInputType input) : SV_Target
{
	float4 textureColor = float4( albedoTexture.Sample( samplerState, input.texCoord ).aaa, 1.0f );

	return textureColor;
}

// For rendering depth buffer.
//float4 main(PixelInputType input) : SV_Target
//{
//	float4 textureColor = albedoTexture.Sample( samplerState, input.texCoord );

//    float depth = linearizeDepth( textureColor.r ) / zFar;
//    depth *= 5.0f; // To make differences in depth more visible.

//    return float4( depth.rrr, 1.0f );
//}

//float linearizeDepth( float depthSample )
//{
//    depthSample = 2.0 * depthSample - 1.0;
//    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));

//    return zLinear;
//}