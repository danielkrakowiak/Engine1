#include "stdafx.h"
#include "CppUnitTest.h"

#include "float44.h"
#include "MathUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//TODO: tests to add
// - non-unform scaling

namespace UnitTests {
	TEST_CLASS( float44Tests ) {
	public:

	TEST_METHOD( float44_getScaleRotationTranslationInverse_1__Identity ) {
		float44 transf, invTransf;

		transf.identity( );

		invTransf = transf.getScaleOrientationTranslationInverse();

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2 ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_2__Translation ) {
		float44 transf, invTransf;

		transf.identity( );
		transf.setTranslation( float3( 13.0f, 2.0f, 8.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2 ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_3__Uniform_Scaling ) {
		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( transf.getOrientation( ) * 3.0f );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_4__Non_Uniform_Scaling ) {
		float44 transf, invTransf;

		transf.identity( );
		transf.setRotationRow1( transf.getRotationRow1() * 3.0f );
		transf.setRotationRow2( transf.getRotationRow2( ) * -0.4f );
		transf.setRotationRow3( transf.getRotationRow3( ) * 0.7f );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_5__Uniform_Scaling_Translation ) {
		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( transf.getOrientation( ) * 3.0f );
		transf.setRow4( float4( 13.0f, 2.0f, 8.0f, 1.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_6__Non_Uniform_Scaling_Translation ) {
		float44 transf, invTransf;

		transf.identity( );
		transf.setRotationRow1( transf.getRotationRow1( ) * 3.0f );
		transf.setRotationRow2( transf.getRotationRow2( ) * -0.4f );
		transf.setRotationRow3( transf.getRotationRow3( ) * 0.7f );
		transf.setRow4( float4( 13.0f, 2.0f, 8.0f, 1.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_7__Rotation ) {
		float3 baseVectorForward( 0.4f, 0.3f, 0.8f ); baseVectorForward.normalize( );
		float3 up( -0.1f, 0.9f, 0.2f ); up.normalize( );
		float3 baseVectorSide = float3::cross( up, baseVectorForward ); baseVectorSide.normalize();
		float3 baseVectorUp = float3::cross( baseVectorForward, baseVectorSide ); baseVectorUp.normalize( );

		float33 rotationTransf( baseVectorSide, baseVectorUp, baseVectorForward );

		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( rotationTransf );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_7__Rotation_Translation ) {
		float3 baseVectorForward( 0.4f, 0.3f, 0.8f ); baseVectorForward.normalize( );
		float3 up( -0.1f, 0.9f, 0.2f ); up.normalize( );
		float3 baseVectorSide = float3::cross( up, baseVectorForward ); baseVectorSide.normalize( );
		float3 baseVectorUp = float3::cross( baseVectorForward, baseVectorSide ); baseVectorUp.normalize( );

		float33 rotationTransf( baseVectorSide, baseVectorUp, baseVectorForward );

		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( rotationTransf );
		transf.setTranslation( float3( 13.0f, 2.0f, 8.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_7__Uniform_Scaling_Rotation_Translation ) {
		float3 baseVectorForward( 0.4f, 0.3f, 0.8f ); baseVectorForward.normalize( );
		float3 up( -0.1f, 0.9f, 0.2f ); up.normalize( );
		float3 baseVectorSide = float3::cross( up, baseVectorForward ); baseVectorSide.normalize( );
		float3 baseVectorUp = float3::cross( baseVectorForward, baseVectorSide ); baseVectorUp.normalize( );

		float33 rotationTransf( baseVectorSide, baseVectorUp, baseVectorForward );

		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( rotationTransf * 3.0f );
		transf.setTranslation( float3( 13.0f, 2.0f, 8.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	TEST_METHOD( float44_getScaleRotationTranslationInverse_7__Non_Uniform_Scaling_Rotation_Translation ) {
		float3 baseVectorForward( 0.4f, 0.3f, 0.8f ); baseVectorForward.normalize( );
		float3 up( -0.1f, 0.9f, 0.2f ); up.normalize( );
		float3 baseVectorSide = float3::cross( up, baseVectorForward ); baseVectorSide.normalize( );
		float3 baseVectorUp = float3::cross( baseVectorForward, baseVectorSide ); baseVectorUp.normalize( );

		float33 rotationTransf( baseVectorSide, baseVectorUp, baseVectorForward );

		float44 transf, invTransf;

		transf.identity( );
		transf.setOrientation( rotationTransf );
		transf.setRotationRow1( transf.getRotationRow1( ) * 3.0f );
		transf.setRotationRow2( transf.getRotationRow2( ) * -0.4f );
		transf.setRotationRow3( transf.getRotationRow3( ) * 0.7f );
		transf.setTranslation( float3( 13.0f, 2.0f, 8.0f ) );

		invTransf = transf.getScaleOrientationTranslationInverse( );

		float4 pointPos( 5.0f, 3.0f, 2.0f, 1.0f );

		float4 pointTransformed = pointPos * transf;
		float4 pointTransformed2 = pointTransformed * invTransf;

		Assert::IsTrue( MathUtil::areEqual( pointPos, pointTransformed2, 0.00001f, MathUtil::epsilonFifty ) );
	}

	};
}