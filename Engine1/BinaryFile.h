#pragma once

#include <vector>
#include <memory>

#include "float4.h"
#include "float43.h"

namespace Engine1
{
    class BinaryFile
    {
        public:

        static std::shared_ptr< std::vector<char> > load( const std::string& path );
        static void                                 save( const std::string& path, std::vector<char>& data );

        static std::string readText( std::vector<char>::const_iterator& dataIt, const int size );
        static char        readChar( std::vector<char>::const_iterator& dataIt );
        static int         readInt( std::vector<char>::const_iterator& dataIt );
        static bool        readBool( std::vector<char>::const_iterator& dataIt );
        static float4      readFloat4( std::vector<char>::const_iterator& dataIt );
        static float43     readFloat43( std::vector<char>::const_iterator& dataIt );

        static void writeText( std::vector<char>& data, const std::string& text );
        static void writeChar( std::vector<char>& data, const char value );
        static void writeInt( std::vector<char>& data, const int value );
        static void writeBool( std::vector<char>& data, const bool value );
        static void writeFloat4( std::vector<char>& data, const float4& value );

        static void writeFloat43( std::vector<char>& data, const float43& value );
    };
}

