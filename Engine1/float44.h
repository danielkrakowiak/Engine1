#pragma once

#include "float4.h"

#include "float33.h"
#include "float43.h"
#include "quat.h"

#include "PhysX/foundation/PxMat44.h"

//4x4 matrix
//m11 m12 m13 m14
//m21 m22 m23 m24
//m31 m32 m33 m34
//m41 m42 m43 m44
//
//All vectors are assumed to be row vectors

//row-major

namespace Engine1
{
    class float44
    {
        public:
        float m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34,
            m41, m42, m43, m44;

        static const float44 IDENTITY;
        static const float44 ZERO;

        // Uses quaternions to interpolate between two orientations. Translation is interpolated linearly. Other components are set as in identity matrix or left untouched (if copy of an input matrix is returned).
        // For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations.
        static float44 slerp( const float44& from, const float44& to, float factor );

        // Interpolates linearly between two matrices. For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations. 
        // Returned matrix may not be orthogonal (where base vectors are perpendicular to each other and have unit length).
        static float44 lerp( const float44& from, const float44& to, const float factor );

        float44() {}

        //data[row][column]
        float44( float data[ 4 ][ 4 ] ) :
            m11( data[ 0 ][ 0 ] ), m12( data[ 0 ][ 1 ] ), m13( data[ 0 ][ 2 ] ), m14( data[ 0 ][ 3 ] ),
            m21( data[ 1 ][ 0 ] ), m22( data[ 1 ][ 1 ] ), m23( data[ 1 ][ 2 ] ), m24( data[ 1 ][ 3 ] ),
            m31( data[ 2 ][ 0 ] ), m32( data[ 2 ][ 1 ] ), m33( data[ 2 ][ 2 ] ), m34( data[ 2 ][ 3 ] ),
            m41( data[ 3 ][ 0 ] ), m42( data[ 3 ][ 1 ] ), m43( data[ 3 ][ 2 ] ), m44( data[ 3 ][ 3 ] )
        {}

        //data{m11, m12, m13, m21, m22...}
        float44( float data[ 16 ] ) :
            m11( data[ 0 ] ), m12( data[ 1 ] ), m13( data[ 2 ] ), m14( data[ 3 ] ),
            m21( data[ 4 ] ), m22( data[ 5 ] ), m23( data[ 6 ] ), m24( data[ 7 ] ),
            m31( data[ 8 ] ), m32( data[ 9 ] ), m33( data[ 10 ] ), m34( data[ 11 ] ),
            m41( data[ 12 ] ), m42( data[ 13 ] ), m43( data[ 14 ] ), m44( data[ 15 ] )
        {}

        float44( float d11, float d12, float d13, float d14,
                 float d21, float d22, float d23, float d24,
                 float d31, float d32, float d33, float d34,
                 float d41, float d42, float d43, float d44 ) :
                 m11( d11 ), m12( d12 ), m13( d13 ), m14( d14 ),
                 m21( d21 ), m22( d22 ), m23( d23 ), m24( d24 ),
                 m31( d31 ), m32( d32 ), m33( d33 ), m34( d34 ),
                 m41( d41 ), m42( d42 ), m43( d43 ), m44( d44 )
        {}

        float44( const float33& mat ) :
            m11( mat.m11 ), m12( mat.m12 ), m13( mat.m13 ), m14( 0.0f ),
            m21( mat.m21 ), m22( mat.m22 ), m23( mat.m23 ), m24( 0.0f ),
            m31( mat.m31 ), m32( mat.m32 ), m33( mat.m33 ), m34( 0.0f ),
            m41( 0.0f ), m42( 0.0f ), m43( 0.0f ), m44( 1.0f )
        {}

        float44( const float43& mat ) :
            m11( mat.m11 ), m12( mat.m12 ), m13( mat.m13 ), m14( 0.0f ),
            m21( mat.m21 ), m22( mat.m22 ), m23( mat.m23 ), m24( 0.0f ),
            m31( mat.m31 ), m32( mat.m32 ), m33( mat.m33 ), m34( 0.0f ),
            m41( mat.t1 ), m42( mat.t2 ), m43( mat.t3 ), m44( 1.0f )
        {}

