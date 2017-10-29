#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

#include "Common\Constants.hlsl"

cbuffer ConstantBuffer : register( b0 )
{
    float3 g_cameraPos;
    float  pad1;
    float3 g_lightPosition;
    float  pad2;
    float  g_lightConeMinDot;
    float3 pad3;
    float3 g_lightDirection;
    float  pad4;
    float  g_lightEmitterRadius;
    float3 pad5;
    float2 g_outputTextureSize;
    float2 pad6;
    float  g_positionThreshold;
    float3 pad7;
};

//#define DEBUG
//#define DEBUG2

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture            : register( t0 );
Texture2D<float4> g_normalTexture              : register( t1 ); 
Texture2D<float>  g_shadowTexture              : register( t2 ); 
Texture2D<float>  g_distToOccluderTexture      : register( t3 );
Texture2D<float>  g_finalDistToOccluderTexture : register( t4 );

// Input / Output.
RWTexture2D<uint> g_blurredShadowTexture : register( u0 );

static const float sampleCountPerSide = 20.0;

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

    const float3 surfacePosition = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 surfaceNormal   = g_normalTexture[ dispatchThreadId.xy ].xyz;

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

    const float distToOccluder = g_finalDistToOccluderTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f );

    const float pixelSizeInWorldSpace   = (distToCamera * tan( Pi / 8.0f )) / (g_outputTextureSize.y * 0.5f);
    const float maxBlurRadiusWorldSpace = 1.0f; 
    const float distLightToOccluder     = distToLight - distToOccluder;
    
    const float blurRadiusInWorldSpace = min( maxBlurRadiusWorldSpace, g_lightEmitterRadius * ( distToOccluder / distLightToOccluder ) );

    // Scale search-radius by abs( dot( surface-normal, camera-dir ) ) - 
    // to decrease search radius when looking at walls/floors at flat angle.
    const float samplingRadiusMul = saturate( abs( dot( surfaceNormal, dirToCamera ) ) );

    const float blurRadiusInScreenSpace = blurRadiusInWorldSpace / pixelSizeInWorldSpace;/// log2( distToCamera + 1.0f );
    const float samplingRadius          = blurRadiusInScreenSpace * samplingRadiusMul;
    //float samplingMipmapLevel = log2( blurRadius / 2.0f );

    const float3 centerPosition = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords, 0.0f ).xyz; 

    float surfaceShadow = 0.0f;
    float sampleCount   = 0.000001f; // Note: Small value to avoid division by zero.

    const float samplingStep = max(1.0, samplingRadius / sampleCountPerSide);

    for ( float y = -samplingRadius; y <= samplingRadius; y += samplingStep) 
    {
        for ( float x = -samplingRadius; x <= samplingRadius; x += samplingStep ) 
        {
            const float2 texCoordShift = float2( pixelSize0.x * x, pixelSize0.y * y );

            //#TODO: When we sample outside of light cone - the sample should be black.

            //#TODO: Sampling could be optimized by sampling higher level mipmap. But be carefull, because such samples are blurred by themselves and can cause shadow leaking etc.
            const float  sampleShadow = g_shadowTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift, 0.0f );
            //#TODO: Should I sample position (bilinear) at the same level as illumination? At the same level so it could contain the same amount of influence from sorounding pixels.
                
            const float3 samplePosition = g_positionTexture.SampleLevel( g_pointSamplerState, texcoords + texCoordShift, 0.0f ).xyz; 
            const float positionDiff    = length( samplePosition - centerPosition );

            const float sampleWeight2 = pow( e, -positionDiff * positionDiff / g_positionThreshold );

            float sampleWeight = sampleWeight2;

            // #TODO: Increase positionThreshold!! 
            surfaceShadow += sampleShadow * sampleWeight;
            sampleCount   += sampleWeight;
        }
    }

    // Note: We assume that at least the central pixel will get accepted - otherwise division by zero would occur.
    surfaceShadow /= sampleCount;

    g_blurredShadowTexture[ dispatchThreadId.xy ] = (int)(255.0 * surfaceShadow);
}