#include "FontLibrary.h"

FT_Library FontLibrary::library;

FontLibrary::FontLibrary() {
	FT_Init_FreeType( &library );
}


FontLibrary::~FontLibrary() {
	FT_Done_FreeType( library );
}

FT_Library& FontLibrary::get() {
	return library;
}
