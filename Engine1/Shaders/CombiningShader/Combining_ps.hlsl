Texture2D<float4> g_colorTexture          : register( t0 );
Texture2D<float4> g_contributionTermRoughnessTexture : register( t1 );
Texture2D<float4> g_normalTexture         : register( t2 );
Texture2D<float4> g_positionTexture       : register( t3 );
Texture2D<float>  g_depthTexture          : register( t4 );
Texture2D<float>  g_hitDistanceTexture    : register( t5 );

SamplerState g_pointSamplerState  : register( s0 );
SamplerState g_linearSamplerState : register( s1 );

cbuffer ConstantBuffer
{
    float  normalThreshold;
    float3 pad1;
    float  positionThresholdSquare;
    float3 pad2;
    float3 cameraPosition;
    float  pad3;
    float2 imageSize;
    float2 contributionTextureFillSize;
    float2 srcTextureFillSize;
    float2 pad4;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

// Position threshold needs to be larger when we sample with larger radius (oversampling ratio). Same for normal threshold.
// Some object id buffer would be useful to reject samples from different objects. It's impossible now to tell whether we hit the same object when sampling radius is big.

// Wanted sampling level should decrease with distance to the pixel - because roughness in screen-space decreaes (and sampling radius).

static const float zNear = 0.1f;
static const float zFar  = 1000.0f;

static const float Pi     = 3.14159265f;
static const float PiHalf = 1.570796325f;

static const float maxDepth = 200.0f;
static const float maxHitDistance = 200.0f; // Should be less than the initial ray length. 

static const float hitDistanceBlendIn  = 20.0f; // Should be less than the initial ray length.
static const float hitDistanceBlendOut = 200.0f; // Should be less than the initial ray length. 

static const float blurRadiusFadeMin = 50.0f;
static const float blurRadiusFadeMax = 100.0f;

// Idea: Could check which level normal differs from highest level normal to learn how far from the edge center pixel is.
// IDEA: Could use depth to decide the maximum sampling level. It's because when we are very close to the object, there is more flat surface on the screen.
// IDEA: When depth is below 1 - skip taking log2 from it. log2 is too steep below 1. Use depth directly or something else.

// IMPORTANT: Only normals and position of the reflecting surface matter (because they influence the direction and shape of ray cone) when accepting/rejecting samples. What's seen by rays is irrelevant.

// IDEA: Select reflection mipmap based on ray distance. If distance is high, we don't care if reflection blurs over the edges? Probably we still care...

// IDEA: Always first check ray distance of neighboring pixels to decide the sampling radius. But the problem is that sampling radius could be different in each direction.

// IDEA: Maybe instead of box shaped sampling, use circle shaped sampling with different radius in each direction and different number of samples in each direction. Quite costly, but may give nice quality.

// IDEA: Sample low-res mipmap of ray distance to check if there is any object around, despite center ray distance being so high.
//       That should cause enough blurring near the edges to forget about the problem.
//       Tested - doesn't work, because difference in depth are so big that edges stay sharp as knife anyways. Plus normal mapping causes some depth values to be really high even in the middle of objects.

// If I sample only the highest reflection mipmap there is no color bleeding over the edges. If we're checking normals and positions...

// We can actually sample low level reflection mipmaps and it still looks good. We should maybe blur them a bit first in a separate pass. Or use 3x3 sampling here.

// How come reflections are not bleeding through the edges even when sampling low-level mipmaps? They actually do for rougher surfaces.

// Size of the object and reflected object on screen depends linearly on camera to object distance and reflective surface to object distance.

// Using smaller sampling radius works only to compansate for camera moving away from surface. It's because ( 1 / depth ) is not a linear function and it gets more linear only for larger depth values. 
// It has more impact for low depth values, rather than high depth values. Wait, but pixel sizes on screen also don't change linearly with depth...
// Maybe it's because properly blurred "mipmap" would look sharper at closer distance, but mipmaps don't look sharper, because they were made to be seen from distance. Artifacts of linear sampling etc?

// CURRENT ISSUES:
// - perceived blur changed when moving camera closer/farther to the reflective surface. Should be the same. Only the distance from the reflective surface to the object should changed perceived blur.

float linearizeDepth( float depthSample );
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal );

