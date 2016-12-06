#include "float2.h"

using namespace Engine1;

const float2 float2::ZERO( 0.0f, 0.0f );
const float2 float2::ONE( 1.0f, 1.0f );

float2 Engine1::operator * (const float value, const float2& vec)
{
	return float2(vec.x * value, vec.y * value);
}

float dot(const float2& vec1, const float2& vec2){
	return (vec1.x * vec2.x) + (vec1.y * vec2.y);
}

float2 max(const float2& a, const float2& b){
	return float2(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y
		);
}

float2 min(const float2& a, const float2& b){
	return float2(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y
		);
}
