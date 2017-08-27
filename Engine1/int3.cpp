#include "int3.h"

#include "float3.h"
#include "int2.h"

using namespace Engine1;

const int3 int3::ZERO( 0, 0, 0 );

int3::operator float3() const
{
    return float3(
        (float)x,
        (float)y,
        (float)z
    );
}

int3::operator int2() const
{
    return int2(
        x,
        y
    );
}