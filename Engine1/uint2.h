#pragma once

#include "float2.h"

namespace Engine1
{
    class uint2
    {
        typedef unsigned int uint2_type;

        public:
        uint2_type x;
        uint2_type y;

        static const uint2 ZERO;

        uint2() {}
        uint2( const uint2& a ) : x( a.x ), y( a.y ) {}

        uint2( uint2_type x, uint2_type y ) : x( x ), y( y ) {}

        static int size()
        {
            return 2;
        }

        uint2_type* getData()
        {
            return &x;
        }

        uint2& operator = ( const uint2& vec )
        {
            x = vec.x;
            y = vec.y;

            return *this;
        }

        bool operator == (const uint2& vec) const
        {
            return x == vec.x && y == vec.y;
        }

        bool operator != (const uint2& vec) const
        {
            return x != vec.x || y != vec.y;
        }

        explicit operator float2() const
        {
            return float2(
                (float)x,
                (float)y
                );
        }
    };
}