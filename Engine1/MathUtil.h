#pragma once

#include <cmath>
#include <float.h>
#include <tuple>
#include <vector>

#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "quat.h"
#include "float44.h"
#include "uint3.h"

//#TODO: use const references in arguments where possible.

namespace Engine1
{
    namespace MathUtil
    {
        const float pi = 3.1415926535897932384626433832795f;
        const float piTwo = 6.283185307179586476925286766559f;
        const float piHalf = 1.5707963267948966192313216916398f;
        const float epsilon = FLT_EPSILON;
        const float epsilonFive = FLT_EPSILON * 5.0f;
        const float epsilonTwenty = FLT_EPSILON * 20.0f;
        const float epsilonFifty = FLT_EPSILON * 50.0f;

        float radiansToDegrees( float radians );
        float degreesToRadians( float degrees );

        // maxRelDiff - used when comparing larger numbers. Value depends on the context - value of 0.01f means that two numbers are equal if the difference is <= 1%. 
        // maxAbsDiff - used when comparing small numbers (near zero). Should be something around FLT_EPSILON * some_small_number (ex.: FLT_EPSILON * 4.0f)
        // numbers are considered equal if they fulfill at least one of the above conditions
        //source: http://www.altdevblogaday.com/2012/02/22/comparing-floating-point-numbers-2012-edition/
        bool areEqual( float A, float B, float maxRelDiff = 0.0f, float maxAbsDiff = epsilonFive );

        bool areEqual( double A, double B, double maxRelDiff = 0.0f, double maxAbsDiff = epsilonFive );

        // maxRelDiff - used when comparing larger numbers. Value depends on the context - value of 0.01f means that two numbers are equal if the difference is <= 1%. 
        // maxAbsDiff - used when comparing small numbers (near zero). Should be something around FLT_EPSILON * some_small_number (ex.: FLT_EPSILON * 4.0f)
        // numbers are considered equal if they fulfill at least one of the above conditions
        bool areEqual( float2 A, float2 B, float maxRelDiff = 0.0f, float maxAbsDiff = epsilonFive );
        bool areEqual( float3 A, float3 B, float maxRelDiff = 0.0f, float maxAbsDiff = epsilonFive );
        bool areEqual( float4 A, float4 B, float maxRelDiff = 0.0f, float maxAbsDiff = epsilonFive );
        bool areEqual( quat A, quat B, float maxRelDiff = 0.0f, float maxAbsDiff = epsilonFive );

        float44 lookAtTransformation( float3 at, float3 eye, float3 up );

        float44 perspectiveProjectionTransformation( float fovY, float aspect, float zNear, float zFar );

        float44 orthographicProjectionTransformation( float width, float height, float zNear, float zFar );

        float33 anglesToRotationMatrix( const float3& rot );

        float3 rotationMatrixToAngles( const float33& mat );

        // Returns <min, max> of the bounding box.
        std::tuple<float3, float3> calculateBoundingBox( const std::vector<float3>& vertices );
    }
}