#include "float3.h"

#include "int3.h"
#include "float4.h"
#include "MathUtil.h"

using namespace Engine1;

const float3 float3::ZERO( 0.0f, 0.0f, 0.0f );
const float3 float3::ONE( 1.0f, 1.0f, 1.0f );

float3::float3(const float4& a) : x(a.x), y(a.y), z(a.z) {}

float3 Engine1::operator * ( const float value, const float3& vec ) 
{
	return float3(vec.x * value, vec.y * value, vec.z * value);
}

void float3::rotate( const float3& rotationAngles )
{
    *this = *this * MathUtil::anglesToRotationMatrix( rotationAngles );
}

float Engine1::dot( const float3& vec1, const float3& vec2 ) 
{
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

float3 Engine1::cross( const float3& vec1, const float3& vec2 ) 
{
	return float3(
		(vec1.y * vec2.z) - (vec1.z * vec2.y),
		(vec1.z * vec2.x) - (vec1.x * vec2.z),
		(vec1.x * vec2.y) - (vec1.y * vec2.x)
		);
}

float3 Engine1::max( const float3& a, const float3& b ) 
{
	return float3(
		fmax(a.x, b.x),
		fmax(a.y, b.y),
		fmax(a.z, b.z)
		);
}

float3 Engine1::min( const float3& a, const float3& b ) 
{
	return float3(
		fmin(a.x, b.x),
		fmin(a.y, b.y),
		fmin(a.z, b.z)
		);
}

float3::operator int3() const
{
    return int3(
        (int)x,
        (int)y,
        (int)z
    );
}
