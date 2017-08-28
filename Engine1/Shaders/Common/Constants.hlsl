
#ifndef CONSTANTS_HLSL
#define CONSTANTS_HLSL

static const float epsilon = 0.0001f;
static const float e       = 2.71828f;
static const float Pi      = 3.14159265f;
static const float PiHalf  = 1.570796325f;
static const float zNear   = 0.1f;
static const float zFar    = 1000.0f;
static const float zRange  = zFar - zNear;
static const float lightAttenuationFactorCutoff = 0.002f;

static const float3 dielectricSpecularColor = float3( 0.04f, 0.04f, 0.04f );

#endif