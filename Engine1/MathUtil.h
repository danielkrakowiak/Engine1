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

#include "BoundingBox.h"

//#TODO: use const references in arguments where possible.

namespace Engine1
{
    class BlockActor;

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

        float sign( const float x );

        template< typename T >
        T lerp( const T& value1, const T& value2, float ratio );

        float smoothstep( float value );

        float44 lookAtTransformation( float3 at, float3 eye, float3 up );

        float44 perspectiveProjectionTransformation( float fovY, float aspect, float zNear, float zFar );

        float44 orthographicProjectionTransformation( float width, float height, float zNear, float zFar );

        float33 anglesToRotationMatrix( const float3& rot );

        float3 rotationMatrixToAngles( const float33& mat );

        // Creates an orthogonal rotation matrix with given direction as 3rd row (z-axis).
        float33 directionToRotationMatrix( const float3& direction );

        // Returns <min, max> of the bounding box.
        BoundingBox calculateBoundingBox( const std::vector<float3>& vertices );

        float3 getRayDirectionAtPixel( const float43& cameraPose, const float2& targetPixel,
                                       const float2& screenDimensions, const float fieldOfView );

        // Returns true if the ray intersects the box and returns the distance of the intersection.
        // Returns false if there is no intersection.
        std::tuple< bool, float > intersectRayWithBoundingBox( const float3& rayOriginWorld, const float3& rayDirWorld, const float43& boxPose, const BoundingBox& bbBoxLocal );
        std::tuple< bool, float > intersectRayWithBoundingBox( const float3& rayOriginInBoxSpace, const float3& rayDirInBoxSpace, const BoundingBox& bbBox );

        std::tuple< bool, float > intersectRayWithBlockActor( const float3& rayOriginWorld, const float3& rayDirWorld, const BlockActor& actor, const float maxDist = FLT_MAX );

        bool rayTriangleIntersect( const float3& rayOrigin, const float3& rayDir, const float3& vertexPos1, const float3& vertexPos2, const float3& vertexPos3 );
        float calcDistToTriangle( const float3& rayOrigin, const float3& rayDir, const float3& vertexPos1, const float3& vertexPos2, const float3& vertexPos3 );

        bool isPowerOfTwo( unsigned int number );
        uint32_t ceilToNextPowerOfTwo( uint32_t value );
        uint64_t ceilToNextPowerOfTwo( uint64_t value );

        BoundingBox boundingBoxLocalToWorld( const BoundingBox& bboxInLocalSpace, const float43& bboxPose );

        bool intersectBoundingBoxes( const BoundingBox& bbBox1, const BoundingBox& bbBox2 );
    }

    template< typename T >
    T MathUtil::lerp( const T& value1, const T& value2, float ratio )
    {
        if (ratio <= 0.0f)
            return value1;
        else if (ratio >= 1.0f)
            return value2;
        else
            return value1 * (1.0f - ratio) + value2 * ratio;
    }
}