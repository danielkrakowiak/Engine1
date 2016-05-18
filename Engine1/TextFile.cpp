#include "TextFile.h"

#include <fstream>

using namespace Engine1;

std::shared_ptr< std::vector<char> > TextFile::load( const std::string& path ) {

	std::ifstream file;
	std::vector<char> fileData;

	// Open the file.
	file.open( path.c_str( ), std::ifstream::in | std::ifstream::binary );

	// Check if open succeeded.
	if ( !file.is_open( ) )	throw std::exception( ("TextFile::load - Failed to open file. (" + path + ")").c_str() );

	try {
		// Check file size.
		std::streamoff fileSize = 0;
		file.seekg( 0, std::ios::end ); // Move cursor to the end of the file.
		fileSize = file.tellg( ); // Save cursor position.

		// Allocate memory for the file.
		fileData.reserve( (size_t)fileSize + 1 );
		fileData.resize( (size_t)fileSize + 1 );

		// Read the file.
		file.seekg( 0, std::ios::beg );  // Move cursor to the beginning of the file.
		file.read( reinterpret_cast<char*>( fileData.data( ) ), fileSize );

		// Trim the data vector to the real number of characters read (on Windows multiple bytes can be read as one character - ex: LF CR -> '\n').
		fileData.resize( (size_t)file.gcount() + 1 );

		// Set last character in the buffer to 0.
		fileData.at( (size_t)file.gcount() ) = 0;

		// Close the file.
		file.close( );

		return std::make_shared< std::vector<char> >( fileData );

	} catch ( ... ) {
		// In case of errors - close the file.
		file.close( );

		throw std::exception( "TextFile::loadText - Error occured." );
	}
}
