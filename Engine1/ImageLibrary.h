#pragma once

/* 
This class is used to initialize/deinitialize FreeImage library.
To initialize/deinitialize simply create a static field of that class in the main application class.
*/

class Application;
enum FREE_IMAGE_FORMAT;

namespace Engine1
{
    class ImageLibrary
    {
        friend class Application;

        private:

        ImageLibrary();
        ~ImageLibrary();

        static void errorHandler( FREE_IMAGE_FORMAT fif, const char *message );

        // Copying is not allowed.
        ImageLibrary( const ImageLibrary& ) = delete;
        ImageLibrary& operator=(const ImageLibrary&) = delete;
    };
}
