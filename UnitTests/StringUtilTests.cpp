#include "stdafx.h"
#include "CppUnitTest.h"

#include <string>

#include "StringUtil.h"

using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS( StringUtilTestSet ) {
public:

	TEST_METHOD( String_ToUppercase_1 ) {
		std::string str = "abcdefgh";
		std::string correctStr = "ABCDEFGH";

		try {
			std::string str2 = StringUtil::toUppercase( str );

			Assert::IsTrue( str2.compare( correctStr ) == 0, L"StringUtil::toUppercase returned incorrect string" );
		} catch ( ... ) {
			Assert::Fail( L"StringUtil::toUppercase threw an exception" );
		}
	}

	TEST_METHOD( String_ToUppercase_2 ) {
		std::string str = "ùûüÿàâæçéèêëïîôœ ãñõøåþšžвцйфячмитьб";
		std::string correctStr = "ÙÛÜŸÀÂÆÇÉÈÊËÏÎÔŒ ÃÑÕØÅÞŠŽВЦЙФЯЧМИТЬБ";

		try {
			std::string str2 = StringUtil::toUppercase( str );

			Assert::IsTrue( str2.compare( correctStr ) == 0, L"StringUtil::toUppercase returned incorrect string" );
		} catch ( ... ) {
			Assert::Fail( L"StringUtil::toUppercase threw an exception" );
		}
	}

	TEST_METHOD( String_ToLowercase_1 ) {
		std::string str = "ABCDEFGH";
		std::string correctStr = "abcdefgh";

		try {
			std::string str2 = StringUtil::toLowercase( str );

			Assert::IsTrue( str2.compare( correctStr ) == 0, L"StringUtil::toLowercase returned incorrect string" );
		} catch ( ... ) {
			Assert::Fail( L"StringUtil::toLowercase threw an exception" );
		}
	}

	TEST_METHOD( String_ToLowercase_2 ) {
		std::string str = "ÙÛÜŸÀÂÆÇÉÈÊËÏÎÔŒ ÃÑÕØÅÞŠŽВЦЙФЯЧМИТЬБ";
		std::string correctStr = "ùûüÿàâæçéèêëïîôœ ãñõøåþšžвцйфячмитьб";

		try {
			std::string str2 = StringUtil::toLowercase( str );

			Assert::IsTrue( str2.compare( correctStr ) == 0, L"StringUtil::toLowercase returned incorrect string" );
		} catch ( ... ) {
			Assert::Fail( L"StringUtil::toLowercase threw an exception" );
		}
	}

	};
}