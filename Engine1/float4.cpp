#include "float4.h"

#include "float3.h"
#include "uchar2.h"
#include "uchar4.h"
#include "uint2.h"
#include "uint3.h"
#include "uint4.h"

using namespace Engine1;

const float4 float4::ZERO( 0.0f, 0.0f, 0.0f, 0.0f );
const float4 float4::ONE( 1.0f, 1.0f, 1.0f, 1.0f );

float4::float4(const float3& a, float w) : x(a.x), y(a.y), z(a.z), w(w) {}

float4 Engine1::operator * (const float value, const float4& vec)
{
	return float4(vec.x * value, vec.y * value, vec.z * value, vec.w * value);
}

float4::operator unsigned char() const
{
    return (unsigned char)x;
}

float4::operator uchar2() const
{
    return uchar2(
        (unsigned char)x,
        (unsigned char)y
    );
}

float4::operator uchar4() const
{
    return uchar4(
        (unsigned char)x,
        (unsigned char)y,
        (unsigned char)z,
        (unsigned char)w
    );
}

float4::operator unsigned int() const
{
    return (unsigned int)x;
}

float4::operator uint2() const
{
    return uint2(
        (unsigned int)x,
        (unsigned int)y
    );
}

float4::operator uint3() const
{
    return uint3(
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z
    );
}

float4::operator uint4() const
{
    return uint4(
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z,
        (unsigned int)w
    );
}

float dot(const float4& vec1, const float4& vec2)
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z) + (vec1.w * vec2.w);
}

float4 max(const float4& a, const float4& b)
{
	return float4(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y,
		a.z > b.z ? a.z : b.z,
		a.w > b.w ? a.w : b.w
	);
}

float4 min(const float4& a, const float4& b)
{
	return float4(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y,
		a.z < b.z ? a.z : b.z,
		a.w < b.w ? a.w : b.w
	);
}