        float44( const quat& q ) :
            /*                                  */ m14( 0.0f ),
            /*                                  */ m24( 0.0f ),
            /*                                  */ m34( 0.0f ),
            m41( 0.0f ), m42( 0.0f ), m43( 0.0f ), m44( 1.0f )
        {
            setOrientation( q );
        }

        void identity()
        {
            m11 = 1.0f;	m12 = 0.0f;	m13 = 0.0f; m14 = 0.0f;
            m21 = 0.0f;	m22 = 1.0f;	m23 = 0.0f; m24 = 0.0f;
            m31 = 0.0f;	m32 = 0.0f;	m33 = 1.0f; m34 = 0.0f;
            m41 = 0.0f; m42 = 0.0f;	m43 = 0.0f; m44 = 1.0f;
        }

        void zero()
        {
            m11 = 0.0f;	m12 = 0.0f;	m13 = 0.0f; m14 = 0.0f;
            m21 = 0.0f;	m22 = 0.0f;	m23 = 0.0f; m24 = 0.0f;
            m31 = 0.0f;	m32 = 0.0f;	m33 = 0.0f; m34 = 0.0f;
            m41 = 0.0f; m42 = 0.0f;	m43 = 0.0f; m44 = 0.0f;
        }

        float44 getTranspose() const
        {
            return float44(
                m11, m21, m31, m41,
                m12, m22, m32, m42,
                m13, m23, m33, m43,
                m14, m24, m34, m44
                );
        }

        //returns inverse transformation for a matrix which contains rotation, uniform/non-uniform scaling and translation
        float44 getScaleOrientationTranslationInverse() const
        {

            //Current scale is the length of the coordinate system base vectors
            //ex. for axis x, scale is the length of base vector x - first matrix row.
            //Dividing current base vectors by their length (scale) disables the scaling - but we need to invert the scaling, so we use base vectors' lengths square.
            //We invert the scale already to avoid too many divisions later.
            float3 scaleSquareInv(
                1.0f / float3( m11, m12, m13 ).lengthSquare(),
                1.0f / float3( m21, m22, m23 ).lengthSquare(),
                1.0f / float3( m31, m32, m33 ).lengthSquare()
                );

            //Apply scale inversion + rotation inversion (transpose).
            //Scaling is applied to the original matrix base vectors - which are columns in the new matrix after the transpose.
            float44 invTransf(
                m11 * scaleSquareInv.x, m21 * scaleSquareInv.y, m31 * scaleSquareInv.z, 0.0f,
                m12 * scaleSquareInv.x, m22 * scaleSquareInv.y, m32 * scaleSquareInv.z, 0.0f,
                m13 * scaleSquareInv.x, m23 * scaleSquareInv.y, m33 * scaleSquareInv.z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
                );

            //translation inversion
            invTransf.setTranslation( -getTranslation() * invTransf.getOrientation() ); //TODO: optimize - write direct calculations instead of a method call

            return invTransf;
        }

        float44& operator = (const float44& mat)
        {
            m11 = mat.m11; m12 = mat.m12; m13 = mat.m13; m14 = mat.m14;
            m21 = mat.m21; m22 = mat.m22; m23 = mat.m23; m24 = mat.m24;
            m31 = mat.m31; m32 = mat.m32; m33 = mat.m33; m34 = mat.m34;
            m41 = mat.m41; m42 = mat.m42; m43 = mat.m43; m44 = mat.m44;

            return *this;
        }

        float44 operator + (const float44& mat) const
        {
            return float44(
                m11 + mat.m11, m12 + mat.m12, m13 + mat.m13, m14 + mat.m14,
                m21 + mat.m21, m22 + mat.m22, m23 + mat.m23, m24 + mat.m24,
                m31 + mat.m31, m32 + mat.m32, m33 + mat.m33, m34 + mat.m34,
                m31 + mat.m31, m32 + mat.m32, m33 + mat.m33, m34 + mat.m34
                );
        }

