#pragma once

#include <cmath>

#include "float4.h"

namespace Engine1
{
    class uchar4
    {
        public:
        unsigned char x;
        unsigned char y;
        unsigned char z;
        unsigned char w;

        static const uchar4 ZERO;

        uchar4() {}

        uchar4( const uchar4& a ) : x( a.x ), y( a.y ), z( a.z ), w( a.w ) {}

        uchar4( unsigned char x, unsigned char y, unsigned char z, unsigned char w ) : x( x ), y( y ), z( z ), w( w ) {}

        static int size()
        {
            return 4;
        }

        unsigned char* getData()
        {
            return &x;
        }

        uchar4& operator = (const uchar4& vec)
        {
            x = vec.x;
            y = vec.y;
            z = vec.z;
            w = vec.w;

            return *this;
        }

        bool operator == (const uchar4& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
        }

        bool operator != (const uchar4& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z || w != vec.w;
        }

        uchar4 operator - () const
        {
            return uchar4( -x, -y, -z, -w );
        }

        uchar4 operator + (const uchar4& vec) const
        {
            return uchar4( x + vec.x, y + vec.y, z + vec.z, w + vec.w );
        }

        uchar4 operator - (const uchar4& vec) const
        {
            return uchar4( x - vec.x, y - vec.y, z - vec.z, w - vec.w );
        }

        uchar4 operator * (const unsigned char value) const
        {
            return uchar4( x * value, y * value, z * value, w * value );
        }

        //Per-component multiplication
        uchar4 operator * (const uchar4& vec) const
        {
            return uchar4( x * vec.x, y * vec.y, z * vec.z, w * vec.w );
        }

        uchar4 operator / (const unsigned char value) const
        {
            return uchar4( x / value, y / value, z / value, w / value );
        }

        //Per-component division
        uchar4 operator / (const uchar4& vec) const
        {
            return uchar4( x / vec.x, y / vec.y, z / vec.z, w / vec.w );
        }

        uchar4& operator += (const unsigned char value)
        {
            x += value;
            y += value;
            z += value;
            w += value;
            return *this;
        }

        uchar4& operator -= (const unsigned char value)
        {
            x -= value;
            y -= value;
            z -= value;
            w -= value;
            return *this;
        }

        uchar4& operator *= (const unsigned char value)
        {
            x *= value;
            y *= value;
            z *= value;
            w *= value;
            return *this;
        }

        uchar4& operator /= (const unsigned char value)
        {
            x /= value;
            y /= value;
            z /= value;
            w /= value;
            return *this;
        }

        uchar4& operator += (const uchar4& value)
        {
            x += value.x;
            y += value.y;
            z += value.z;
            w += value.w;
            return *this;
        }

        uchar4& operator -= (const uchar4& value)
        {
            x -= value.x;
            y -= value.y;
            z -= value.z;
            w -= value.w;
            return *this;
        }

        explicit operator float4() const
        {
            return float4(
                (float)x,
                (float)y,
                (float)z,
                (float)w
            );
        }

    };

    uchar4 operator * (const unsigned char value, const uchar4& vec);
}

