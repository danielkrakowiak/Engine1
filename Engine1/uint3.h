#pragma once

#undef min
#undef max
#define NOMINMAX

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

        uint3& operator = ( const uint3& vec )
        {
            x = vec.x;
            y = vec.y;
            z = vec.z;

            return *this;
        }

        bool operator == (const uint3& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z;
        }

        bool operator != (const uint3& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z;
        }

        uint3 operator + ( const uint3& vec ) const
        {
            return uint3( x + vec.x, y + vec.y, z + vec.z );
        }

        uint3 operator - ( const uint3& vec ) const
        {
            return uint3( x - vec.x, y - vec.y, z - vec.z );
        }

        explicit operator float3() const
        {
            return float3(
                (float)x,
                (float)y,
                (float)z
                );
        }

        std::string toString() const
        {
            std::string text = "(";

            text += std::to_string(x);
            text += ", ";
            text += std::to_string(y);
            text += ", ";
            text += std::to_string(z);
            text += ")";

            return std::move( text );
        }
    };
}