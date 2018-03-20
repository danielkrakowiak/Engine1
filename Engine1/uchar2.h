#pragma once

#undef min
#undef max
#define NOMINMAX

#include "float2.h"

namespace Engine1
{
    class uchar2
    {
        typedef unsigned char uchar2_type;

        public:
        uchar2_type x;
        uchar2_type y;

        static const uchar2 ZERO;

        uchar2() {}
        uchar2( const uchar2& a ) : x( a.x ), y( a.y ) {}

        uchar2( uchar2_type x, uchar2_type y ) : x( x ), y( y ) {}

        static int size()
        {
            return 2;
        }

        uchar2_type* getData()
        {
            return &x;
        }

        uchar2& operator = ( const uchar2& vec )
        {
            x = vec.x;
            y = vec.y;

            return *this;
        }

        bool operator == (const uchar2& vec) const
        {
            return x == vec.x && y == vec.y;
        }

        bool operator != (const uchar2& vec) const
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