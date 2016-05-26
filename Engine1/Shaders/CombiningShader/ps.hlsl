Texture2D       srcTexture          : register( t0 );
Texture2D<uint> edgeDistanceTexture : register( t1 );


SamplerState samplerState;

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

static const float2 imageSize = float2( 1024.0f, 768.0f );

float4 main(PixelInputType input) : SV_Target
{
    float level = 3.0f;

    float4 textureColor = float4( 0.0f, 0.0f, 0.0f, 0.5f );

    const int2 texCoordsInt = (int2)( imageSize * input.texCoord );
    const uint distToEdge   = edgeDistanceTexture.Load( int3( texCoordsInt, 0 ) );

    //level = ( distToEdge == 0 ? 0.0f : round( min( level, floor( log2( distToEdge ) ) ) ) ); // OK - but jerky transitions.
    level = ( distToEdge == 0 ? 0.0f : min( level, log2( distToEdge ) ) );

    { // 9-tap Gaussian blur applied through 4 texture samples (bilinear sampling).
        float2 pixelHalfSize0 = float2( 1.0f / 2048.0f, 1.0f / 1536.0f ); // 1024x768 * 2
        float2 pixelSize = pixelHalfSize0 * 2.0f * level;

        float3 textureColorLeftUp    = srcTexture.SampleLevel( samplerState, input.texCoord + float2( -pixelSize.x, -pixelSize.y ), level ).rgb;
        float3 textureColorRightUp   = srcTexture.SampleLevel( samplerState, input.texCoord + float2(  pixelSize.x, -pixelSize.y ), level ).rgb;
        float3 textureColorLeftDown  = srcTexture.SampleLevel( samplerState, input.texCoord + float2( -pixelSize.x,  pixelSize.y ), level ).rgb;
        float3 textureColorRightDown = srcTexture.SampleLevel( samplerState, input.texCoord + float2(  pixelSize.x,  pixelSize.y ), level ).rgb;

        textureColor.rgb = ( textureColorLeftUp + textureColorRightUp + textureColorLeftDown + textureColorRightDown ) * 0.25f;
    }

    // No blur.
    //textureColor.rgb = srcTexture.SampleLevel( samplerState, input.texCoord, round(level) ).rgb;

    //if ( level >= 3.99f )
    //    textureColor.rgb = float3( 0.0f, 1.0f, 0.0f ); // green
    //else if ( level >= 2.99f )
    //    textureColor.rgb = float3( 1.0f, 1.0f, 0.0f ); // yellow
    //else if ( level >= 1.99f )
    //    textureColor.rgb = float3( 1.0f, 0.0f, 0.0f ); // red
    //else if ( level >= 0.99f )
    //    textureColor.rgb = float3( 1.0f, 0.0f, 1.0f ); // purple
    //else if ( level >= 0.0f )
    //    textureColor.rgb = float3( 1.0f, 1.0f, 1.0f ); // white

    // When sampling mipmaps, sampling between mipmaps gets us free bilinear filtering.
    // Sampling at different level of mipmaps is actually a gaussian blur (if used higher weights for sharper mipmaps).
	//   float4 textureColor = 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 4.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 5.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 6.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 7.5f );
    //   textureColor += 0.2f * srcTexture.SampleLevel( samplerState, input.texCoord, 8.5f );

	return textureColor;
}