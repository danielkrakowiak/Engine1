#pragma once

#include "float3.h"

namespace Engine1
{
    class uint3
    {
        typedef unsigned int uint3_type;

        public:
        uint3_type x;
        uint3_type y;
        uint3_type z;

        static const uint3 ZERO;

        uint3() {}
        uint3( const uint3& a ) : x( a.x ), y( a.y ), z( a.z ) {}

        uint3( uint3_type x, uint3_type y, uint3_type z ) : x( x ), y( y ), z( z ) {}

        static int size()
        {
            return 3;
        }

        uint3_type* getData()
        {
            return &x;
        }

        bool operator == (const uint3& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z;
        }

        bool operator != (const uint3& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z;
        }

        explicit operator float3() const
        {
            return float3(
                (float)x,
                (float)y,
                (float)z
                );
        }
    };
}