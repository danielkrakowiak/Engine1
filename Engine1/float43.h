#pragma once

#include "float33.h"
#include "quat.h"


//4x3 matrix
//m11 m12 m13 (0)
//m21 m22 m23 (0)
//m31 m32 m33 (0)
//t1  t2  t3  (1)
//
//All vectors are assumed to be row vectors

//row-major

namespace Engine1
{
    class float43
    {
        public:
        float m11, m12, m13,
            m21, m22, m23,
            m31, m32, m33,
            t1, t2, t3;

        static const float43 IDENTITY;
        static const float43 ZERO;

        // Uses quaternions to interpolate between two orientations. Translation is interpolated linearly. For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations. 
        static float43 slerp( const float43& from, const float43& to, float factor );

        // Interpolates linearly between two matrices. For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations. 
        // Returned matrix may not be orthogonal (where base vectors are perpendicular to each other and have unit length).
        static float43 lerp( const float43& from, const float43& to, const float factor );

        float43() {}

        //data[row][column]
        float43( float data[ 4 ][ 3 ] ) : m11( data[ 0 ][ 0 ] ), m12( data[ 0 ][ 1 ] ), m13( data[ 0 ][ 2 ] ), m21( data[ 1 ][ 0 ] ), m22( data[ 1 ][ 1 ] ), m23( data[ 1 ][ 2 ] ),
            m31( data[ 2 ][ 0 ] ), m32( data[ 2 ][ 1 ] ), m33( data[ 2 ][ 2 ] ), t1( data[ 3 ][ 0 ] ), t2( data[ 3 ][ 1 ] ), t3( data[ 3 ][ 2 ] )
        {}

        //data{m11, m12, m13, m21, m22..., t1, t2, t3}
        float43( float data[ 12 ] ) : m11( data[ 0 ] ), m12( data[ 1 ] ), m13( data[ 2 ] ), m21( data[ 3 ] ), m22( data[ 4 ] ), m23( data[ 5 ] ), m31( data[ 6 ] ), m32( data[ 7 ] ), m33( data[ 8 ] ), t1( data[ 9 ] ), t2( data[ 10 ] ), t3( data[ 11 ] ) {}

        float43( float d11, float d12, float d13, float d21, float d22, float d23, float d31, float d32, float d33, float dt1, float dt2, float dt3 ) : m11( d11 ), m12( d12 ), m13( d13 ),
            m21( d21 ), m22( d22 ), m23( d23 ), m31( d31 ), m32( d32 ), m33( d33 ), t1( dt1 ), t2( dt2 ), t3( dt3 )
        {}

        float43( const quat& q )
        {
            setOrientation( q );

            t1 = 0.0f;
            t2 = 0.0f;
            t3 = 0.0f;
        }

        void identity()
        {
            m11 = 1.0f;
            m12 = 0.0f;
            m13 = 0.0f;
            m21 = 0.0f;
            m22 = 1.0f;
            m23 = 0.0f;
            m31 = 0.0f;
            m32 = 0.0f;
            m33 = 1.0f;
            t1 = 0.0f;
            t2 = 0.0f;
            t3 = 0.0f;
        }

        void zero()
        {
            m11 = 0.0f;
            m12 = 0.0f;
            m13 = 0.0f;
            m21 = 0.0f;
            m22 = 0.0f;
            m23 = 0.0f;
            m31 = 0.0f;
            m32 = 0.0f;
            m33 = 0.0f;
            t1 = 0.0f;
            t2 = 0.0f;
            t3 = 0.0f;
        }

        //returns inverse transformation for a matrix which contains rotation, uniform/non-uniform scaling and translation
        float43 getScaleOrientationTranslationInverse() const
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
            float43 invTransf(
                m11 * scaleSquareInv.x, m21 * scaleSquareInv.y, m31 * scaleSquareInv.z,
                m12 * scaleSquareInv.x, m22 * scaleSquareInv.y, m32 * scaleSquareInv.z,
                m13 * scaleSquareInv.x, m23 * scaleSquareInv.y, m33 * scaleSquareInv.z,
                0.0f, 0.0f, 0.0f
                );

            //translation inversion
            invTransf.setTranslation( -getTranslation() * invTransf.getOrientation() ); //TODO: optimize - write direct calculations instead of a method call

            return invTransf;
        }

        float43& operator = (const float43& mat)
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
            t1 = mat.t1;
            t2 = mat.t2;
            t3 = mat.t3;

