#include "ImageLibrary.h"

#include <iostream>

using namespace Engine1;

ImageLibrary::ImageLibrary() {
	FreeImage_Initialise( );

	FreeImage_SetOutputMessage( errorHandler );
}


ImageLibrary::~ImageLibrary() {
	FreeImage_DeInitialise( );
}

/**
FreeImage error handler
@param fif Format / Plugin responsible for the error
@param message Error message
*/
void ImageLibrary::errorHandler( FREE_IMAGE_FORMAT fif, const char *message ) {
	std::cout << std::endl;
	if ( fif != FIF_UNKNOWN ) {
		std::cout << FreeImage_GetFormatFromFIF( fif ) << "Format\n";
	}
	std::cout << message << std::endl;
}
