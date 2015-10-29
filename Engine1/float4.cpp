#include "float4.h"

#include "float3.h"

float4::float4(const float3& a, float w) : x(a.x), y(a.y), z(a.z), w(w) {}

float4 operator * (const float value, const float4& vec) {
	return float4(vec.x * value, vec.y * value, vec.z * value, vec.w * value);
}

float dot(const float4& vec1, const float4& vec2){
	return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z) + (vec1.w * vec2.w);
}

float4 max(const float4& a, const float4& b){
	return float4(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y,
		a.z > b.z ? a.z : b.z,
		a.w > b.w ? a.w : b.w
		);
}

float4 min(const float4& a, const float4& b){
	return float4(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y,
		a.z < b.z ? a.z : b.z,
		a.w < b.w ? a.w : b.w
		);
}