float4 main(PixelInputType input) : SV_Target
{
 //   const float3 contributionTermyyyyyyyy = float3( 0.5f, 0.5f, 0.5f ); //g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f ).rgb;
    
 //   float4 reflectionColoryyyyyy = float4( 0.0f, 0.0f, 0.0f, 1.0f );

 //   reflectionColoryyyyyy.rgb += g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord * srcTextureFillSize / imageSize, 0.0f ).rgb;
 //   float3 outputColoryyyyy = contributionTermyyyyyyyy * reflectionColoryyyyyy.rgb;
	//return float4( outputColoryyyyy, 1.0f );


    //////////////////////////////////


    const float depth = linearizeDepth( g_depthTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ) ); // Have to account for fov tanges?

    if ( depth > maxDepth )
        return float4( 0.0f, 0.0f, 0.0f, 0.0f );

    //#TODO: Select mipmap based on central hitDistance - maybe a log of what we need - the rest through avaraging.
    // Why reflection doesn't blur through edges?
    // Check linear/point sampling.
    // Avarage pixels from some region.
    float hitDistance = 0.0f;
    {
        const float mipmap = 5.0f;
        const float2 pixelSize0 = 1.0f / imageSize;
        const float2 pixelSize  = pixelSize0 * pow( 2.0f, mipmap );

        float sampleWeightSum = 0.0f;

        float searchRadius = 5.0f;

        for ( float y = -searchRadius; y <= searchRadius; y += 1.0f )
        {
            for ( float x = -searchRadius; x <= searchRadius; x += 1.0f )
            {
                const float2 sampleTexcoords = input.texCoord + float2( x * pixelSize.x, y * pixelSize.y );

                const float sampleHitDistance = g_hitDistanceTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, mipmap );

                // Weight decreasing importance of samples further from the pixel.
                const float sampleWeight1 = pow(1.0f - (length(searchRadius) / sqrt(searchRadius*searchRadius * 2.0f)), 4.0f);

                // Weight diminishing importance of samples hitting the sky if any other samples are available.
                const float sampleWeight2 = 1.0f - saturate(sampleHitDistance / 200.0f);
			    //if ( sampleHitDistance > maxHitDistance)
				//    sampleWeight1 = 0.0f;

                // Weight discarding samples which are off-screen (zero dist-to-occluder).
                /*const*/ float sampleWeight3 = 1.0f;//saturate( 10000.0f * sampleDistToOccluder );
			    if ( sampleHitDistance < 0.0001f)
				    sampleWeight3 = 0.0f;

                hitDistance += sampleHitDistance * sampleWeight1 * sampleWeight2 * sampleWeight3;
                sampleWeightSum += sampleWeight1 * sampleWeight2 * sampleWeight3;
            }
        }

        hitDistance /= sampleWeightSum;
        //#TODO: Not yet working as expected...
        // If all samples were rejected - use max hit-distance.
        if (sampleWeightSum < 0.0001f)
            hitDistance = 200.0f;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    /*float distanceCloseSampleCount = 0.0f;
    float distanceTotalSampleCount = 0.0f;

    float maxNeighborHitDistance = 0.0f;

    float2 pixelSize5 = ( 1.0f / imageSize ) * 3.0f;
    for ( float x = -6.0f; x <= 6.0f; x += 1.0f )
    {
        for ( float y = -6.0f; y <= 6.0f; y += 1.0f )
        {
            distanceTotalSampleCount += 1.0f;

            float tempHitDistance = g_hitDistanceTexture.SampleLevel( g_pointSamplerState, input.texCoord + float2( x * pixelSize5.x, y * pixelSize5.y ), 0.0f );

            if ( tempHitDistance < 100.0f )
            {
                maxNeighborHitDistance = max( maxNeighborHitDistance, tempHitDistance );
                distanceCloseSampleCount += 1.0f;
            }
        }
    }

     Start blending out only when there is less than 50% of near depth samples.
    distanceTotalSampleCount /= 2.0f;
    distanceCloseSampleCount = min( distanceCloseSampleCount, distanceTotalSampleCount );

    if ( hitDistance > 100.0f )
        hitDistance = lerp( 30.0f, maxNeighborHitDistance, distanceCloseSampleCount / distanceTotalSampleCount );*/

    /////////////////////////////////////////////////////////////////////////////////////

    //if ( hitDistance > maxHitDistance )
    //    return float4( 0.0f, 0.0f, 0.0f, 0.0f );

    const float roughnessMult = 75.0f;
    const float roughness = roughnessMult * g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f ).a;/*g_roughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f )*/;
    float blurRadius = max( 0.0f, log2( hitDistance + 1.0f ) * roughness / log2( depth + 1.0f ) );// * tan( roughness * PiHalf );

    //const float alpha = 1.0f - min( 1.0f,  max( 0.0f, blurRadius - blurRadiusFadeMin ) / ( blurRadiusFadeMax - blurRadiusFadeMin ) );

    //float samplingMipmapTest = log2( blurRadius );

    float samplingRadius      = min( 1.0f, blurRadius );
    float samplingMipmapLevel = log2( blurRadius );

    //const float samplingLevel2 = 0.0f;//min( 4.0f, samplingLevel1 );

    const float3 centerNormal   = g_normalTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;
    const float3 centerPosition = g_positionTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;

    const float3 contributionTerm = g_contributionTermRoughnessTexture.SampleLevel( g_linearSamplerState, input.texCoord * contributionTextureFillSize / imageSize, 0.0f ).rgb;
    
    float4 reflectionColor = float4( 0.0f, 0.0f, 0.0f, 1.0f );

    const int2 texCoordsInt = (int2)( imageSize * input.texCoord );

    ////////////////////////////////////////////////////////////////////////////////////////////////

    //// TODO: generate up to 4th mipmap and sample it instead of full res.
    //const float hitDistance = min( maxHitDistance, g_hitDistanceTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ) );

    //// Check if the ray hit anything.
    ////if ( hitDistance > maxHitDistance )
    ////    return float4( 0.0f, 0.0f, 0.0f, 0.0f );

    //const float roughness = 2.5f;
    //const float coneRadius = hitDistance * roughness;// * tan( roughness * PiHalf );

    //const float samplingLevel0 = max( 0.0f, log2( coneRadius ) );

    //const float depth = max( 0.0f, linearizeDepth( g_depthTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ) ) ); // Have to account for fov tanges?

    //// Decreased by one from the real wanted level.
    //const float samplingLevel1 = depth > 1.0f 
    //    ? min( 8.0f, samplingLevel0 / log2( depth ) )
    //    : min( 8.0f, samplingLevel0 /       depth ); 

    //const int2 texCoordsInt = (int2)( g_imageSize * input.texCoord );

    //const float samplingLevel2 = min( 4.0f, samplingLevel1 );
    
    //float4 reflectionColor = float4( 0.0f, 0.0f, 0.0f, 0.5f );

    ////////////////////////////////////////////////////////////////////////////////////////////////

    if ( /*samplingLevel2 <= 0.0001f*/ samplingRadius <= 0.0001f )
    {
        reflectionColor.rgb = g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord * srcTextureFillSize / imageSize, 0.0f ).rgb;
    }
    else
    {
        //const float samplingRadius = pow( 2.0f, samplingLevel1 - samplingLevel2 );
        const float samplingStep = 1.0f;//max( 1.0f, floor( sqrt( samplingRadius ) ) );

        // Sample highest resolution mipmap to get precise normal at the center pixel.
        //const float3 centerNormal   = g_normalTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;
        //const float3 centerPosition = g_positionTexture.SampleLevel( g_linearSamplerState, input.texCoord, 0.0f ).xyz;

        float2 pixelSize0 = ( 1.0f / imageSize );
        float2 pixelSize = pixelSize0 * (float)pow( 2, samplingMipmapLevel );

        // Always use highest resolution center pixel sample to avoid black pixels.
        float sampleCount = 0.0f;

        const float2 centerTexCoords = input.texCoord;

        //float x = 0.0f;
        //float y = 0.0f;

        // Note: Sample more sparsly when large sampling radius is used to avoid taking too many samples and decreasing performance.
        // Usually large sampling radius is caused by zooming on an object or very high roughness, to skipping some samples causes no problem then.

        // #TODO: What if sampling radius < 1 - step always equals to 1... makes no sense..?
        // Shouldn't this loop be hardcoded from -1 to 1 with step size of 1? Only pixel size would differ...
        // IMPORTANT: Maybe the whole loop can be unrolled?
        for ( float y = -samplingRadius; y <= samplingRadius; y += samplingStep ) 
        {
            for ( float x = -samplingRadius; x <= samplingRadius; x += samplingStep ) 
            {
                // Has to account for depth, because the farther we are from the object,
                // the bigger the difference in world position between neighboring pixels.
                // TODO: How does it relate to sampling level?
                //const float neighborMaxAllowedDistFromCenter = sqrt(positionThresholdSquare) * sqrt(x*x + y*y) /** depth*/;

                const float2 texCoordShift = float2( pixelSize.x * x, pixelSize.y * y );

                //const int2   texCoordsInt2 = (int2)( g_imageSize * (float2(0.5f, 0.5f) + texCoordShift) );

                //if ( abs(texCoordsInt.x - texCoordsInt2.x) <= 1 || abs(texCoordsInt.y - texCoordsInt2.y) <= 1 )
                //    return float4(1.0f, 0.0f, 0.0f, 0.15f);

                //const float  neighborDistToEdge = (float)g_edgeDistanceTexture.Load( int3( texCoordsInt2, 0 ) );

                // Sample lower res normal to get possibly blurred or sharp normal at neighbor pixel.
                //const float3 neightborNormal   = normalize( g_normalTexture.SampleLevel( g_pointSamplerState, centerTexCoords + texCoordShift, samplingMipmapLevel ).xyz );
                //const float3 neightborPosition = g_positionTexture.SampleLevel( g_linearSamplerState, centerTexCoords + texCoordShift, samplingMipmapLevel ).xyz;
                    
                //const float  normalsDot = dot( centerNormal, neightborNormal );
                //const float3 positionDiff = centerPosition - neightborPosition;

                //if ( /*dot( positionDiff, positionDiff )*/ length( positionDiff ) < neighborMaxAllowedDistFromCenter && normalsDot > normalThreshold ) 
                //{
                    // For test.
                    //const float neighborWeight = max( 0.0f, samplingRadius - sqrt(x*x + y*y) );

                    reflectionColor.rgb += /*neighborWeight **/ g_colorTexture.SampleLevel( g_linearSamplerState, (centerTexCoords + texCoordShift * 0.5f)  * srcTextureFillSize / imageSize, samplingMipmapLevel /*samplingLevel2*/ ).rgb;
                    
                    sampleCount += /*neighborWeight;*/ 1.0f;
                //}
            }
        }

        if ( sampleCount > 0 ) {
            // Divide summed color by the number of samples.
            reflectionColor.rgb /= sampleCount;
        } else {
            reflectionColor.rgb += g_colorTexture.SampleLevel( g_linearSamplerState, input.texCoord * srcTextureFillSize / imageSize, 0.0f ).rgb;
        }
    }

    float3 outputColor = contributionTerm * reflectionColor.rgb;
	return float4( outputColor, 1.0f );
}

float linearizeDepth( float depthSample )
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));

    return zLinear;
}

// Reflects the vector which represents an incident ray hitting a surface.
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal )
{
    return incidentRay - 2.0f * surfaceNormal * dot( surfaceNormal, incidentRay );
}