            return *this;
        }

        float43 operator + (const float43& mat) const
        {
            return float43(
                m11 + mat.m11,
                m12 + mat.m12,
                m13 + mat.m13,
                m21 + mat.m21,
                m22 + mat.m22,
                m23 + mat.m23,
                m31 + mat.m31,
                m32 + mat.m32,
                m33 + mat.m33,
                t1 + mat.t1,
                t2 + mat.t2,
                t3 + mat.t3
                );
        }

        float43 operator - (const float43& mat) const
        {
            return float43(
                m11 - mat.m11,
                m12 - mat.m12,
                m13 - mat.m13,
                m21 - mat.m21,
                m22 - mat.m22,
                m23 - mat.m23,
                m31 - mat.m31,
                m32 - mat.m32,
                m33 - mat.m33,
                t1 - mat.t1,
                t2 - mat.t2,
                t3 - mat.t3
                );
        }

        float43 operator * (const float43& mat) const
        {
            return float43(
                m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31,
                m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32,
                m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33,
                m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31,
                m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32,
                m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33,
                m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31,
                m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32,
                m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33,
                t1  * mat.m11 + t2  * mat.m21 + t3  * mat.m31 + mat.t1,
                t1  * mat.m12 + t2  * mat.m22 + t3  * mat.m32 + mat.t2,
                t1  * mat.m13 + t2  * mat.m23 + t3  * mat.m33 + mat.t3
                );
        }

        float43 operator * (const float value) const
        {
            return float43(
                m11 * value,
                m12 * value,
                m13 * value,
                m21 * value,
                m22 * value,
                m23 * value,
                m31 * value,
                m32 * value,
                m33 * value,
                t1  * value,
                t2  * value,
                t3  * value
                );
        }

        float43& operator *= (const float value)
        {
            m11 *= value;
            m12 *= value;
            m13 *= value;
            m21 *= value;
            m22 *= value;
            m23 *= value;
            m31 *= value;
            m32 *= value;
            m33 *= value;
            t1 *= value;
            t2 *= value;
            t3 *= value;

            return *this;
        }

        float43 operator / (const float value) const
        {
            const float inverse = 1.0f / value;
            return float43(
                m11 * inverse,
                m12 * inverse,
                m13 * inverse,
                m21 * inverse,
                m22 * inverse,
                m23 * inverse,
                m31 * inverse,
                m32 * inverse,
                m33 * inverse,
                t1  * inverse,
                t2  * inverse,
                t3  * inverse
                );
        }

        float43& operator /= (const float value)
        {
            const float inverse = 1.0f / value;
            m11 *= inverse;
            m12 *= inverse;
            m13 *= inverse;
            m21 *= inverse;
            m22 *= inverse;
            m23 *= inverse;
            m31 *= inverse;
            m32 *= inverse;
            m33 *= inverse;
            t1 *= inverse;
            t2 *= inverse;
            t3 *= inverse;

            return *this;
        }

        /*void resetScale( ) {
            float xScale = float3( m11, m12, m13 ).length();
            float yScale = float3( m21, m22, m23 ).length( );
            float zScale = float3( m31, m32, m33 ).length( );

            m11 /= xScale;
            m12 /= xScale;
            m13 /= xScale;
            m21 /= yScale;
            m22 /= yScale;
            m23 /= yScale;
            m31 /= zScale;
            m32 /= zScale;
            m33 /= zScale;
            }*/

        void scale( const float3& vec )
        {
            m11 *= vec.x; m12 *= vec.x; m13 *= vec.x;
            m21 *= vec.y; m22 *= vec.y; m23 *= vec.y;
            m31 *= vec.z; m32 *= vec.z; m33 *= vec.z;
        }

        void normalizeRotationRows()
        {
            modifyRow1().normalize();
            modifyRow2().normalize();
            modifyRow3().normalize();
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

        void setRow1( const float3& vec )
        {
            m11 = vec.x;
            m12 = vec.y;
            m13 = vec.z;
        }

        void setRow2( const float3& vec )
        {
            m21 = vec.x;
            m22 = vec.y;
            m23 = vec.z;
        }

        void setRow3( const float3& vec )
        {
            m31 = vec.x;
            m32 = vec.y;
            m33 = vec.z;
        }

        void setTranslation( const float3& vec )
        {
            t1 = vec.x;
            t2 = vec.y;
            t3 = vec.z;
        }

        void setTranslation( float d1, float d2, float d3 )
        {
            t1 = d1;
            t2 = d2;
            t3 = d3;
        }

        float3 getRow1() const
        {
            return float3( m11, m12, m13 );
        }

        float3 getRow2() const
        {
            return float3( m21, m22, m23 );
        }

        float3 getRow3() const
        {
            return float3( m31, m32, m33 );
        }

        float3 getTranslation() const
        {
            return float3( t1, t2, t3 );
        }

        float3& modifyRow1()
        {
            return *reinterpret_cast<float3*>(&m11);
        }

        float3& modifyRow2()
        {
            return *reinterpret_cast<float3*>(&m21);
        }

        float3& modifyRow3()
        {
            return *reinterpret_cast<float3*>(&m31);
        }

        void translate( const float3& translation );
        void rotate( const float3& rotationAngles );
    };

    float3 operator * (const float3& a, const float43& b);
    float43 operator * (const float value, const float43& b);
}

