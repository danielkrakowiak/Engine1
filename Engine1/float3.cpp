#include "float3.h"

#include "float4.h"

using namespace Engine1;

const float3 float3::ZERO( 0.0f, 0.0f, 0.0f );

float3::float3(const float4& a) : x(a.x), y(a.y), z(a.z) {}

float3 Engine1::operator * ( const float value, const float3& vec ) {
	return float3(vec.x * value, vec.y * value, vec.z * value);
}

float float3::dot( const float3& vec1, const float3& vec2 ) {
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
}

float3 float3::cross( const float3& vec1, const float3& vec2 ) {
	return float3(
		(vec1.y * vec2.z) - (vec1.z * vec2.y),
		(vec1.z * vec2.x) - (vec1.x * vec2.z),
		(vec1.x * vec2.y) - (vec1.y * vec2.x)
		);
}

float3 float3::max( const float3& a, const float3& b ) {
	return float3(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y,
		a.z > b.z ? a.z : b.z
		);
}

float3 float3::min( const float3& a, const float3& b ) {
	return float3(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y,
		a.z < b.z ? a.z : b.z
		);
}
