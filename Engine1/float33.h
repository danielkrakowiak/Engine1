#pragma once

#include "float3.h"
#include "quat.h"


//3x3 matrix
//m11 m12 m13
//m21 m22 m23
//m31 m32 m33
//
//All vectors are assumed to be row vectors

//row-major

namespace Engine1
{
    class quat;

    class float33
    {
        public:
        float m11, m12, m13,
            m21, m22, m23,
            m31, m32, m33;

        static const float33 IDENTITY;
        static const float33 ZERO;

        // Uses quaternions to interpolate between two orientations. For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations. 
        static float33 slerp( const float33& from, const float33& to, const float factor );

        // Interpolates linearly between two orientations. For factor <= 0 or >= 1 it returns one of the input matrices without performing calculations. 
        // Returned matrix may not be orthogonal (where base vectors are perpendicular to each other and have unit length).
        static float33 lerp( const float33& from, const float33& to, const float factor );

        float33() {}

        //data[row][column]
        float33( float data[ 3 ][ 3 ] ) : m11( data[ 0 ][ 0 ] ), m12( data[ 0 ][ 1 ] ), m13( data[ 0 ][ 2 ] ), m21( data[ 1 ][ 0 ] ), m22( data[ 1 ][ 1 ] ), m23( data[ 1 ][ 2 ] ),
            m31( data[ 2 ][ 0 ] ), m32( data[ 2 ][ 1 ] ), m33( data[ 2 ][ 2 ] )
        {}

        //data{m11, m12, m13, m21, m22...}
        float33( float data[ 9 ] ) : m11( data[ 0 ] ), m12( data[ 1 ] ), m13( data[ 2 ] ), m21( data[ 3 ] ), m22( data[ 4 ] ), m23( data[ 5 ] ), m31( data[ 6 ] ), m32( data[ 7 ] ), m33( data[ 8 ] ) {}

        float33( float d11, float d12, float d13, float d21, float d22, float d23, float d31, float d32, float d33 ) : m11( d11 ), m12( d12 ), m13( d13 ),
            m21( d21 ), m22( d22 ), m23( d23 ), m31( d31 ), m32( d32 ), m33( d33 )
        {}

        float33( float3 row1, float3 row2, float3 row3 ) :
            m11( row1.x ), m12( row1.y ), m13( row1.z ),
            m21( row2.x ), m22( row2.y ), m23( row2.z ),
            m31( row3.x ), m32( row3.y ), m33( row3.z )
        {}

        float33( const quat& q )
        {
            set( q );
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
        }

        float33 getTranspose() const
        {
            return float33(
                m11, m21, m31,
                m12, m22, m32,
                m13, m23, m33
                );
        }

        float33& operator = (const float33& mat)
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

            return *this;
        }

        float33 operator + (const float33& mat) const
        {
            return float33(
                m11 + mat.m11,
                m12 + mat.m12,
                m13 + mat.m13,
                m21 + mat.m21,
                m22 + mat.m22,
                m23 + mat.m23,
                m31 + mat.m31,
                m32 + mat.m32,
                m33 + mat.m33
                );
        }

        float33 operator - (const float33& mat) const
        {
            return float33(
                m11 - mat.m11,
                m12 - mat.m12,
                m13 - mat.m13,
                m21 - mat.m21,
                m22 - mat.m22,
                m23 - mat.m23,
                m31 - mat.m31,
                m32 - mat.m32,
                m33 - mat.m33
                );
        }

        float33 operator * (const float33& mat) const
        {
            return float33(
                m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31,
                m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32,
                m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33,
                m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31,
                m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32,
                m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33,
                m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31,
                m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32,
                m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33
                );
        }

        float33 operator * (const float value) const
        {
            return float33(
                m11 * value,
                m12 * value,
                m13 * value,
                m21 * value,
                m22 * value,
                m23 * value,
                m31 * value,
                m32 * value,
                m33 * value
                );
        }

        float33& operator *= (const float value)
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

            return *this;
        }

        float33 operator / (const float value) const
        {
            const float inverse = 1.0f / value;
            return float33(
                m11 * inverse,
                m12 * inverse,
                m13 * inverse,
                m21 * inverse,
                m22 * inverse,
                m23 * inverse,
                m31 * inverse,
                m32 * inverse,
                m33 * inverse
                );
        }

        float33& operator /= (const float value)
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

            return *this;
        }

        void scale( const float3& vec )
        {
            m11 *= vec.x; m12 *= vec.x; m13 *= vec.x;
            m21 *= vec.y; m22 *= vec.y; m23 *= vec.y;
            m31 *= vec.z; m32 *= vec.z; m33 *= vec.z;
        }

        void normalizeRows()
        {
            modifyRow1().normalize();
            modifyRow2().normalize();
            modifyRow3().normalize();
        }

        // normal version
        void set( const quat& q )
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

        // transposed version
        /*void set( const quat& q )
        {
        const float	ww = 2.0f * q.w;
        const float	xx = 2.0f * q.x;
        const float	yy = 2.0f * q.y;
        const float	zz = 2.0f * q.z;

        m11 = 1.0f - yy * q.y - zz * q.z;
        m21 = xx * q.y + ww * q.z;
        m31 = xx * q.z - ww * q.y;

        m12 = xx * q.y - ww * q.z;
        m22 = 1.0f - xx * q.x - zz * q.z;
        m32 = yy * q.z + ww * q.x;

        m13 = xx * q.z + ww * q.y;
        m23 = yy * q.z - ww * q.x;
        m33 = 1.0f - xx * q.x - yy * q.y;
        }*/

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

        void setColumn1( const float3& vec )
        {
            m11 = vec.x;
            m21 = vec.y;
            m31 = vec.z;
        }

        void setColumn2( const float3& vec )
        {
            m11 = vec.x;
            m21 = vec.y;
            m31 = vec.z;
        }

        void setColumn3( const float3& vec )
        {
            m11 = vec.x;
            m21 = vec.y;
            m31 = vec.z;
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

        float3 getColumn1() const
        {
            return float3( m11, m21, m31 );
        }

        float3 getColumn2() const
        {
            return float3( m12, m22, m32 );
        }

        float3 getColumn3() const
        {
            return float3( m13, m23, m33 );
        }
    };

    float3 operator * (const float3& a, const float33& b);
    float33 operator * (const float value, const float33& b);
}