
#ifndef SHADING_HLSL
#define SHADING_HLSL

#include "Common\Constants.hlsl"

float3 calculateSurfaceDiffuseColor( float surfaceAlpha, float3 surfaceAlbedoColor, float surfaceMetalness );
float3 calculateSurfaceBaseReflectivity( float surfaceAlpha, float3 surfaceAlbedoColor, float surfaceMetalness );
float3 calculateSurfaceLighting( float3 lightColor, float3 surfaceNormal, float3 dirToLight, float3 dirToCamera,
                                 float3 surfaceDiffuseColor, float3 surfaceBaseReflectivity, float surfaceMetalness, float surfaceRoughness );
float3 calculateFresnelTerm( float3 surfaceBaseReflectivity, float normalViewDot );
float3 calculateDiffuseBRDF( float3 surfaceAlbedoColor );
float3 calculateSpecularBRDF( float3 surfaceBaseReflectivity, float surfaceRoughness,
                              float normalHalfDot, float normalViewDot, float normalLightDot, float lightHalfDot );

float3 calculateSurfaceDiffuseColor( float surfaceAlpha, float3 surfaceAlbedoColor, float surfaceMetalness )
{
    return surfaceAlpha * (1.0f - surfaceMetalness) * surfaceAlbedoColor;
}

// Also called "specular color" - how much light gets reflected of a surface when light direction is parallel to surface normal.
float3 calculateSurfaceBaseReflectivity( float surfaceAlpha, float3 surfaceAlbedoColor, float surfaceMetalness )
{
    return surfaceAlpha * lerp( dielectricSpecularColor, surfaceAlbedoColor, surfaceMetalness );
}

float3 calculateSurfaceLighting( float3 lightColor, float3 surfaceNormal, float3 dirToLight, float3 dirToCamera,
                                 float3 surfaceDiffuseColor, float3 surfaceBaseReflectivity, float surfaceMetalness, float surfaceRoughness )
{
    const float3 halfDir        = normalize( dirToLight + dirToCamera );
    const float  lightHalfDot   = max( 0.0, dot( dirToLight, halfDir ) );
    const float  normalHalfDot  = max( 0.0, dot( surfaceNormal, halfDir ) );
    const float  normalLightDot = max( 0.0, dot( surfaceNormal, dirToLight ) );
    const float  normalViewDot  = max( 0.0, dot( surfaceNormal, dirToCamera ) ); // #Note: This dot could be calculated once, as it's the same for all light sources.

    // Fresnel is only used is specular BRDF, but not in diffuse BRDF. 
    // Normally we would expect to multiply specular BRDF by fresnel (reflected light fraction)
    // and diffuse BRDF by (1 - fresnel) (refracted light fraction), but we don't do that for diffuse.
    // It has sth to do with the fact that the refraction occurs twice (light entering surface and leaving it)
    // and diffuse color already contains that in the color itself.
    // I don't full understand yet. Source: https://seblagarde.wordpress.com/2011/08/17/hello-world/

    const float lightAttenuation = 1.0f;

    const float3 diffuseBRDF  = calculateDiffuseBRDF( surfaceDiffuseColor );
    const float3 specularBRDF = calculateSpecularBRDF( surfaceBaseReflectivity, surfaceRoughness,
                                                       normalHalfDot, normalViewDot, normalLightDot, lightHalfDot );

    return normalLightDot * lightAttenuation * lightColor * ( diffuseBRDF + specularBRDF );
}

// Fresnel term - only for microfacets shading - it assumes that micorfacets are directed towards half vector. 
// And we calculate Fresnel term only for those facets, thus overall surface normal is not important. Schlick Approximation.
float3 calculateFresnelTermForMicrofacets( float3 surfaceBaseReflectivity, float lightHalfDot )
{
    return surfaceBaseReflectivity + (( 1.0f - surfaceBaseReflectivity ) * pow( 1.0f - max( 0.0f, lightHalfDot), 5.0f ));
}

// Fresnel term. Schlick Approximation.
float3 calculateFresnelTerm( float3 surfaceBaseReflectivity, float normalViewDot )
{
    return surfaceBaseReflectivity + (( 1.0f - surfaceBaseReflectivity ) * pow( 1.0f - max( 0.0f, normalViewDot), 5.0f ));
}

// Lambertian diffuse BRDF.
float3 calculateDiffuseBRDF( float3 surfaceDiffuseColor )
{
    return surfaceDiffuseColor / Pi;
}

// Cook-Torrance specular BRDF (without Fresnel term).
float3 calculateSpecularBRDF( float3 surfaceBaseReflectivity, float surfaceRoughness,
                              float normalHalfDot, float normalViewDot, float normalLightDot, float lightHalfDot )
{
    // Source: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
    // Great explanation: https://learnopengl.com/#!PBR/Theory

    // Normal distribution function term (Trowbridge-Reitz GGX).
    // Statistically approximates the ratio of microfacets aligned to some (halfway) vector.
    const float a = surfaceRoughness * surfaceRoughness;
    const float aSqr = a * a;
    const float b = ( normalHalfDot * normalHalfDot * ( aSqr - 1.0f ) + 1.0f );
    const float normalDistribution = aSqr / ( Pi * b * b );

    // Specular geometric attenuation term (Schlick-GGX with Smith's method of combining geometric attenuation for view and light directions).
    // The geometry function statistically approximates the ratio of microfacets 
    // that overshadow each other causing light rays to lose their energy in the process.
    const float alpha = surfaceRoughness; // This mapping could be different.
    const float k = (( alpha + 1.0f ) * ( alpha + 1.0f )) / 8.0f; // k - used for direct lighting (point lights, spot lights, NOT image-based lighting).
    const float gv = normalViewDot / ( normalViewDot * ( 1.0f - k ) + k ); // Geometric attenuation for the view direction (masking).
    const float gl = normalLightDot / ( normalLightDot * ( 1.0f - k ) + k ); // Geometric attenuation for the light direction (shadowing).
    const float geometricAttenuation = gv * gl;

    const float3 fresnel = calculateFresnelTermForMicrofacets( surfaceBaseReflectivity, lightHalfDot );

    // 0.001 added to avoid divide by zero.
    const float3 specular = ( normalDistribution * fresnel * geometricAttenuation ) / ( 4.0f * ( normalLightDot * normalViewDot + 0.001 ) );

    return max( 0.0f, specular * surfaceBaseReflectivity );
}

#endif