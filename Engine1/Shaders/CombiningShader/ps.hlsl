Texture2D<float4> g_colorTexture           : register( t0 );
Texture2D<float4> g_normalTexture          : register( t1 );
Texture2D<float4> g_positionTexture        : register( t2 );
Texture2D<float>  g_depthTexture           : register( t3 );

SamplerState g_pointSamplerState;
SamplerState g_linearSamplerState;

cbuffer ConstantBuffer
{
    float  normalThreshold;
    float3 pad1;
    float  positionThresholdSquare;
    float3 pad2;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

static const float2 g_imageSize = float2( 1024.0f, 768.0f );

// Position threshold needs to be larger when we sample with larger radius (oversampling ratio). Same for normal threshold.
// Some object id buffer would be useful to reject samples from different objects. It's impossible now to tell whether we hit the same object when sampling radius is big.

// Wanted sampling level should decrease with distance to the pixel - because roughness in screen-space decreaes (and sampling radius).

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

// Idea: Could check which level normal differs from highest level normal to learn how far from the edge center pixel is.
// IDEA: Could use depth to decide the maximum sampling level. It's because when we are very close to the object, there is more flat surface on the screen.

float linearizeDepth( float depthSample );

float4 main(PixelInputType input) : SV_Target
{
    // IDEA: When depth is below 1 - skip taking log2 from it. log2 is too steep below 1. Use depth directly.

    const float depth = max( 0.0f, linearizeDepth( g_depthTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ) ) ); // Have to account for fov tanges?

    float4 textureColor = float4( 0.0f, 0.0f, 0.0f, 0.5f );

    const float samplingLevel0 = 0.0f;

    // Decreased by one from the real wanted level.
    const float samplingLevel1 = depth > 1.0f 
        ? min( 8.0f, samplingLevel0 / log2( depth ) )
        : min( 8.0f, samplingLevel0 /       depth ); 

    const int2 texCoordsInt = (int2)( g_imageSize * input.texCoord );

    const float samplingLevel2 = min( 4.0f, samplingLevel1 );
    const float samplingRadius = pow( 2.0f, samplingLevel1 - samplingLevel2 );

    const float samplingStep = max( 1.0f, floor( sqrt( samplingRadius ) ) );

    if ( samplingLevel2 <= 0.0001f )
    {
        textureColor.rgb = g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).rgb;
    }
    else
    {
        // Sample highest resolution mipmap to get precise normal at the center pixel.
        const float3 centerNormal   = g_normalTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;
        const float3 centerPosition = g_positionTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;

        float2 pixelSize0 = float2( 1.0f / 1024.0f, 1.0f / 768.0f );
        float2 pixelSize = pixelSize0 * (float)pow( 2, samplingLevel2 );

        // Always use highest resolution center pixel sample to avoid black pixels.
        float sampleCount = 0.0f;

        const float2 centerTexCoords = input.texCoord;

        // Note: Sample more sparsly when large sampling radius is used to avoid taking too many samples and decreasing performance.
        // Usually large sampling radius is caused by zooming on an object or very high roughness, to skipping some samples causes no problem then.
        for ( float y = -samplingRadius; y <= samplingRadius; y += samplingStep ) 
        {
            for ( float x = -samplingRadius; x <= samplingRadius; x += samplingStep ) 
            {
                // Has to account for depth, because the farther we are from the object,
                // the bigger the difference in world position between neighboring pixels.
                // TODO: How does it relate to sampling level?
                const float neighborMaxAllowedDistFromCenter = sqrt(positionThresholdSquare) * sqrt(x*x + y*y) * depth;

                const float2 texCoordShift = float2( pixelSize.x * x, pixelSize.y * y );

                //const int2   texCoordsInt2 = (int2)( g_imageSize * (input.texCoord + texCoordShift) );
                const int2   texCoordsInt2 = (int2)( g_imageSize * (float2(0.5f, 0.5f) + texCoordShift) );

                if ( abs(texCoordsInt.x - texCoordsInt2.x) <= 1 || abs(texCoordsInt.y - texCoordsInt2.y) <= 1 )
                    return float4(1.0f, 0.0f, 0.0f, 0.15f);

                //const float  neighborDistToEdge = (float)g_edgeDistanceTexture.Load( int3( texCoordsInt2, 0 ) );

                // Sample lower res normal to get possibly blurred or sharp normal at neighbor pixel.
                const float3 neightborNormal   = normalize( g_normalTexture.SampleLevel( g_pointSamplerState, centerTexCoords + texCoordShift, samplingLevel2 ).xyz );
                const float3 neightborPosition = g_positionTexture.SampleLevel( g_linearSamplerState, centerTexCoords + texCoordShift, samplingLevel2 ).xyz;
                    
                const float  normalsDot = dot( centerNormal, neightborNormal );
                const float3 positionDiff = centerPosition - neightborPosition;

                // Why normals sometimes get 0 correct samples, when they should get at least several? The same with position. Because of avaraged colors in mipmaps! Obviously...

                //// PROBLEM: normal texture has much higher resolution, so normals near the edge can match, while colors near the edge don't match. 
                if ( /*dot( positionDiff, positionDiff )*/ length( positionDiff ) < neighborMaxAllowedDistFromCenter && normalsDot > normalThreshold ) 
                {
                    textureColor.rgb += g_colorTexture.SampleLevel( g_linearSamplerState, centerTexCoords + texCoordShift * 0.5f, samplingLevel2 ).rgb;
                    
                    sampleCount += 1.0f;
                }
            }
        }

        //textureColor.r = sampleCount / 5.0f;

        if ( sampleCount > 0 ) {
            // Divide summed color by the number of samples.
            textureColor.rgb /= sampleCount;
        } else {
            textureColor.rgb += g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).rgb;
        }
    }

	return textureColor;
}

float linearizeDepth( float depthSample )
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));

    return zLinear;
}