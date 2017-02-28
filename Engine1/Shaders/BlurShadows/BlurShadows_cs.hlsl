#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
    float3 lightPosition;
    float  pad2;
    float  lightConeMinDot;
    float3 pad3;
    float3 lightDirection;
    float  pad4;
    float  lightEmitterRadius;
    float3 pad5;
    float2 outputTextureSize;
    float2 pad6;
    float  positionThreshold;
    float3 pad7;
};

//#define DEBUG
//#define DEBUG2

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture                  : register( t0 );
Texture2D<float4> g_normalTexture                    : register( t1 ); 
Texture2D<float>  g_hardIlluminationTexture          : register( t2 ); 
Texture2D<float>  g_softIlluminationTexture          : register( t3 ); 
Texture2D<float>  g_distToOccluder                   : register( t4 );

// Input / Output.
RWTexture2D<float4> g_blurredIlluminationTexture : register( u0 );

float readIlluminationBlurRadius( float2 texcoords );

static const float Pi = 3.14159265f;
static const float e = 2.71828f;
static const float positionThresholdFalloff = 0.4f;

static const float maxBlurRadius = 999.0f; // Every distance-to-occluder sampled from texture, which is greater than that is not a real value - rather a missing value.

bool canUseSample( const float3 blurCenterPosition, const float3 samplePosition, const float blurRadiusInWorldSpace, const float positionThreshold );

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(16, 16, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    #ifdef DEBUG ///////////////////////////////////////////////////////
    if ( dispatchThreadId.x != 512 || dispatchThreadId.y != 384 )
        return;
    #endif
    /////////////////////////////////////////////////////////////////

    //const float2 texcoords = dispatchThreadId.xy / outputTextureSize;
    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / outputTextureSize;

    //const float2 outputTextureHalfPixelSize = 1.0f / outputTextureSize; // Should be illumination texture size?

    const float2 pixelSize0 = 1.0f / outputTextureSize;

    const float3 surfacePosition     = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceNormal       = g_normalTexture[ dispatchThreadId.xy ].xyz;

    const float3 vectorToCamera = cameraPos - surfacePosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    const float3 vectorToLight       = lightPosition.xyz - surfacePosition;
    const float3 dirToLight          = normalize( vectorToLight );
    const float  distToLight         = length( vectorToLight );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( lightDirection, -dirToLight ) < lightConeMinDot ) {
        g_blurredIlluminationTexture[ dispatchThreadId.xy ] = 0;
        return;
    }

    ///////////////////// TEST POWER VR METHOD OF FINDING BLUR RADIUS /////////////////////////
    const float  centerIllumination   = g_hardIlluminationTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f );
    const float  centerDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, texcoords, 0.0f );
    const float3 centerPosition       = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f ).xyz; 
    const float  centerWeight         = max( 0.0f, 500.0f - centerDistToOccluder );

    const float pixelSizeInWorldSpace = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);

    const float2 pixelSize = pixelSize0;//float2(1.0f/1024.0f, 1.0f / 768.0f);///*pow(2.0f, mipmap) **/ pixelSize0;

    float2 texCoordShift = float2(0.0f, 0.0f); // x horizontal, y - vertical.
    
    float valueSum = centerDistToOccluder * centerWeight;
    float weightSum = 0.00001f + centerWeight;

    // Decrease search radius if central sample is available (is in shadow).
    const float maxSearchRadius = lerp( 20.0f, 200.0f, min(1.0f, centerDistToOccluder / 0.5f) ) / distToCamera;

    #ifdef DEBUG2 /////////////////////////////////////
    g_blurredIlluminationTexture[ dispatchThreadId.xy ] = maxSearchRadius / 600.0f;
    return;
    #endif

    float searchRadius = 0.0f;
    // Note: Surprisingly this loop doesn't work correctly without unrolling - texcoord offsets are always positive.
    [unroll(20)] 
    for (float searchProgress = 0.0f; searchRadius <= maxSearchRadius; searchProgress += 0.05f )
    {
        searchRadius = lerp( 1.0f, 600.0f, pow(searchProgress, 5.0f) ) / distToCamera;

        texCoordShift.xy = pixelSize * searchRadius;

        // #TODO: Should not depend on search radius, because it then changes when zooming in - pointlessly.
        const float stepCount = floor( lerp(1.5f, 8.0f, searchRadius * distToCamera / 600.0f) ); 
        const float step = 1.0f / stepCount;

        for ( float y = -1.0f; y <= 1.0f; y += step )
        {
            for ( float x = -1.0f; x <= 1.0f; x += step )
            {
                float2 texcoordShiftMul = normalize( float2( x, y ) );

                #ifdef DEBUG /////////////////////////////////
                int2 texcoordsInt = int2( (texcoords + float2( texcoordShiftMul.x * texCoordShift.x, texcoordShiftMul.y * texCoordShift.y )) * float2( 1024.0f, 768.0f ) );
                g_blurredIlluminationTexture[ texcoordsInt ] = 500.0f;
                #endif
                //////////////////////////////////////////
                
                const float sampleDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, texcoords + float2( texcoordShiftMul.x * texCoordShift.x, texcoordShiftMul.y * texCoordShift.y ), 0.0f );
                
                // Weight discarding samples which are non-shadowed (huge dist-to-ccluder).
                const float sampleWeight1 = max( 0.0f, 500.0f - sampleDistToOccluder );

                // Weight discarding samples which are off-screen (zero dist-to-occluder).
                const float sampleWeight2 = min( 1.0f, 10000.0f * sampleDistToOccluder );

                const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords + float2( x * texCoordShift.x, y * texCoordShift.y ), 0.0f ).xyz; 

                const float positionDiff = length( samplePosition - centerPosition );

                const float sampleWeight3 = pow( e, -positionDiff * positionDiff / positionThreshold );

                valueSum  += sampleDistToOccluder * sampleWeight1 * sampleWeight2 * sampleWeight3;
                weightSum += sampleWeight1 * sampleWeight2 * sampleWeight3;
            }
        }
    }

    float distToOccluder = valueSum / weightSum;

    #ifdef DEBUG /////////////////////////////////////
    g_blurredIlluminationTexture[ dispatchThreadId.xy ] = 500.0f;
    return;
    #endif
    /////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////
    
    //const float minBlurRadiusInWorldSpace = g_distToOccluder.SampleLevel( g_linearSamplerState, texcoords, 3.0f );
    //const float pixelSizeInWorldSpace      = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);
    const float maxIlluminationWorldSpaceBlurRadius = 1.0f; 
    const float distLightToOccluder = distToLight - distToOccluder;
    
    const float blurRadiusInWorldSpace = min( maxIlluminationWorldSpaceBlurRadius, lightEmitterRadius * ( distToOccluder / distLightToOccluder ) );

    const float minBlurRadiusInScreenSpace = blurRadiusInWorldSpace / pixelSizeInWorldSpace;/// log2( distToCamera + 1.0f );
    const float samplingRadius             = minBlurRadiusInScreenSpace;
    //float samplingMipmapLevel = log2( blurRadius / 2.0f );

    //const float illuminationSoftness = min( 1.0f, minBlurRadiusInWorldSpace / 1.0f );
    //const float illuminationHardness = 1.0f - illuminationSoftness;

    float surfaceIllumination = 0.0f;

    if ( samplingRadius <= 0.0001f || samplingRadius > maxBlurRadius )
    {
        surfaceIllumination = g_hardIlluminationTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f );
    }
    else
    {
        // Note: It's impossible to tell how far samples are from each other in world space
        // by using blur radius in screen space (because it depends on camera distance from surface).
        // So we calculate blur radius in world space.
        //const float blurRadiusInWorldSpace = blurRadius * log2( distToCamera + 1.0f );

        float sampleCount = 0.0f;

        const float samplingStep = 0.05f * samplingRadius;

        for ( float y = -samplingRadius; y <= samplingRadius; y += samplingStep) 
        {
            for ( float x = -samplingRadius; x <= samplingRadius; x += samplingStep ) 
            {
                const float2 texCoordShift = float2( pixelSize0.x * x, pixelSize0.y * y );

                //#TODO: When we sample outside of light cone - the sample should be black.

                //float sampleDistToOccluder  = g_distToOccluder.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, 1.5f );
                //const float sampleIlluminationSoftness = min( 1.0f, sampleBlurRadiusInWorldSpace / 1.0f );
                //const float sampleIlluminationHardness = 1.0f - sampleIlluminationSoftness;

                //#TODO: Sampling could be optimized by sampling higher level mipmap. But be carefull, because such samples are blurred by themselves and can cause shadow leaking etc.
                const float  sampleHardIllumination = g_hardIlluminationTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, 0.0f );
                //const float  sampleSoftIllumination = g_softIlluminationTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, 0.0f );
                //const float  sampleBlurRadius   = g_illuminationBlurRadiusTexture.SampleLevel( g_pointSamplerState, texcoords + texCoordShift, 0.0f );
                //#TODO: Should I sample position (bilinear) at the same level as illumination? At the same level so it could contain the same amount of influence from sorounding pixels.
                
                //const float sampleIllumination = illuminationSoftness * sampleSoftIllumination;//lerp( sampleHardIllumination, sampleSoftIllumination, illuminationSoftness );//illuminationHardness * sampleHardIllumination + illuminationSoftness * sampleSoftIllumination;
                //const float sampleIllumination1 = min( 1.0f, illuminationHardness / sampleIlluminationHardness ) * sampleHardIllumination;
                //const float sampleIllumination2 = min( 1.0f, illuminationSoftness / sampleIlluminationSoftness ) * sampleSoftIllumination;
                const float sampleIllumination = sampleHardIllumination;// + sampleHardIllumination;*/ //sampleIllumination1 + sampleIllumination2;
                
                const float3 samplePosition     = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords + texCoordShift, 0.0f ).xyz; 

                const float positionDiff = length( samplePosition - centerPosition );

                const float sampleWeight2 = pow( e, -positionDiff * positionDiff / positionThreshold );

                float sampleWeight = sampleWeight2;

                //if (samplePointIllumination < 0.8f ) { 
                //    sampleWeight = max( 0.0f, 1.0f - abs(blurRadius - sampleBlurRadius) / blurRadius );
                //}

                // #TODO: Increase positionThreshold!! 
                bool useSample = true;///*sampleBlurRadiusInWorldSpace >= minBlurRadiusInWorldSpace &&*/ canUseSample( surfacePosition, samplePosition, minBlurRadiusInWorldSpace, positionThreshold );

                if ( useSample )
                {
                    surfaceIllumination += sampleIllumination * sampleWeight;

                    // Add fully lit samples twice.
                    //if ( sampleIllumination > 0.99f )
                    //    surfaceIllumination += sampleIllumination;
                    
                    //const float weightFromIllumination = sampleIllumination * 0.5f + 0.5f; // Note: To correct the transition from black to white, which would otherwise be from black to gray.
                    

                    sampleCount += sampleWeight;// * weightFromIllumination;// * weightFromPositionDiff;
                }
            }
        }

        // Note: We assume that at least the central pixel will get accepted - otherwise division by zero would occur.
        surfaceIllumination /= sampleCount;
    }

    g_blurredIlluminationTexture[ dispatchThreadId.xy ] = surfaceIllumination;
}

bool canUseSample( const float3 blurCenterPosition, const float3 samplePosition, const float blurRadiusInWorldSpace, const float positionThreshold )
{
    // Dev: Tried to compare normals here as well, but it didn't prove to be useful.
    
    //#TODO: Could be optimized by using "dot" instead of "length" and comparing against squared position threshold. It would save one square root.
    const float  posDiff    = length( samplePosition - blurCenterPosition );
    const float  maxPosDiff = positionThreshold * blurRadiusInWorldSpace;

    return posDiff < maxPosDiff;
}