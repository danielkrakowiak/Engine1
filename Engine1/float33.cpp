#include "float33.h"

#include <cmath>

#include "quat.h"

#include "MathUtil.h"

using namespace Engine1;

const float33 float33::IDENTITY(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
	);

const float33 float33::ZERO(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
	);

float33 float33::slerp( const float33& from, const float33& to, const float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		return float33( quat::slerp( quat( from ), quat( to ), factor ) );
	}
}

float33 float33::lerp( const float33& from, const float33& to, const float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		const float invFactor = 1.0f - factor;
		return float33( 
			from.m11 * invFactor + to.m11 * factor, from.m12 * invFactor + to.m12 * factor, from.m13 * invFactor + to.m13 * factor,
			from.m21 * invFactor + to.m21 * factor, from.m22 * invFactor + to.m22 * factor, from.m23 * invFactor + to.m23 * factor,
			from.m31 * invFactor + to.m31 * factor, from.m32 * invFactor + to.m32 * factor, from.m33 * invFactor + to.m33 * factor
			);
	}
}

void float33::rotate( const float3& rotationAngles )
{
    //#TODO: Could be optimized by using *= operator.

    *this = *this * MathUtil::anglesToRotationMatrix( rotationAngles );
}

float3 Engine1::operator * (const float3& a, const float33& b)
{
	return float3(
		a.x * b.m11 + a.y * b.m21 + a.z * b.m31,
		a.x * b.m12 + a.y * b.m22 + a.z * b.m32,
		a.x * b.m13 + a.y * b.m23 + a.z * b.m33
		);
}

float33 Engine1::operator * (const float value, const float33& b)
{
	return float33(
		b.m11 * value,
		b.m12 * value,
		b.m13 * value,
		b.m21 * value,
		b.m22 * value,
		b.m23 * value,
		b.m31 * value,
		b.m32 * value,
		b.m33 * value
		);
}
