#include "float43.h"

#include "quat.h"

#include "MathUtil.h"

using namespace Engine1;

const float43 float43::IDENTITY(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
	);

const float43 float43::ZERO(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
	);

float43 float43::slerp( const float43& from, const float43& to, float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		float43 result;

		result.setOrientation( quat::slerp( quat( from ), quat( to ), factor ) );

		result.setTranslation( from.getTranslation() * ( 1.0f - factor ) + to.getTranslation() * factor );

		return result;
	}
}

float43 float43::lerp( const float43& from, const float43& to, const float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		const float invFactor = 1.0f - factor;
		return float43(
			from.m11 * invFactor + to.m11 * factor, from.m12 * invFactor + to.m12 * factor, from.m13 * invFactor + to.m13 * factor,
			from.m21 * invFactor + to.m21 * factor, from.m22 * invFactor + to.m22 * factor, from.m23 * invFactor + to.m23 * factor,
			from.m31 * invFactor + to.m31 * factor, from.m32 * invFactor + to.m32 * factor, from.m33 * invFactor + to.m33 * factor,
			from.t1 * invFactor + to.t1 * factor, from.t2 * invFactor + to.t2 * factor, from.t3 * invFactor + to.t3 * factor
			);
	}
}

float3 Engine1::operator * (const float3& a, const float43& b)
{
	return
		float3(
		a.x * b.m11 + a.y * b.m21 + a.z * b.m31 + b.t1,
		a.x * b.m12 + a.y * b.m22 + a.z * b.m32 + b.t2,
		a.x * b.m13 + a.y * b.m23 + a.z * b.m33 + b.t3
		);
}

float43 Engine1::operator * (const float value, const float43& b)
{
	return
		float43(
		b.m11 * value,
		b.m12 * value,
		b.m13 * value,
		b.m21 * value,
		b.m22 * value,
		b.m23 * value,
		b.m31 * value,
		b.m32 * value,
		b.m33 * value,
		b.t1 * value,
		b.t2 * value,
		b.t3 * value
		);
}
