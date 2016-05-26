Texture2D srcTexture;

SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_Target
{
    const float level = 4.0f; // Availabe values: 0, 1, 2, 3, 4, 5, 6...

    float4 textureColor = float4( 0.0f, 0.0f, 0.0f, 1.0f );

    { // 9-tap Gaussian blur applied through 4 texture samples (bilinear sampling).
        float2 pixelHalfSize0 = float2( 1.0f / 2048.0f, 1.0f / 1536.0f ); // 1024x768 * 2
        float2 pixelSize = pixelHalfSize0 * 2.0f * level;

        float4 textureColorLeftUp    = srcTexture.SampleLevel( samplerState, input.texCoord + float2( -pixelSize.x, -pixelSize.y ), level );
        float4 textureColorRightUp   = srcTexture.SampleLevel( samplerState, input.texCoord + float2(  pixelSize.x, -pixelSize.y ), level );
        float4 textureColorLeftDown  = srcTexture.SampleLevel( samplerState, input.texCoord + float2( -pixelSize.x,  pixelSize.y ), level );
        float4 textureColorRightDown = srcTexture.SampleLevel( samplerState, input.texCoord + float2(  pixelSize.x,  pixelSize.y ), level );

        textureColor = ( textureColorLeftUp + textureColorRightUp + textureColorLeftDown + textureColorRightDown ) * 0.25f;
    }

    // No blur.
    //textureColor = srcTexture.SampleLevel( samplerState, input.texCoord, level );

    // When sampling mipmaps, sampling between mipmaps gets us free bilinear filtering.
    // Sampling at different level of mipmaps is actually a gaussian blur (if used higher weights for sharper mipmaps).
	//   float4 textureColor = 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 4.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 5.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 6.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 7.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 8.5f );

    textureColor.a = 0.5f;

	return textureColor;
}