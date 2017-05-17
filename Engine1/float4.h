#pragma once

#include <cmath>

namespace Engine1
{
    class float3;

    class float4
    {
        public:
        float x;
        float y;
        float z;
        float w;

        static const float4 ZERO;
        static const float4 ONE;

        float4() {}

        float4( const float4& a ) : x( a.x ), y( a.y ), z( a.z ), w( a.w ) {}

        float4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}

        float4( const float3& a, float w );

        explicit float4( const float v ) : x( v ), y( v ), z( v ), w( v ) {}

        static int size()
        {
            return 4;
        }

        float* getData()
        {
            return &x;
        }

        float4& operator = (const float4& vec)
        {
            x = vec.x;
            y = vec.y;
            z = vec.z;
            w = vec.w;

            return *this;
        }

        bool operator == (const float4& vec) const
        {
            return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
        }

        bool operator != (const float4& vec) const
        {
            return x != vec.x || y != vec.y || z != vec.z || w != vec.w;
        }

        float4 operator - () const
        {
            return float4( -x, -y, -z, -w );
        }

        float4 operator + (const float4& vec) const
        {
            return float4( x + vec.x, y + vec.y, z + vec.z, w + vec.w );
        }

        float4 operator - (const float4& vec) const
        {
            return float4( x - vec.x, y - vec.y, z - vec.z, w - vec.w );
        }

        float4 operator * (const float value) const
        {
            return float4( x * value, y * value, z * value, w * value );
        }

        //Per-component multiplication
        float4 operator * (const float4& vec) const
        {
            return float4( x * vec.x, y * vec.y, z * vec.z, w * vec.w );
        }

        float4 operator / (const float value) const
        {
            float inverse = 1.0f / value;
            return float4( x * inverse, y * inverse, z * inverse, w * inverse );
        }

        //Per-component division
        float4 operator / (const float4& vec) const
        {
            return float4( x / vec.x, y / vec.y, z / vec.z, w / vec.w );
        }

        float4& operator += (const float value)
        {
            x += value;
            y += value;
            z += value;
            w += value;
            return *this;
        }

        float4& operator -= (const float value)
        {
            x -= value;
            y -= value;
            z -= value;
            w -= value;
            return *this;
        }

        float4& operator *= (const float value)
        {
            x *= value;
            y *= value;
            z *= value;
            w *= value;
            return *this;
        }

        float4& operator /= (const float value)
        {
            float inverse = 1.0f / value;
            x *= inverse;
            y *= inverse;
            z *= inverse;
            w *= inverse;
            return *this;
        }

    };

    float4 operator * (const float value, const float4& vec);
    float dot( const float4& vec1, const float4& vec2 );
}