        float44 operator - (const float44& mat) const
        {
            return float44(
                m11 - mat.m11, m12 - mat.m12, m13 - mat.m13, m14 - mat.m14,
                m21 - mat.m21, m22 - mat.m22, m23 - mat.m23, m24 - mat.m24,
                m31 - mat.m31, m32 - mat.m32, m33 - mat.m33, m34 - mat.m34,
                m31 - mat.m31, m32 - mat.m32, m33 - mat.m33, m34 - mat.m34
                );
        }

        float44 operator * (const float44& mat) const
        {
            return float44(
                m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31 + m14 * mat.m41,
                m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32 + m14 * mat.m42,
                m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33 + m14 * mat.m43,
                m11 * mat.m14 + m12 * mat.m24 + m13 * mat.m34 + m14 * mat.m44,

                m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31 + m24 * mat.m41,
                m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32 + m24 * mat.m42,
                m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33 + m24 * mat.m43,
                m21 * mat.m14 + m22 * mat.m24 + m23 * mat.m34 + m24 * mat.m44,

                m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31 + m34 * mat.m41,
                m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32 + m34 * mat.m42,
                m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33 + m34 * mat.m43,
                m31 * mat.m14 + m32 * mat.m24 + m33 * mat.m34 + m34 * mat.m44,

                m41 * mat.m11 + m42 * mat.m21 + m43 * mat.m31 + m44 * mat.m41,
                m41 * mat.m12 + m42 * mat.m22 + m43 * mat.m32 + m44 * mat.m42,
                m41 * mat.m13 + m42 * mat.m23 + m43 * mat.m33 + m44 * mat.m43,
                m41 * mat.m14 + m42 * mat.m24 + m43 * mat.m34 + m44 * mat.m44
                );
        }

        float44 operator * (const float value) const
        {
            return float44(
                m11 * value, m12 * value, m13 * value, m14 * value,
                m21 * value, m22 * value, m23 * value, m24 * value,
                m31 * value, m32 * value, m33 * value, m34 * value,
                m41 * value, m42 * value, m43 * value, m44 * value
                );
        }

        float44& operator *= (const float value)
        {
            m11 *= value; m12 *= value; m13 *= value; m14 *= value;
            m21 *= value; m22 *= value; m23 *= value; m24 *= value;
            m31 *= value; m32 *= value; m33 *= value; m34 *= value;
            m41 *= value; m42 *= value; m43 *= value; m44 *= value;

            return *this;
        }

        float44 operator / (const float value) const
        {
            const float inverse = 1.0f / value;
            return float44(
                m11 * inverse, m12 * inverse, m13 * inverse, m14 * inverse,
                m21 * inverse, m22 * inverse, m23 * inverse, m24 * inverse,
                m31 * inverse, m32 * inverse, m33 * inverse, m34 * inverse,
                m41 * inverse, m42 * inverse, m43 * inverse, m44 * inverse
                );
        }

        float44& operator /= (const float value)
        {
            const float inverse = 1.0f / value;
            m11 *= inverse; m12 *= inverse; m13 *= inverse; m14 *= inverse;
            m21 *= inverse; m22 *= inverse; m23 *= inverse; m24 *= inverse;
            m31 *= inverse; m32 *= inverse; m33 *= inverse; m34 *= inverse;
            m41 *= inverse; m42 *= inverse; m43 *= inverse; m44 *= inverse;

            return *this;
        }

        void normalizeRotationRows()
        {
            modifyRotationRow1().normalize();
            modifyRotationRow2().normalize();
            modifyRotationRow3().normalize();
        }

        void setOrientation( const float33& mat )
        {
            m11 = mat.m11;
            m12 = mat.m12;
            m13 = mat.m13;
            m21 = mat.m21;
            m22 = mat.m22;
            m23 = mat.m23;
            m31 = mat.m31;
            m32 = mat.m32;
            m33 = mat.m33;
        }

