
#ifndef UTILS_HLSL
#define UTILS_HLSL

#include "Common\Constants.hlsl"

//float linearizeDepth( float depthSample )
//{
//    depthSample = 2.0 * depthSample - 1.0;
//    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
//
//    return zLinear;
//}

float linearizeDepth( float depthSample, float zNear, float zFar )
{
    const float zRange = zFar - zNear;

    const float projectionA = zFar / zRange;
    const float projectionB = (-zFar * zNear) / zRange;
    
    const float linearDepth = projectionB / (depthSample - projectionA);

    return linearDepth;
}

// Reflects the vector which represents an incident ray hitting a surface.
float3 calcReflectedRay( float3 incidentRay, float3 surfaceNormal )
{
    return incidentRay - 2.0f * surfaceNormal * dot( surfaceNormal, incidentRay );
}

// Returns refracted ray or zero vector if there is no refraction.
// refractiveIndex = refractiveIndex1(incident) / refractiveIndex2(refracted)
float3 calcRefractedRay( float3 incidentRay, float3 surfaceNormal, float refractiveIndex )
{
	const float cosI  = -dot( surfaceNormal, incidentRay );
	const float sinT2 = ( refractiveIndex * refractiveIndex ) * ( 1.0f - cosI * cosI );
	
    if ( sinT2 > 1.0f ) 
        return 0.0.xxx; // Deactivate the ray.

	const float cosT = sqrt( 1.0f - sinT2 );
	
	return refractiveIndex * incidentRay + ( refractiveIndex * cosI - cosT ) * surfaceNormal;
}

// pixelPos in range (0,0; screen width, screen height) counting from the top-left corner of the viewport.
// cameraPos - camera position in world space.
// viewportCenter - viewport plane center in world space.
// viewportUp - viewport up vector in world space. It's length equals half of height of the viewport plane.
// viewportRight - viewport right vector in world space. It's length equals half of width of the viewport plane.
// viewportSizeHalf - viewport dimensions in pixels divided by 2.
float3 getPrimaryRayDirection( 
    float2 pixelPos, 
    float3 cameraPos,
    float2 viewportSizeHalf, 
    float3 viewportCenter,
    float3 viewportRight,
    float3 viewportUp )
{
    const float2 pixelShift = (pixelPos - viewportSizeHalf + float2(0.5f, 0.5f)) / viewportSizeHalf; // In range (-1;1)

	const float3 pixelPosWorld = viewportCenter + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    return normalize( pixelPosWorld - cameraPos );
}

float3 calculateReflectionTerm( float3 surfaceSpecularColor, float3 surfaceNormal, float3 dirToCamera )
{
    const float normalViewDot = dot( surfaceNormal, dirToCamera );

    // Fresnel term. Schlick Approximation.
    const float3 fresnel = surfaceSpecularColor + (( 1.0f - surfaceSpecularColor ) * pow( 1.0f - max( 0.0f, normalViewDot), 5.0f ));

    // We normalize specular color because it doesn't influence the amount of reflection directly (only through fresnel), but rather the tint of it.
    // We lerp, bacause at glancing angles, the light color is unaltered in reflection, but at normal incidence the light is modulated by the color of the metal.
    // #TODO: Should the same theory be applied to specular shading?
    const float3 tintColor = lerp( float3( 1.0f, 1.0f, 1.0f ), normalize(surfaceSpecularColor), normalViewDot );
    
    return fresnel * tintColor;
}

// Schlick Approximation.
float calculateRefractionTerm( float3 incidentRay, float3 surfaceNormal, float refractionIndex1, float refractionIndex2 )
{
    float r0 = (refractionIndex1 - refractionIndex2) / (refractionIndex1 + refractionIndex2);
    r0 *= r0;
    float cosX = -dot( surfaceNormal, incidentRay );
    if ( refractionIndex1 > refractionIndex2 )
    {
        const float n = refractionIndex1 / refractionIndex2;
        const float sinT2 = n * n * ( 1.0f - cosX * cosX );

        if ( sinT2 > 1.0f )
            return 1.0f; // Total internal reflection.

        cosX = sqrt( 1.0f - sinT2 );
    }

    const float x = 1.0f - cosX;

    return r0 + (1.0f - r0) * x * x * x * x * x;
}

float3 calculateDiffuseOutputColor( float3 surfaceDiffuseColor, float3 surfaceNormal, float3 lightColor, float3 dirToLight )
{
    return max( 0.0f, dot( dirToLight, surfaceNormal ) ) * surfaceDiffuseColor * lightColor;
}

// Schlick Approximation.
float3 calculateFresnel( float3 specularColor, float3 dirToLight, float3 halfDir )
{
    return specularColor + (( 1.0f - specularColor ) * pow( 1.0f - max( 0.0f, dot( dirToLight, halfDir )), 5.0f ));
}

float3 calculateSpecularOutputColor( float3 surfaceSpecularColor, float surfaceRoughness, float3 surfaceNormal, float3 lightColor, float3 dirToLight, float3 dirToCamera )
{
    // Source: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

    float3 halfDir = normalize( dirToLight + dirToCamera );

    // Normal distribution function term.
    const float a = surfaceRoughness * surfaceRoughness;
    const float aSqr = a * a;
    const float normalHalfDot = saturate( dot( surfaceNormal, halfDir ) );
    const float b = ( normalHalfDot * normalHalfDot * ( aSqr - 1.0f ) + 1.0f );
    const float normalDistribution = aSqr / ( /*Pi **/ b * b );

    // Specular geometric attenuation term.
    const float k = (( surfaceRoughness + 1.0f ) * ( surfaceRoughness + 1.0f )) / 8.0f;
    const float normalViewDot = saturate( dot( surfaceNormal, dirToCamera ) );
    const float gv = normalViewDot / ( normalViewDot * ( 1.0f - k ) + k );
    const float normalLightDot = saturate( dot( surfaceNormal, dirToLight ) );
    const float gl = normalLightDot / ( normalLightDot * ( 1.0f - k ) + k );
    const float geometricAttenuation = gv * gl;

    // Fresnel term. Schlick Approximation.
    const float3 fresnel = surfaceSpecularColor + (( 1.0f - surfaceSpecularColor ) * pow( 1.0f - max( 0.0f, dot( dirToCamera, halfDir )), 5.0f ));

    const float3 specular = ( normalDistribution * fresnel * geometricAttenuation ) / ( 4.0f * ( normalLightDot * normalViewDot ) );

    return max( 0.0f, specular * normalLightDot * surfaceSpecularColor * lightColor );
}

#endif