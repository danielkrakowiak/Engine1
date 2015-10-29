#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

/*
This class is used to initialize/deinitialize FreeType library.
To initialize/deinitialize simply create a static field of that class in the main application class.
*/

class FontLibrary {
	friend class Application;

	public:

	static FT_Library& get();

	private:

	static FT_Library library;

	FontLibrary();
	~FontLibrary();

	// Copying is not allowed.
	FontLibrary( const FontLibrary& ) = delete;
	FontLibrary& operator=( const FontLibrary& ) = delete;
};

