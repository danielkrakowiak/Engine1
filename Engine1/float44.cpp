#include "float44.h"

#include "MathUtil.h"

using namespace Engine1;

const float44 float44::IDENTITY(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
	);

const float44 float44::ZERO(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
	);

float44 float44::slerp( const float44& from, const float44& to, float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		float44 result;

		result.setOrientation( quat::slerp( quat( from ), quat( to ), factor ) );

		result.setTranslation( from.getTranslation() * ( 1.0f - factor ) + to.getTranslation() * factor );

		result.m14 = 0.0f;
		result.m24 = 0.0f;
		result.m34 = 0.0f;
		result.m44 = 1.0f;

		return result;
	}		   
}

float44 float44::lerp( const float44& from, const float44& to, const float factor )
{
	if ( factor <= 0.0f + MathUtil::epsilonFifty ) {
		return from;
	} else if ( factor >= 1.0f - MathUtil::epsilonFifty ) {
		return to;
	} else {
		const float invFactor = 1.0f - factor;
		return float44(
			from.m11 * invFactor + to.m11 * factor, from.m12 * invFactor + to.m12 * factor, from.m13 * invFactor + to.m13 * factor, from.m14 * invFactor + to.m14 * factor,
			from.m21 * invFactor + to.m21 * factor, from.m22 * invFactor + to.m22 * factor, from.m23 * invFactor + to.m23 * factor, from.m24 * invFactor + to.m24 * factor,
			from.m31 * invFactor + to.m31 * factor, from.m32 * invFactor + to.m32 * factor, from.m33 * invFactor + to.m33 * factor, from.m34 * invFactor + to.m34 * factor,
			from.m41 * invFactor + to.m41 * factor, from.m42 * invFactor + to.m42 * factor, from.m43 * invFactor + to.m43 * factor, from.m44 * invFactor + to.m44 * factor
			);
	}
}

void float44::translate( const float3& translation )
{
    m41 += translation.x;
    m42 += translation.y;
    m43 += translation.z;
}

void float44::rotate( const float3& rotationAngles )
{
    setOrientation( getOrientation() * MathUtil::anglesToRotationMatrix( rotationAngles ) );
}

float4 Engine1::operator * (const float4& a, const float44& b)
{
	return
		float4(
		a.x * b.m11 + a.y * b.m21 + a.z * b.m31 + a.w * b.m41,
		a.x * b.m12 + a.y * b.m22 + a.z * b.m32 + a.w * b.m42,
		a.x * b.m13 + a.y * b.m23 + a.z * b.m33 + a.w * b.m43,
		a.x * b.m14 + a.y * b.m24 + a.z * b.m34 + a.w * b.m44
		);
}

float44 Engine1::operator * (const float value, const float44& b)
{
	return
		float44(
		b.m11 * value, b.m12 * value, b.m13 * value, b.m14 * value,
		b.m21 * value, b.m22 * value, b.m23 * value, b.m24 * value,
		b.m31 * value, b.m32 * value, b.m33 * value, b.m34 * value,
		b.m41 * value, b.m42 * value, b.m43 * value, b.m44 * value
		);
}
