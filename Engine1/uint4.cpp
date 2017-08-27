#include "uint4.h"

#include "float4.h"

using namespace Engine1;

const uint4 uint4::ZERO( 0, 0, 0, 0 );

uint4::operator float4() const
{
    return float4(
        (float)x,
        (float)y,
        (float)z,
        (float)w
    );
}