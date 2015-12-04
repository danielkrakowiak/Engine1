#include "stdafx.h"
#include "CppUnitTest.h"

#include "quat.h"
#include "float33.h"
#include "MathUtil.h"

using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS( quatTests )
	{
		public:

		TEST_METHOD( quat_slerp_1 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 1.0f, 0.0f, 0.0f, 0.0f );

			quat result = quat::slerp( q1, q2, 0.0f );
			quat expectedResult( 1.0f, 0.0f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_2 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 1.0f, 0.0f, 0.0f, 0.0f );

			quat result = quat::slerp( q1, q2, 1.0f );
			quat expectedResult( 1.0f, 0.0f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_3 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 1.0f, 0.0f, 0.0f, 0.0f );

			quat result = quat::slerp( q1, q2, 0.5f );
			quat expectedResult( 1.0f, 0.0f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_4 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 0.5f, 0.5f, -0.5f, 0.5f );

			quat result = quat::slerp( q1, q2, 0.0f );
			quat expectedResult( 1.0f, 0.0f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_5 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 0.5f, 0.5f, -0.5f, 0.5f );

			quat result = quat::slerp( q1, q2, 1.0f );
			quat expectedResult( 0.5f, 0.5f, -0.5f, 0.5f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_6 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 0.5f, 0.5f, -0.5f, 0.5f );

			quat result = quat::slerp( q1, q2, 0.5f );
			quat expectedResult( 0.8660254037844388f, 0.2886751345948129f, -0.2886751345948129f, 0.2886751345948129f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_7 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 0.5f, 0.5f, -0.5f, 0.5f );

			quat result = quat::slerp( q1, q2, 0.2f );
			quat expectedResult( 0.9781476007338059f, 0.12003787066130363f, -0.12003787066130363f, 0.12003787066130363f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_8 )
		{
			quat q1( 1.0f, 0.0f, 0.0f, 0.0f );
			quat q2( 0.5f, 0.5f, -0.5f, 0.5f );

			quat result = quat::slerp( q1, q2, 0.8f );
			quat expectedResult( 0.6691306063588582f, 0.4290548650362511f, -0.4290548650362511f, 0.4290548650362511f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( quat_slerp_9 )
		{
			quat q1( 0.7233174113647118f, 0.3919038373291199f, -0.200562121146575f, 0.5319756951821668f );
			quat q2( 0.948979454430948f, -0.23081308598761174f, -0.168722160571667f, 0.13302686547026613f );

			quat result = quat::slerp( q1, q2, 0.5f );
			quat expectedResult( 0.9067801288856223f, 0.0873492591246556f, -0.20023935667222548f, 0.36058855337911316f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult ) );
		}

		TEST_METHOD( matrix_to_quat_1 )
		{
			float33 mtx( 1.0f, 0.0f, 0.0f,
						 0.0f, 1.0f, 0.0f,
						 0.0f, 0.0f, 1.0f );

			quat result = quat( mtx );
			quat expectedResult( 1.0f, 0.0f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.0001f ) );
		}

		TEST_METHOD( matrix_to_quat_2 )
		{
			float33 mtx( 1.0f,  0.0f, 0.0f,
						 0.0f,  0.0f, 1.0f,
						 0.0f, -1.0f, 0.0f );

			quat result = quat( mtx );
			quat expectedResult( 0.7071f, 0.7071f, 0.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.001f ) );
		}

		TEST_METHOD( matrix_to_quat_3 )
		{
			float33 mtx(  0.61237f,  0.61237f, -0.50000f,
						 -0.04736f,  0.65973f,  0.75000f,
						  0.78914f, -0.43559f,  0.43301f );

			quat result = quat( mtx );
			quat expectedResult( 0.82236f, 0.36042f, 0.39190f, 0.20056f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.001f ) );
		}

		TEST_METHOD( quat_to_matrix_1 )
		{
			quat q( 1.0f, 0.0f, 0.0f, 0.0f );

			float33 result = float33( q );
			float33 expectedResult( 1.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 1.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.0001f ) );
		}

		TEST_METHOD( quat_to_matrix_2 )
		{
			quat q( 0.7071f, 0.7071f, 0.0f, 0.0f );

			float33 result = float33( q );
			float33 expectedResult( 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f,
									0.0f, -1.0f, 0.0f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.0001f ) );
		}

		TEST_METHOD( quat_to_matrix_3 )
		{
			quat q( 0.82236f, 0.36042f, 0.39190f, 0.20056f );

			float33 result = float33( q );
			float33 expectedResult(  0.61237f,  0.61237f, -0.50000f,
									-0.04736f,  0.65973f,  0.75000f,
									 0.78914f, -0.43559f,  0.43301f );

			Assert::IsTrue( MathUtil::areEqual( result, expectedResult, 0.0001f ) );
		}

		TEST_METHOD( matrix_to_quat_to_matrix_1 )
		{
			float33 mtx( 1.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f,
						 0.0f, -1.0f, 0.0f );

			quat result_quat = quat( mtx );
			float33 result_mtx = float33( result_quat );

			Assert::IsTrue( MathUtil::areEqual( result_mtx, mtx, 0.00001f ) );
		}
	};
}