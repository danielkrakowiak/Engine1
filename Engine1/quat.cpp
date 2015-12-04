#include "quat.h"

#include <math.h>
#include <assert.h>

#include "float3.h"
#include "MathUtil.h"

using namespace Engine1;

// TODO Optimizations:
// 1) Faster version of slerp could be made if it's known that quaternions are of unit length and different, factor within range and not equal to 0 or 1. Checks could be removed.

float quat::dot( const quat &a, const quat &b )
{
	return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

quat quat::slerp( const quat &q0, const quat &q1, float t )
{
	// Check for out-of range parameter.
	if ( t <= 0.0f + MathUtil::epsilonFifty ) return q0;
	if ( t >= 1.0f - MathUtil::epsilonFifty ) return q1;

	// Compute "cosine of angle between quaternions" using dot product
	float cosOmega = dot( q0, q1 );

	// If negative dot, use -q1. Two quaternions q and -q represent the same rotation, 
	// but may produce different slerp. We chose q or -q to rotate using the acute angle.
	float q1w = q1.w;
	float q1x = q1.x;
	float q1y = q1.y;
	float q1z = q1.z;
	if ( cosOmega < 0.0f ) {
		q1w = -q1w;
		q1x = -q1x;
		q1y = -q1y;
		q1z = -q1z;
		cosOmega = -cosOmega;
	}

	// We should have two unit quaternions, so dot should be <= 1.0.
	assert( cosOmega < 1.1f );

	// Compute interpolation fraction, checking for quaternions
	// which are almost exactly the same.
	float k0, k1;
	if ( cosOmega > 0.9999f ) {

		// Very close - just use linear interpolation,
		// which will protect against a divide by zero.
		k0 = 1.0f - t;
		k1 = t;

	} else {

		// Compute the sin of the angle using the
		// trig identity sin^2(omega) + cos^2(omega) = 1.
		float sinOmega = sqrt( 1.0f - cosOmega*cosOmega );

		// Compute the angle from its sin and cosine.
		float omega = atan2( sinOmega, cosOmega );

		float oneOverSinOmega = 1.0f / sinOmega;

		// Compute interpolation parameters.
		k0 = sin( ( 1.0f - t ) * omega ) * oneOverSinOmega;
		k1 = sin( t * omega ) * oneOverSinOmega;
	}

	// Interpolate.
	quat  result;
	result.x = k0 * q0.x + k1 * q1x;
	result.y = k0 * q0.y + k1 * q1y;
	result.z = k0 * q0.z + k1 * q1z;
	result.w = k0 * q0.w + k1 * q1w;

	return result;
}

quat quat::conjugate( const quat &q )
{
	quat result;

	// Same rotation amount.
	result.w = q.w;

	// Opposite axis of rotation.
	result.x = -q.x;
	result.y = -q.y;
	result.z = -q.z;

	return result;
}

quat quat::pow( const quat &q, float exponent )
{
	// Check for the case of an identity quaternion.
	// This will protect against divide by zero.
	if ( fabs( q.w ) > .9999f ) {
		return q;
	}

	// Extract the half angle alpha (alpha = theta/2).
	float alpha = acos( q.w );

	// Compute new alpha value.
	float newAlpha = alpha * exponent;

	// Compute new w value.
	quat result;
	result.w = cos( newAlpha );

	// Compute new xyz values.
	float mult = sin( newAlpha ) / sin( alpha );
	result.x = q.x * mult;
	result.y = q.y * mult;
	result.z = q.z * mult;

	return result;
}

void quat::normalize()
{
	float mag = (float) sqrt( w*w + x*x + y*y + z*z );

	// Check for bogus length, to protect against divide by zero.
	if ( mag > 0.0f ) {

		// Normalize it.
		float oneOverMag = 1.0f / mag;
		w *= oneOverMag;
		x *= oneOverMag;
		y *= oneOverMag;
		z *= oneOverMag;
	} else {
		// Corrupted quaternion!
		assert( false );

		identity();
	}
}

quat quat::operator * ( const quat& q ) const
{
	quat result;

	result.w = w * q.w - x * q.x - y * q.y - z * q.z;
	result.x = w * q.x + x * q.w + z * q.y - y * q.z;
	result.y = w * q.y + y * q.w + x * q.z - z * q.x;
	result.z = w * q.z + z * q.w + y * q.x - x * q.y;

	return result;
}

quat& quat::operator *= ( const quat& q )
{
	*this = *this * q;

	return *this;
}

// normal version
void quat::set( const float33& m )
{
	// Determine which f w, x, y, z has the largest absolute value.
	const float fourWSquaredMinus1 = m.m11 + m.m22 + m.m33;
	const float fourXSquaredMinus1 = m.m11 - m.m22 - m.m33;
	const float fourYSquaredMinus1 = m.m22 - m.m11 - m.m33;
	const float fourZSquaredMinus1 = m.m33 - m.m11 - m.m22;

	int biggestIndex = 0;
	float fourBiggestSquareMinus1 = fourWSquaredMinus1;
	if ( fourXSquaredMinus1 > fourBiggestSquareMinus1 ) {
		fourBiggestSquareMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if ( fourYSquaredMinus1 > fourBiggestSquareMinus1 ) {
		fourBiggestSquareMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if ( fourZSquaredMinus1 > fourBiggestSquareMinus1 ) {
		fourBiggestSquareMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	// Perform square root and division.
	const float biggestVal = sqrt( fourBiggestSquareMinus1 + 1.0f ) * 0.5f;
	const float mult = 0.25f / biggestVal;

	// Compute quaternion values.
	switch ( biggestIndex ) {
		case 0:
			w = biggestVal;
			x = ( m.m23 - m.m32 ) * mult;
			y = ( m.m31 - m.m13 ) * mult;
			z = ( m.m12 - m.m21 ) * mult;
			break;
		case 1:
			w = ( m.m23 - m.m32 ) * mult;
			x = biggestVal;
			y = ( m.m12 + m.m21 ) * mult;
			z = ( m.m31 + m.m13 ) * mult;
			break;
		case 2:
			w = ( m.m31 - m.m13 ) * mult;
			x = ( m.m12 + m.m21 ) * mult;
			y = biggestVal;
			z = ( m.m23 + m.m32 ) * mult;
			break;
		case 3:
			w = ( m.m12 - m.m21 ) * mult;
			x = ( m.m31 + m.m13 ) * mult;
			y = ( m.m23 + m.m32 ) * mult;
			z = biggestVal;
			break;
	}
}

// transposed version
//void quat::set( const float33& m )
//{
//	// Determine which f w, x, y, z has the largest absolute value.
//	const float fourWSquaredMinus1 = m.m11 + m.m22 + m.m33;
//	const float fourXSquaredMinus1 = m.m11 - m.m22 - m.m33;
//	const float fourYSquaredMinus1 = m.m22 - m.m11 - m.m33;
//	const float fourZSquaredMinus1 = m.m33 - m.m11 - m.m22;
//
//	int biggestIndex = 0;
//	float fourBiggestSquareMinus1 = fourWSquaredMinus1;
//	if ( fourXSquaredMinus1 > fourBiggestSquareMinus1 ) {
//		fourBiggestSquareMinus1 = fourXSquaredMinus1;
//		biggestIndex = 1;
//	}
//	if ( fourYSquaredMinus1 > fourBiggestSquareMinus1 ) {
//		fourBiggestSquareMinus1 = fourYSquaredMinus1;
//		biggestIndex = 2;
//	}
//	if ( fourZSquaredMinus1 > fourBiggestSquareMinus1 ) {
//		fourBiggestSquareMinus1 = fourZSquaredMinus1;
//		biggestIndex = 3;
//	}
//
//	// Perform square root and division.
//	const float biggestVal = sqrt( fourBiggestSquareMinus1 + 1.0f ) * 0.5f;
//	const float mult = 0.25f / biggestVal;
//
//	// Compute quaternion values.
//	switch ( biggestIndex ) {
//		case 0:
//			w = biggestVal;
//			x = ( m.m32 - m.m23 ) * mult;
//			y = ( m.m13 - m.m31 ) * mult;
//			z = ( m.m21 - m.m12 ) * mult;
//			break;
//		case 1:
//			w = ( m.m32 - m.m23 ) * mult;
//			x = biggestVal;
//			y = ( m.m21 + m.m12 ) * mult;
//			z = ( m.m13 + m.m31 ) * mult;
//			break;
//		case 2:
//			w = ( m.m13 - m.m31 ) * mult;
//			x = ( m.m21 + m.m12 ) * mult;
//			y = biggestVal;
//			z = ( m.m32 + m.m23 ) * mult;
//			break;
//		case 3:
//			w = ( m.m21 - m.m12 ) * mult;
//			x = ( m.m13 + m.m31 ) * mult;
//			y = ( m.m32 + m.m23 ) * mult;
//			z = biggestVal;
//			break;
//	}
//}

void quat::set( const float43& m )
{
	set( m.getOrientation() );
}

void quat::set( const float44& m )
{
	set( m.getOrientation( ) );
}

float quat::getRotationAngle() const
{
	// Compute the half angle. w = cos(theta / 2).
	float thetaOver2;
	if ( w <= -1.0f )
		thetaOver2 = MathUtil::pi;
	else if ( w >= 1.0f )
		thetaOver2 = 0.0f;
	else
		thetaOver2 = acos( w );

	return thetaOver2 * 2.0f;
}

float3 quat::getRotationAxis() const
{
	// Compute sin^2(theta/2). w = cos(theta/2), sin^2(x) + cos^2(x) = 1.
	float sinThetaOver2Sq = 1.0f - w*w;

	// Protect against numerical imprecision.
	if ( sinThetaOver2Sq <= 0.0f ) {

		// Identity quaternion, or numerical imprecision. Just
		// return any valid vector, since it doesn't matter.
		return float3( 1.0f, 0.0f, 0.0f );
	}

	// Compute 1 / sin(theta/2).
	float oneOverSinThetaOver2 = 1.0f / sqrt( sinThetaOver2Sq );

	return float3(
		x * oneOverSinThetaOver2,
		y * oneOverSinThetaOver2,
		z * oneOverSinThetaOver2
		);
}


