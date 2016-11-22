#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float2   outputTextureSize;
    float2   pad1;
	float3   lightPosition;
	float    pad2;
    float    lightConeMinDot;
    float3   pad3;
    float3   lightDirection;
    float    pad4;
    float4x4 shadowMapViewMatrix;
    float4x4 shadowMapProjectionMatrix;
};

// Input.
Texture2D<float4> g_rayOrigins       : register( t0 );
Texture2D<float4> g_surfaceNormal    : register( t1 );
//Texture2D<float4> g_contributionTerm : register( t1 ); // How much of the ray color is visible by the camera. Used to avoid checking shadows for useless rays.
Texture2D<float>  g_shadowMap        : register( t2 );

SamplerState      g_pointSamplerState     : register( s0 );
SamplerState      g_linearSamplerState    : register( s1 );

// Input / Output.
RWTexture2D<uint>  g_illumination    : register( u0 );

static const float minHitDist = 0.001f;

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
    const float2 texcoords = (float2)dispatchThreadId.xy / outputTextureSize;

	const float3 rayOrigin = g_rayOrigins.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

	const float3 rayDirBase  = normalize( lightPosition - rayOrigin );

    // If pixel is outside of spot light's cone - ignore.
    if ( dot( lightDirection, -rayDirBase ) < lightConeMinDot ) {
        g_illumination[ dispatchThreadId.xy ] = 0;
        return;
    }

	const float3 surfaceNormal = g_surfaceNormal.SampleLevel( g_linearSamplerState, texcoords, 0.0f ).xyz;

    const float normalLightDot = dot( surfaceNormal, rayDirBase );

    const uint illuminationUint = g_illumination[ dispatchThreadId.xy ];

	// If all position components are zeros - ignore. If face is backfacing the light - ignore (shading will take care of that case). Already in shadow - ignore.
    if ( !any( rayOrigin ) || normalLightDot < 0.0f /*|| dot( float3( 1.0f, 1.0f, 1.0f ), contributionTerm ) < requiredContributionTerm*/ ) { 
        g_illumination[ dispatchThreadId.xy ] = 0;
        return;
    }

    float4 rayOriginInShadowMap;
    rayOriginInShadowMap = mul( float4( rayOrigin, 1.0f ), shadowMapViewMatrix );
    rayOriginInShadowMap = mul( rayOriginInShadowMap, shadowMapProjectionMatrix );

    // Calculate the projected texture coordinates.
    rayOriginInShadowMap.x =  rayOriginInShadowMap.x / rayOriginInShadowMap.w / 2.0f + 0.5f;
    rayOriginInShadowMap.y = -rayOriginInShadowMap.y / rayOriginInShadowMap.w / 2.0f + 0.5f;

    // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
    if( (saturate( rayOriginInShadowMap.x ) == rayOriginInShadowMap.x ) && ( saturate( rayOriginInShadowMap.y ) == rayOriginInShadowMap.y ) )
    {
            // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
        const float shadowMapDepth = g_shadowMap.SampleLevel( g_pointSamplerState, rayOriginInShadowMap.xy, 0.0f ).r; // #TODO: Which sampler to use?
            
        // Calculate the depth of the light.
        const float rayOriginDepth = rayOriginInShadowMap.z / rayOriginInShadowMap.w;

        // Subtract the bias from the lightDepthValue.
        const float bias = 0.0005f * tan( acos( saturate( normalLightDot ) ) );

        // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
        // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
        if( rayOriginDepth - bias > shadowMapDepth )
        {
            g_illumination[ dispatchThreadId.xy ] = 0;
            return;
        }
    }
    else
    {
        g_illumination[ dispatchThreadId.xy ] = 0;
        return;
    }
}