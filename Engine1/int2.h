#pragma once

#undef min
#undef max
#define NOMINMAX

#include "float2.h"

namespace Engine1
{
    class int2
    {
        typedef int int2_type;

        public:
        int2_type x;
        int2_type y;

        static const int2 ZERO;
        static const int2 ONE;

        int2() {}
        int2( const int2& a ) : x( a.x ), y( a.y ) {}

        int2( int2_type x, int2_type y ) : x( x ), y( y ) {}

        static int size()
        {
            return 2;
        }

        int2_type* getData()
        {
            return &x;
        }

        bool operator == (const int2& vec) const
        {
            return x == vec.x && y == vec.y;
        }

        bool operator != (const int2& vec) const
        {
            return x != vec.x || y != vec.y;
        }

        int2 operator - () const
        {
            return int2( -x, -y );
        }

        int2 operator + (const int2& vec) const
        {
            return int2( x + vec.x, y + vec.y );
        }

        int2 operator - (const int2& vec) const
        {
            return int2( x - vec.x, y - vec.y );
        }

        int2 operator * (const int value) const
        {
            return int2( x * value, y * value );
        }

        //Per-component multiplication
        int2 operator * (const int2& vec) const
        {
            return int2( x * vec.x, y * vec.y );
        }

        int2 operator / (const int value) const
        {
            return int2( x / value, y / value );
        }

        //Per-component division
        int2 operator / (const int2& vec) const
        {
            return int2( x / vec.x, y / vec.y );
        }

        int2& operator += (const int2& value)
        {
            x += value.x;
            y += value.y;
            return *this;
        }

        int2& operator -= (const int2& value)
        {
            x -= value.x;
            y -= value.y;
            return *this;
        }

        int2& operator *= (const int value)
        {
            x *= value;
            y *= value;
            return *this;
        }

        int2& operator /= (const int value)
        {
            x /= value;
            y /= value;
            return *this;
        }

        explicit operator float2() const
        {
            return float2(
                (float)x,
                (float)y
                );
        }

        std::string toString() const
        {
            std::string text = "(";

            text += std::to_string(x);
            text += ", ";
            text += std::to_string(y);
            text += ")";

            return std::move( text );
        }
    };
}



