#include "uint4.h"

#include "float4.h"
#include "uint3.h"

using namespace Engine1;

const uint4 uint4::ZERO( 0, 0, 0, 0 );

uint4::uint4( const uint3& xyz, const unsigned int w ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( w ) {}

uint4::operator float4() const
{
    return float4(
        (float)x,
        (float)y,
        (float)z,
        (float)w
    );
}