#include "BlurShadowPatternCommon_cs.hlsl"

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

    // Note: Calculate texcoords for the pixel center.
    const float2 texcoords = ((float2)dispatchThreadId.xy + 0.5f) / g_outputTextureSize;

    const float2 pixelSize0 = 1.0f / g_outputTextureSize;

    const float3 surfacePosition = g_positionTexture.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz; 
    const float3 surfaceNormal   = g_normalTexture.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz; 

    const float3 vectorToCamera = g_cameraPos - surfacePosition;
    const float3 dirToCamera    = normalize( vectorToCamera );
    const float  distToCamera   = length( vectorToCamera );

    const float3 vectorToLight  = g_lightPosition.xyz - surfacePosition;
    const float3 dirToLight     = normalize( vectorToLight );
    const float  distToLight    = length( vectorToLight );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( g_lightDirection, -dirToLight ) < g_lightConeMinDot ) {
        g_blurredShadowTexture[ dispatchThreadId.xy ] = 255;
        return;
    }

    const float distToOccluder = g_finalDistToOccluderTexture.SampleLevel( g_linearSamplerState, texcoords, 0.0f );

    const float pixelSizeInWorldSpace   = (distToCamera * tan( Pi / 8.0f )) / (g_outputTextureSize.y * 0.5f);
    const float maxBlurRadiusWorldSpace = 1.0f; 
    const float distLightToOccluder     = distToLight - distToOccluder;
    
    const float blurRadiusInWorldSpace = min( maxBlurRadiusWorldSpace, g_lightEmitterRadius * ( distToOccluder / distLightToOccluder ) );

    // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
    // to decrease search radius when looking at walls/floors at flat angle.
    const float samplingRadiusMul = saturate( abs( dot( surfaceNormal, dirToCamera ) ) );

    const float blurRadiusInScreenSpace = blurRadiusInWorldSpace / pixelSizeInWorldSpace;/// log2( distToCamera + 1.0f );

    // #TODO: pixelSizeMul should depend on blurRadiusInScreenSpace - so only soft shadow samples higher shadow mipmaps.
    // Otherwise hard shadow will get blurred and aliased (as it is now).
    const float  pixelSizeMul = pow(2.0, g_shadowSampleMipmapLevel);
    const float2 pixelSize    = pixelSize0 * pixelSizeMul;

    const float samplingRadius          = max(1.0, min( samplingRadiusBase, blurRadiusInScreenSpace) * getSampleWeightLowerThan( distToOccluder, 750.0 ) / pixelSizeMul);

    float surfaceShadow = 0.0f;
    float sampleCount   = 0.000001f; // Note: Small value to avoid division by zero.

    const float y = 0.0f;

    for ( float x = -samplingRadius; x <= samplingRadius; x += 1.0 ) 
    {
        const float2 texCoordShift = float2( x, y ) * pixelSize;

        //#TODO: When we sample outside of light cone - the sample should be black.

        //#TODO: Discard out-of-screen samples.

        //#TODO: Sampling could be optimized by sampling higher level mipmap. But be carefull, because such samples are blurred by themselves and can cause shadow leaking etc.
        const float  sampleShadow = g_shadowTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, g_shadowSampleMipmapLevel );
                
        const float3 samplePosition = g_positionTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, g_positionSampleMipmapLevel ).xyz; 
        const float3 sampleNormal   = g_normalTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, g_normalSampleMipmapLevel ).xyz; 

        // Calculate expected distance in world space between sample and center 
        // assuming they are on flat surface facing the camera.
        const float distanceInPixelsSqr  = x*x + y*y;
        const float distanceInPixels     = sqrt(distanceInPixelsSqr);
        const float expectedPositionDiff = distanceInPixels * pixelSizeInWorldSpace;

        const float positionDiff           = length( samplePosition - surfacePosition );
        const float positionDiffErrorRatio = max( 0.0, positionDiff - expectedPositionDiff) / expectedPositionDiff;
        const float normalDiff             = 1.0 - max( 0.0, dot( sampleNormal, surfaceNormal ));

        //#TODO: g_positionThreshold is unused. Should I use getSampleWeightSimilarSmooth for positionDiffErrorRatio?
        const float sampleWeight1 = lerp( 1.0, 0.0, saturate( positionDiffErrorRatio ) ); 
        const float sampleWeight2 = getSampleWeightSimilarSmooth( normalDiff, g_normalThreshold );

        float sampleWeight = sampleWeight1 * sampleWeight2;

        surfaceShadow += sampleShadow * sampleWeight;
        sampleCount   += sampleWeight;
    }

    surfaceShadow /= sampleCount;

    g_blurredShadowTexture[ dispatchThreadId.xy ] = (uint)( ceil( surfaceShadow * 255.0 ) );
}