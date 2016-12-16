#pragma once

#include "float4.h"

namespace Engine1
{
    class uint4
    {
        typedef unsigned int uint4_type;

        public:
        uint4_type x;
        uint4_type y;
        uint4_type z;
        uint4_type w;

        static const uint4 ZERO;

        uint4() {}
        uint4( const uint4& a ) : x( a.x ), y( a.y ), z( a.z ), w( a.w ) {}

        uint4( uint4_type x, uint4_type y, uint4_type z, uint4_type w ) : x( x ), y( y ), z( z ), w( w ) {}

        static int size()
        {
            return 4;
        }

        uint4_type* getData()
        {
            return &x;
        }

        uint4& operator = ( const uint4& vec )
        {
            x = vec.x;
            y = vec.y;
            z = vec.z;
            w = vec.w;

            return *this;
        }

        bool operator == (const uint4& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
        }

        bool operator != (const uint4& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z || w != vec.w;
        }

        uint4 operator + ( const uint4& vec ) const
        {
            return uint4( x + vec.x, y + vec.y, z + vec.z, w + vec.w );
        }

        uint4 operator - ( const uint4& vec ) const
        {
            return uint4( x - vec.x, y - vec.y, z - vec.z, w - vec.w );
        }

        uint4 operator * ( const unsigned char value ) const
        {
            return uint4( x * value, y * value, z * value, w * value );
        }

        //Per-component multiplication
        uint4 operator * ( const uint4& vec ) const
        {
            return uint4( x * vec.x, y * vec.y, z * vec.z, w * vec.w );
        }

        uint4 operator / ( const unsigned char value ) const
        {
            return uint4( x / value, y / value, z / value, w / value );
        }

        //Per-component division
        uint4 operator / ( const uint4& vec ) const
        {
            return uint4( x / vec.x, y / vec.y, z / vec.z, w / vec.w );
        }

        uint4& operator += ( const unsigned char value )
        {
            x += value;
            y += value;
            z += value;
            w += value;
            return *this;
        }

        uint4& operator -= ( const unsigned char value )
        {
            x -= value;
            y -= value;
            z -= value;
            w -= value;
            return *this;
        }

        uint4& operator *= ( const unsigned char value )
        {
            x *= value;
            y *= value;
            z *= value;
            w *= value;
            return *this;
        }

        uint4& operator /= ( const unsigned char value )
        {
            x /= value;
            y /= value;
            z /= value;
            w /= value;
            return *this;
        }

        uint4& operator += ( const uint4& value )
        {
            x += value.x;
            y += value.y;
            z += value.z;
            w += value.w;
            return *this;
        }

        uint4& operator -= ( const uint4& value )
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
}