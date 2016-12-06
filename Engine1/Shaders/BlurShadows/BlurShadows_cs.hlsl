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
};

SamplerState g_linearSamplerState;
SamplerState g_pointSamplerState;

// Input.
Texture2D<float4> g_positionTexture           : register( t0 );
Texture2D<float4> g_normalTexture             : register( t1 ); 
Texture2D<float>  g_illuminationTexture       : register( t2 ); 
Texture2D<float>  g_distanceToOccluderTexture : register( t3 );

// Input / Output.
RWTexture2D<float4> g_blurredIlluminationTexture : register( u0 );

float  readDistToOccluder( float2 texcoords );

static const float Pi = 3.14159265f;

static const float maxDistToOccluder = 999.0f; // Every distance-to-occluder sampled from texture, which is greater than that is not a real value - rather a missing value.

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
    const float2 texcoords = dispatchThreadId.xy / outputTextureSize;
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

    // #TODO: Try to sample form 0 mip to lower mips until sampled value is lower than maximal.
    //const float  distToOccluder      = g_distanceToOccluderTexture[ dispatchThreadId.xy ];
    float distToOccluder = readDistToOccluder( texcoords ); 

    const float  distLightToOccluder = distToLight - distToOccluder;

    const float baseBlurRadius = lightEmitterRadius * ( distToOccluder / distLightToOccluder );

    const float blurRadius = baseBlurRadius / log2( distToCamera + 1.0f );

    float samplingRadius      = 2.0f * min( 1.0f, blurRadius );
    float samplingMipmapLevel = log2( blurRadius / 2.0f );

    float surfaceIllumination = 0.0f;

    if ( samplingRadius <= 0.0001f )
    {
        surfaceIllumination = g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords, 0.0f );
    }
    else
    {
        float2 pixelSize = pixelSize0 * (float)pow( 2, samplingMipmapLevel );
        float sampleCount = 0.0f;

        for ( float y = -samplingRadius; y <= samplingRadius; y += 1.0f ) 
        {
            for ( float x = -samplingRadius; x <= samplingRadius; x += 1.0f ) 
            {
                const float2 texCoordShift = float2( pixelSize.x * x, pixelSize.y * y );

                // #TODO: When we sample outside of light cone - the sample should be black.

                surfaceIllumination += g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + texCoordShift * 0.5f, samplingMipmapLevel );
                sampleCount += 1.0f;
            }
        }

        surfaceIllumination /= sampleCount;
    }

    //const float surfaceIllumination = g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords, mipmapLevel );

    /*const float surfaceIllumination = (
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2( -outputTextureHalfPixelSize.x, -outputTextureHalfPixelSize.y ), mipmapLevel ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2(  outputTextureHalfPixelSize.x, -outputTextureHalfPixelSize.y ), mipmapLevel ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2(  outputTextureHalfPixelSize.x,  outputTextureHalfPixelSize.y ), mipmapLevel ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2( -outputTextureHalfPixelSize.x,  outputTextureHalfPixelSize.y ), mipmapLevel ) ) / 4.0f;*/

	/*const float surfaceIllumination = (
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2( -outputTextureHalfPixelSize.x, -outputTextureHalfPixelSize.y ), 0.0f ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2(  outputTextureHalfPixelSize.x, -outputTextureHalfPixelSize.y ), 0.0f ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2(  outputTextureHalfPixelSize.x,  outputTextureHalfPixelSize.y ), 0.0f ) +
        g_illuminationTexture.SampleLevel( g_linearSamplerState, texcoords + float2( -outputTextureHalfPixelSize.x,  outputTextureHalfPixelSize.y ), 0.0f ) ) / 4.0f;*/

    g_blurredIlluminationTexture[ dispatchThreadId.xy ] = surfaceIllumination;
}

float readDistToOccluder( float2 texcoords )
{
    float distToOccluder;
    
    // Try to sample from 0 mip to lower mips until sampled value is lower than maximal (otherwise it's a missing value).
    for ( float mipmap = 0.0f; mipmap <= 6.0f; mipmap += 1.0f )
    {
        distToOccluder = g_distanceToOccluderTexture.SampleLevel( g_pointSamplerState, texcoords, mipmap + 1.0f ); // TEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEESTTTT MIPMAP + 1

        if ( distToOccluder < maxDistToOccluder )
            return distToOccluder;
    }

    return distToOccluder;
}