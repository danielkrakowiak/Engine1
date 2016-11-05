#pragma once

#include "float3.h"

namespace Engine1
{
    class int3
    {
        typedef int int3_type;

        public:
        int3_type x;
        int3_type y;
        int3_type z;

        static const int3 ZERO;

        int3() {}
        int3( const int3& a ) : x( a.x ), y( a.y ), z( a.z ) {}

        int3( int3_type x, int3_type y, int3_type z ) : x( x ), y( y ), z( z ) {}

        static int size()
        {
            return 3;
        }

        int3_type* getData()
        {
            return &x;
        }

        bool operator == (const int3& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z;
        }

        bool operator != (const int3& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z;
        }

        int3 operator - () const
        {
            return int3( -x, -y, -z );
        }

        int3 operator + (const int3& vec) const
        {
            return int3( x + vec.x, y + vec.y, z + vec.z );
        }

        int3 operator - (const int3& vec) const
        {
            return int3( x - vec.x, y - vec.y, z - vec.z );
        }

        int3 operator * (const int value) const
        {
            return int3( x * value, y * value, z * value );
        }

        //Per-component multiplication
        int3 operator * (const int3& vec) const
        {
            return int3( x * vec.x, y * vec.y, z * vec.z );
        }

        int3 operator / (const int value) const
        {
            return int3( x / value, y / value, z / value );
        }

        //Per-component division
        int3 operator / (const int3& vec) const
        {
            return int3( x / vec.x, y / vec.y, z / vec.z );
        }

        int3& operator += (const int value)
        {
            x += value;
            y += value;
            z += value;
            return *this;
        }

        int3& operator += (const int3& vec)
        {
            x += vec.x;
            y += vec.y;
            z += vec.z;
            return *this;
        }

        int3& operator -= (const int value)
        {
            x -= value;
            y -= value;
            z -= value;
            return *this;
        }

        int3& operator -= (const int3& vec)
        {
            x -= vec.x;
            y -= vec.y;
            z -= vec.z;
            return *this;
        }

        int3& operator *= (const int value)
        {
            x *= value;
            y *= value;
            z *= value;
            return *this;
        }

        int3& operator /= (const int value)
        {
            x /= value;
            y /= value;
            z /= value;
            return *this;
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

