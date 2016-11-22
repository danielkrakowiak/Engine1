#include "BinaryFile.h"

#include <fstream>

#include "File.h"

using namespace Engine1;

std::shared_ptr< std::vector<char> > BinaryFile::load( const std::string& path )
{

	std::ifstream file;
	auto fileData = std::make_shared< std::vector< char > >();

    // Get file size.
    const long long fileSize = File::getFileSize( path );

	// Open the file.
	file.open( path.c_str(), std::ifstream::in | std::ifstream::binary );

	// Check if open succeeded.
	if ( !file.is_open() )	
        throw std::exception( "BinaryFile::load - Failed to open file." );

	try {
		// Check file size.
		//std::streamoff fileSize = 0;
		//file.seekg( 0, std::ios::end ); // Move cursor to the end of the file.
		//fileSize = file.tellg(); // Save cursor position.

		// Allocate memory for the file (+ zero at the end).
		fileData->reserve( (size_t)fileSize );
		fileData->resize( (size_t)fileSize );

		// Read the file.
		file.seekg( 0, std::ios::beg );  // Move cursor to the beginning of the file.
		file.read( fileData->data(), fileSize );

		// Close the file.
		file.close();

		return fileData;

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
	if ( !file.is_open() )	
        throw std::exception( "BinaryFile::save - Failed to open file." );

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

char BinaryFile::readChar( std::vector<char>::const_iterator& dataIt )
{
    char value = 0;
    std::memcpy( &value, &(*dataIt), sizeof(char) );

    dataIt += sizeof(char) / sizeof(char);

    return value;
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

float BinaryFile::readFloat( std::vector<char>::const_iterator& dataIt )
{
    float value = 0;
    std::memcpy( &value, &( *dataIt ), sizeof( float ) );

    dataIt += sizeof( float ) / sizeof( char );

    return value;
}

float3 BinaryFile::readFloat3( std::vector<char>::const_iterator& dataIt )
{
    float3 value( 0.0f, 0.0f, 0.0f );
    std::memcpy( &value, &(*dataIt), sizeof(float3) );

    dataIt += sizeof(float3) / sizeof(char);

    return value;
}

float4 BinaryFile::readFloat4( std::vector<char>::const_iterator& dataIt )
{
	float4 value( 0.0f, 0.0f, 0.0f, 0.0f );
	std::memcpy( &value, &( *dataIt ), sizeof( float4 ) );

	dataIt += sizeof( float4 ) / sizeof( char );

	return value;
}

float43 BinaryFile::readFloat43( std::vector<char>::const_iterator& dataIt )
{
    float43 value( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
    std::memcpy( &value, &(*dataIt), sizeof(float43) );

    dataIt += sizeof(float43) / sizeof(char);

    return value;
}

void BinaryFile::writeText( std::vector<char>& file, const std::string& text )
{
	for ( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
		file.push_back( *it );
}

void BinaryFile::writeChar( std::vector<char>& file, const char value )
{
    const size_t size = file.size();

    const size_t sizeIncrease = sizeof(char) / sizeof(char);

    file.resize( size + sizeIncrease );

    std::memcpy( &file[ size ], &value, sizeof(char) );
}

void BinaryFile::writeInt( std::vector<char>& file, const int value )
{
	const size_t size = file.size();

	const size_t sizeIncrease = sizeof( int ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( int ) );
}

void BinaryFile::writeBool( std::vector<char>& file, const bool value )
{
	const size_t size = file.size();

	const size_t sizeIncrease = sizeof( bool ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( bool ) );
}

void BinaryFile::writeFloat( std::vector<char>& file, const float value )
{
    const size_t size = file.size();

    const size_t sizeIncrease = sizeof( float ) / sizeof( char );

    file.resize( size + sizeIncrease );

    std::memcpy( &file[ size ], &value, sizeof( float ) );
}

void BinaryFile::writeFloat3( std::vector<char>& file, const float3& value )
{
    const size_t size = file.size();

    const size_t sizeIncrease = sizeof(float3) / sizeof(char);

    file.resize( size + sizeIncrease );

    std::memcpy( &file[ size ], &value, sizeof(float3) );
}

void BinaryFile::writeFloat4( std::vector<char>& file, const float4& value )
{
	const size_t size = file.size();

	const size_t sizeIncrease = sizeof( float4 ) / sizeof( char );

	file.resize( size + sizeIncrease );

	std::memcpy( &file[ size ], &value, sizeof( float4 ) );
}

void BinaryFile::writeFloat43( std::vector<char>& file, const float43& value )
{
    const size_t size = file.size();

    const size_t sizeIncrease = sizeof(float43) / sizeof(char);

    file.resize( size + sizeIncrease );

    std::memcpy( &file[ size ], &value, sizeof(float43) );
}