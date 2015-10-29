#include "stdafx.h"
#include "CppUnitTest.h"

#include "MathUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(MathUtilTests)
	{
	public:
		
		TEST_METHOD(MathUtil_AreEqual_1) {
			Assert::IsFalse( MathUtil::areEqual( 5.0f, 10.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned true" );
			Assert::IsFalse( MathUtil::areEqual( 5.0f, -5.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned true" );
			Assert::IsFalse( MathUtil::areEqual( 0.0f,  3.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned true" );
			Assert::IsFalse( MathUtil::areEqual( -1.0f, 1.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned true" );
			Assert::IsFalse( MathUtil::areEqual( 99999.0f, 1.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned true" );

			Assert::IsTrue( MathUtil::areEqual( 1.0f, 1.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( -1.0f, -1.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( 500.0f, 500.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( 50000.0f, 50000.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( -0.0f, 0.0f, 0.001f, 0.001f ), L"MathUtil::areEqual() returned false" );

			Assert::IsTrue( MathUtil::areEqual( 1000.0f, 1001.0f, 0.01f, 1.0f ), L"MathUtil::areEqual() returned false" );
			Assert::IsFalse( MathUtil::areEqual( 1000.0f, 1001.0f, 0.000001f, 0.99f ), L"MathUtil::areEqual() returned true" );
			Assert::IsTrue( MathUtil::areEqual( 1000.0f, 1001.0f, 0.01f, 0.99f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( 1000.0f, 1001.0f, 0.001f, 1.1f ), L"MathUtil::areEqual() returned false" );
			Assert::IsTrue( MathUtil::areEqual( 1000.0f, 1001.0f, 0.00000001f, 1.1f ), L"MathUtil::areEqual() returned false" );

			Assert::IsFalse( MathUtil::areEqual( 0.0f, 2.0f * FLT_EPSILON, 0.001f, FLT_EPSILON ), L"MathUtil::areEqual() returned true" );
			Assert::IsTrue( MathUtil::areEqual( 0.0f, 2.0f * FLT_EPSILON, 0.001f, 4.0f * FLT_EPSILON ), L"MathUtil::areEqual() returned false" );
		}

	};
}