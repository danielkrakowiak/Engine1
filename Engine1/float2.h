#pragma once

#include <cmath>

namespace Engine1
{
    class int2;

    class float2
    {

        public:
        float x;
        float y;

        static const float2 ZERO;
        static const float2 ONE;

        float2() {}

        float2( const float2& a ) : x( a.x ), y( a.y ) {}

        float2( float x, float y ) : x( x ), y( y ) {}

        static int size()
        {
            return 2;
        }

        float* getData()
        {
            return &x;
        }

        float2& operator = (const float2& vec)
        {
            x = vec.x;
            y = vec.y;

            return *this;
        }

        bool operator == (const float2& vec) const
        {
            return x == vec.x && y == vec.y;
        }

        bool operator != (const float2& vec) const
        {
            return x != vec.x || y != vec.y;
        }

        float2 operator - () const
        {
            return float2( -x, -y );
        }

        float2 operator + (const float2& vec) const
        {
            return float2( x + vec.x, y + vec.y );
        }

        float2 operator - (const float2& vec) const
        {
            return float2( x - vec.x, y - vec.y );
        }

        float2 operator * (const float value) const
        {
            return float2( x * value, y * value );
        }

        //Per-component multiplication
        float2 operator * (const float2& vec) const
        {
            return float2( x * vec.x, y * vec.y );
        }

        float2 operator / (const float value) const
        {
            float inverse = 1.0f / value;
            return float2( x * inverse, y * inverse );
        }

        //Per-component division
        float2 operator / (const float2& vec) const
        {
            return float2( x / vec.x, y / vec.y );
        }

        float2& operator += (const float value)
        {
            x += value;
            y += value;
            return *this;
        }

        float2& operator -= (const float value)
        {
            x -= value;
            y -= value;
            return *this;
        }

        float2& operator *= (const float value)
        {
            x *= value;
            y *= value;
            return *this;
        }

        float2& operator /= (const float value)
        {
            float inverse = 1.0f / value;
            x *= inverse;
            y *= inverse;
            return *this;
        }

        void normalize()
        {
            float lengthInverse = 1.0f / sqrt( x*x + y*y );
            x *= lengthInverse;
            y *= lengthInverse;
        }

        explicit operator int2() const;
    };

    float2 operator * (const float value, const float2& vec);
    float dot( const float2& vec1, const float2& vec2 );
}

