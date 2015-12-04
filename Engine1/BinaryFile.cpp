#include "BinaryFile.h"

#include <fstream>

#include "float4.h"

using namespace Engine1;

std::shared_ptr< std::vector<char> > BinaryFile::load( const std::string& path )
{

	std::ifstream file;
	std::vector<char> fileData;

	// Open the file.
	file.open( path.c_str(), std::ifstream::in | std::ifstream::binary );

	// Check if open succeeded.
	if ( !file.is_open() )	throw std::exception( "BinaryFile::load - Failed to open file." );

	try {
		// Check file size.
		std::streamoff fileSize = 0;
		file.seekg( 0, std::ios::end ); // Move cursor to the end of the file.
		fileSize = file.tellg(); // Save cursor position.

		// Allocate memory for the file (+ zero at the end).
		fileData.reserve( fileSize + 1 );
		fileData.resize( fileSize + 1 );

		// Read the file.
		file.seekg( 0, std::ios::beg );  // Move cursor to the beginning of the file.
		file.read( fileData.data(), fileSize );

		// Trim the data vector to the real number of characters read (on Windows multiple bytes can be read as one character - ex: LF CR -> '\n').
		fileData.resize( file.gcount() + 1 );

		// Set last character in the buffer to 0.
		fileData.at( file.gcount() ) = 0;

		// Close the file.
		file.close();

		return std::make_shared< std::vector<char> >( fileData );

	} catch ( ... ) {
		// In case of errors - close the file.
		file.close();

		throw std::exception( "BinaryFile::load - Error occured." );
	}
}

void BinaryFile::save( const std::string& path, std::vector<char>& data )
{
	std::ofstream file;

	// Open the file.
	file.open( path.c_str(), std::ofstream::out | std::ofstream::binary );

	// Check if open succeeded.
	if ( !file.is_open() )	throw std::exception( "BinaryFile::save - Failed to open file." );

	try {
		file.write( data.data( ), data.size( ) );

		// Close the file.
		file.close();
	} catch ( ... ) {
		// In case of errors - close the file.
		file.close();

		throw std::exception( "BinaryFile::save - Error occured." );
	}
}

std::string BinaryFile::readText( std::vector<char>::const_iterator& dataIt, const int size )
{
	std::string text( reinterpret_cast<const char*>(&( *dataIt )), size );

	dataIt += size;

	return text;
}

int BinaryFile::readInt( std::vector<char>::const_iterator& dataIt )
{
	int value = 0;
	std::memcpy( &value, &( *dataIt ), sizeof( int ) );

	dataIt += sizeof( int ) / sizeof( char );

	return value;
}

bool BinaryFile::readBool( std::vector<char>::const_iterator& dataIt )
{
	bool value = 0;
	std::memcpy( &value, &( *dataIt ), sizeof( bool ) );

	dataIt += sizeof( bool ) / sizeof( char );

	return value;
}

float4 BinaryFile::readFloat4( std::vector<char>::const_iterator& dataIt )
{
	float4 value( 0.0f, 0.0f, 0.0f, 0.0f );
	std::memcpy( &value, &( *dataIt ), sizeof( float4 ) );

	dataIt += sizeof( float4 ) / sizeof( char );

	return value;
}

void BinaryFile::writeText( std::vector<char>& file, const std::string& text )
{
	for ( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
		file.push_back( *it );
}

void BinaryFile::writeInt( std::vector<char>& file, const int value )
{
	const int size = file.size();

	const int sizeIncrease = sizeof( int ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( int ) );
}

void BinaryFile::writeBool( std::vector<char>& file, const bool value )
{
	const int size = file.size();

	const int sizeIncrease = sizeof( bool ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( bool ) );
}

void BinaryFile::writeFloat4( std::vector<char>& file, const float4& value )
{
	const int size = file.size();

	const int sizeIncrease = sizeof( float4 ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( float4 ) );
}