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
Texture2D<float4> g_positionTexture : register( t0 );
Texture2D<float4> g_normalTexture   : register( t1 ); 
Texture2D<float>  g_distToOccluder  : register( t2 );

// Input / Output.
RWTexture2D<float> g_finalDistToOccluder : register( u0 );

static const float Pi = 3.14159265f;
static const float e = 2.71828f;
static const float positionThresholdFalloff = 0.4f;

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
        g_finalDistToOccluder[ dispatchThreadId.xy ] = 0.0f;
        return;
    }

    const float  centerDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, texcoords, 0.0f );
    const float3 centerPosition       = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f ).xyz; 
    const float  centerWeight         = saturate( 500.0f - centerDistToOccluder );

    const float pixelSizeInWorldSpace = (distToCamera * tan( Pi / 8.0f )) / (outputTextureSize.y * 0.5f);

    const float mipmap = 2.0f;
    const float2 pixelSize = pixelSize0 * pow( 2.0f, mipmap );

    float2 texCoordShift = float2(0.0f, 0.0f); // x horizontal, y - vertical.
    
    float valueSum = centerDistToOccluder * centerWeight;
    float weightSum = 0.00001f + centerWeight;

    // Decrease search radius if central sample is available (is in shadow).
    const float maxSearchRadius = 30.0f;//lerp( 20.0f, 200.0f, min(1.0f, centerDistToOccluder / 0.5f) ) / distToCamera;

    float searchRadius = 2.0f;

    if (centerDistToOccluder > 1.0f && centerDistToOccluder < 500.0)
        searchRadius = 10.0f;
    else if (centerDistToOccluder > 500.0f)
        searchRadius = 25.0f;

    //float searchRadius = lerp( 4.0f, 100.0f, saturate( centerDistToOccluder / 10.0f ) ); /*/ distToCamera*/;
    // Note: Surprisingly this loop doesn't work correctly without unrolling - texcoord offsets are always positive.
    //[unroll(20)] 
    //for (float searchProgress = 0.0f; searchRadius <= maxSearchRadius; searchProgress += 0.05f )
    //{
    //    searchRadius = lerp( 1.0f, 600.0f, pow(searchProgress, 5.0f) ) / distToCamera;

    //searchRadius = 10.0f; // DEEEEEEEEEEEEBUG
    //    texCoordShift.xy = pixelSize * searchRadius;

    //    // #TODO: Should not depend on search radius, because it then changes when zooming in - pointlessly.
    //    const float stepCount = floor( lerp(1.5f, 8.0f, searchRadius * distToCamera / 600.0f) ); 
    //    const float step = 1.0f / stepCount;
    for ( float y = -searchRadius; y <= searchRadius; y += 4.0f )
    {
        for ( float x = -searchRadius; x <= searchRadius; x += 4.0f )
        {
            const float2 sampleTexcoords = texcoords + float2( x * pixelSize.x, y * pixelSize.y );

            const float sampleDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f );

            // Weight discarding samples which are non-shadowed (huge dist-to-ccluder).
            const float sampleWeight1 = saturate( 500.0f - sampleDistToOccluder );

            // Weight discarding samples which are off-screen (zero dist-to-occluder).
            const float sampleWeight2 = saturate( 10000.0f * sampleDistToOccluder );

            const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 
            const float positionDiff = length( samplePosition - centerPosition );

            const float sampleWeight3 = pow( e, -positionDiff * positionDiff / positionThreshold );

            valueSum  += sampleDistToOccluder * sampleWeight1 * sampleWeight2 * sampleWeight3;
            weightSum += sampleWeight1 * sampleWeight2 * sampleWeight3;
        }
    }

        //for ( float y = -1.0f; y <= 1.0f; y += 0.1f/*step*/ )
        //{
        //    for ( float x = -1.0f; x <= 1.0f; x += 0.1f/*step*/ )
        //    {
        //        float2 texcoordShiftMul = normalize( float2( x, y ) );

        //        const float2 sampleTexcoords = texcoords + float2( texcoordShiftMul.x * texCoordShift.x, texcoordShiftMul.y * texCoordShift.y );

        //        #ifdef DEBUG /////////////////////////////////
        //        int2 texcoordsInt = int2( sampleTexcoords * float2( 1024.0f, 768.0f ) );
        //        g_blurredIlluminationTexture[ texcoordsInt ] = 500.0f;
        //        #endif
        //        //////////////////////////////////////////
        //        
        //        const float sampleDistToOccluder = g_distToOccluder.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f );
        //        
        //        // Weight discarding samples which are non-shadowed (huge dist-to-ccluder).
        //        const float sampleWeight1 = saturate( 500.0f - sampleDistToOccluder );

        //        // Weight discarding samples which are off-screen (zero dist-to-occluder).
        //        const float sampleWeight2 = saturate( 10000.0f * sampleDistToOccluder );

        //        const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, sampleTexcoords, 0.0f ).xyz; 

        //        const float positionDiff = length( samplePosition - centerPosition );

        //        const float sampleWeight3 = pow( e, -positionDiff * positionDiff / positionThreshold );

        //        valueSum  += sampleDistToOccluder * sampleWeight1;// * sampleWeight2 * sampleWeight3;
        //        weightSum += sampleWeight1;// * sampleWeight2 * sampleWeight3;
        //    }
        //}
    //}

    const float distToOccluder = valueSum / weightSum;

    g_finalDistToOccluder[ dispatchThreadId.xy ] = distToOccluder;

    //#ifdef DEBUG /////////////////////////////////////
    //g_blurredIlluminationTexture[ dispatchThreadId.xy ] = 500.0f;
    //return;
    //#endif
}
