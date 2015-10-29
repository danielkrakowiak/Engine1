#include "StringUtil.h"

#include <Windows.h>

const std::string StringUtil::localeName = "en-US";
std::locale StringUtil::locale( localeName );

std::wstring StringUtil::widen( const char *narrowString ) {
	int wideCharCount = MultiByteToWideChar( CP_UTF8, 0, narrowString, -1, nullptr, 0 );
	std::wstring wideString( wideCharCount, 0 );
	MultiByteToWideChar( CP_UTF8, 0, narrowString, -1, &wideString[ 0 ], wideCharCount );

	return wideString;
}

std::wstring StringUtil::widen( const std::string &narrowString ) {
	int wideCharCount = MultiByteToWideChar( CP_UTF8, 0, &narrowString[ 0 ], (int)narrowString.size( ), nullptr, 0 );
	std::wstring wideString( wideCharCount, 0 );
	MultiByteToWideChar( CP_UTF8, 0, &narrowString[ 0 ], (int)narrowString.size( ), &wideString[ 0 ], wideCharCount );

	return wideString;
}

std::string StringUtil::narrow( const wchar_t *wideString ) {
	int charCount = WideCharToMultiByte( CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr );
	std::string narrowString( charCount, 0 );
	WideCharToMultiByte( CP_UTF8, 0, wideString, -1, &narrowString[ 0 ], charCount, nullptr, nullptr );
	
	return narrowString;
}

std::string StringUtil::narrow( const std::wstring &wideString ) {
	int charCount = WideCharToMultiByte( CP_UTF8, 0, &wideString[ 0 ], (int)wideString.size( ), nullptr, 0, nullptr, nullptr );
	std::string narrowString( charCount, 0 );
	WideCharToMultiByte( CP_UTF8, 0, &wideString[ 0 ], (int)wideString.size( ), &narrowString[ 0 ], charCount, nullptr, nullptr );
	
	return narrowString;
}

//std::string StringUtil::toLowercase( const std::string str ) {
//	std::string strLowercase = str;
//
//	const std::string::size_type strLength = str.length( );
//
//	for ( std::string::size_type i = 0; i < strLength; ++i ) {
//		strLowercase[ i ] = std::tolower( str[ i ], loc );
//	}
//
//	return strLowercase;
//}
//
//std::string StringUtil::toUppercase( const std::string str ) {
//	std::string strUppercase = str;
//
//	const std::string::size_type strLength = str.length();
//
//	for ( std::string::size_type i = 0; i < strLength; ++i ) {
//		strUppercase[ i ] = std::toupper( str[ i ], loc );
//	}
//
//	return strUppercase;
//}

std::string StringUtil::toLowercase( const std::string& str ) {
	std::wstring wstrLowercase = widen(str);

	const std::wstring::size_type wstrLength = wstrLowercase.length( );

	for ( std::wstring::size_type i = 0; i < wstrLength; ++i ) {
		wstrLowercase[ i ] = std::tolower( wstrLowercase[ i ], locale );
	}

	return narrow(wstrLowercase);
}

std::string StringUtil::toUppercase( const std::string& str ) {
	std::wstring wstrUppercase = widen(str);

	const std::wstring::size_type wstrLength = wstrUppercase.length( );

	for ( std::wstring::size_type i = 0; i < wstrLength; ++i ) {
		wstrUppercase[ i ] = std::toupper( wstrUppercase[ i ], locale );
	}

	return narrow(wstrUppercase);
}