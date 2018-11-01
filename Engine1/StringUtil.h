#pragma once

#include <string>
#include <locale>

namespace Engine1
{
    namespace StringUtil
    {

        //use when converting UTF-8 to WinAPI UTF-16 functions
        std::wstring widen( const char *s );
        std::wstring widen( const std::string &s );

        //use when converting text received from WinAPI UTF-16 to UTF-8
        std::string narrow( const wchar_t *s );
        std::string narrow( const std::wstring &s );

        std::string toLowercase( const std::string& str );

        std::string toUppercase( const std::string& str );

        std::string replaceSubstring( const std::string& str, const std::string& from, const std::string& to );
        std::string replaceAllSubstrings( const std::string& str, const std::string& from, const std::string& to );

        std::string getSubstrBefore( const std::string& str, const std::string& pivot );
        std::string getSubstrAfter( const std::string& str, const std::string& pivot );
        std::string getSubstrBetween( const std::string& str, const std::string& leftLimit, const std::string& rightLimit );

        extern std::locale locale;
        extern const std::string localeName;

    };
}