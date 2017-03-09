Texture2D albedoTexture;
SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

static const float zNear   = 0.1f;
static const float zFar    = 100.0f;
static const float zRange  = zFar - zNear;

float linearizeDepth( float depthSample );

float4 main(PixelInputType input) : SV_Target
{
	float4 textureColor = albedoTexture.Sample( samplerState, input.texCoord );

    // Useful to show very high value pixels.
    //////////////////////////////////////////////////////////////////
    //if (textureColor.r > 1.5f)
    //    textureColor /= 100.0f;
    //////////////////////////////////////////////////////////////////

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

float linearizeDepth( float depthSample )
{
    //depthSample = 2.0 * depthSample - 1.0;
    //float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    
    //float zLinear = depthSample / (zFar - depthSample * zRange);
    //const float zLinear = (zNear * zFar) / (zFar - depthSample * zRange);
    
    const float projectionA = zFar / zRange;
    const float projectionB = (-zFar * zNear) / zRange;

    /*return float44(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zFar / ( zFar - zNear ), 1.0f,
		0.0f, 0.0f, -zNear*zFar / ( zFar - zNear ), 0.0f
		);*/
    
    const float linearDepth = projectionB / (depthSample - projectionA);

    return linearDepth;
}