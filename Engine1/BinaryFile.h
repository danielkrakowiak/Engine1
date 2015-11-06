#pragma once

#include <vector>
#include <memory>

#include "float4.h"

class BinaryFile
{
	public:

	static std::shared_ptr< std::vector<unsigned char> > load( const std::string& path );
	static void save( const std::string& path, std::vector<unsigned char>& data );

	static std::string readText( std::vector<unsigned char>::const_iterator& dataIt, const int size );
	static int         readInt( std::vector<unsigned char>::const_iterator& dataIt );
	static bool        readBool( std::vector<unsigned char>::const_iterator& dataIt );
	static float4      readFloat4( std::vector<unsigned char>::const_iterator& dataIt );

	static void writeText( std::vector<unsigned char>& data, const std::string& text );
	static void writeInt( std::vector<unsigned char>& data, const int value );
	static void writeBool( std::vector<unsigned char>& data, const bool value );
	static void writeFloat4( std::vector<unsigned char>& data, const float4& value );
};