        void setOrientation( const quat& q )
        {
            const float	ww = 2.0f * q.w;
            const float	xx = 2.0f * q.x;
            const float	yy = 2.0f * q.y;
            const float	zz = 2.0f * q.z;

            m11 = 1.0f - yy * q.y - zz * q.z;
            m12 = xx * q.y + ww * q.z;
            m13 = xx * q.z - ww * q.y;

            m21 = xx * q.y - ww * q.z;
            m22 = 1.0f - xx * q.x - zz * q.z;
            m23 = yy * q.z + ww * q.x;

            m31 = xx * q.z + ww * q.y;
            m32 = yy * q.z - ww * q.x;
            m33 = 1.0f - xx * q.x - yy * q.y;
        }

        float33 getOrientation() const
        {
            return float33(
                m11, m12, m13,
                m21, m22, m23,
                m31, m32, m33
                );
        }

        float43 getOrientationTranslation() const
        {
            return float43(
                m11, m12, m13,
                m21, m22, m23,
                m31, m32, m33,
                m41, m42, m43
                );
        }

        void setRow1( const float4& vec )
        {
            m11 = vec.x;
            m12 = vec.y;
            m13 = vec.z;
            m14 = vec.w;
        }

        void setRow2( const float4& vec )
        {
            m21 = vec.x;
            m22 = vec.y;
            m23 = vec.z;
            m24 = vec.w;
        }

        void setRow3( const float4& vec )
        {
            m31 = vec.x;
            m32 = vec.y;
            m33 = vec.z;
            m34 = vec.w;
        }

        void setRow4( const float4& vec )
        {
            m41 = vec.x;
            m42 = vec.y;
            m43 = vec.z;
            m44 = vec.w;
        }

        void setRotationRow1( const float3& vec )
        {
            m11 = vec.x;
            m12 = vec.y;
            m13 = vec.z;
        }

        void setRotationRow2( const float3& vec )
        {
            m21 = vec.x;
            m22 = vec.y;
            m23 = vec.z;
        }

        void setRotationRow3( const float3& vec )
        {
            m31 = vec.x;
            m32 = vec.y;
            m33 = vec.z;
        }

        void setTranslation( const float3& vec )
        {
            m41 = vec.x;
            m42 = vec.y;
            m43 = vec.z;
        }

        float4 getRow1() const
        {
            return float4( m11, m12, m13, m14 );
        }

        float4 getRow2() const
        {
            return float4( m21, m22, m23, m24 );
        }

        float4 getRow3() const
        {
            return float4( m31, m32, m33, m34 );
        }

        float4 getRow4() const
        {
            return float4( m41, m42, m43, m44 );
        }

        float4& modifyRow1()
        {
            return *reinterpret_cast<float4*>(&m11);
        }

        float4& modifyRow2()
        {
            return *reinterpret_cast<float4*>(&m21);
        }

        float4& modifyRow3()
        {
            return *reinterpret_cast<float4*>(&m31);
        }

        float4& modifyRow4()
        {
            return *reinterpret_cast<float4*>(&m41);
        }

        float3 getRotationRow1() const
        {
            return float3( m11, m12, m13 );
        }

        float3 getRotationRow2() const
        {
            return float3( m21, m22, m23 );
        }

        float3 getRotationRow3() const
        {
            return float3( m31, m32, m33 );
        }

        float3 getTranslation() const
        {
            return float3( m41, m42, m43 );
        }

        float3& modifyRotationRow1()
        {
            return *reinterpret_cast<float3*>(&m11);
        }

        float3& modifyRotationRow2()
        {
            return *reinterpret_cast<float3*>(&m21);
        }

        float3& modifyRotationRow3()
        {
            return *reinterpret_cast<float3*>(&m31);
        }

        void translate( const float3& translation );
        void rotate( const float3& rotationAngles );

        // Implicit conversion to PhysX matrix type.
        operator physx::PxMat44()
        {
            return reinterpret_cast<physx::PxMat44&>( *this );
        }

        operator physx::PxMat44() const
        {
            return reinterpret_cast<const physx::PxMat44&>( *this );
        }
    };

    float4 operator * (const float4& a, const float44& b);
    float44 operator * (const float value, const float44& b);

    // Conversion from PhysX matrix type.
    float44& toFloat44( physx::PxMat44& matrix );
    const float44& toFloat44( const physx::PxMat44& matrix );
}

