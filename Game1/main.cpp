#include <Windows.h>
#include <fstream>

#include "GameApplication.h"

#include "StringUtil.h"

using namespace Engine1;

void writeErrorToFile( std::string path, std::string errorMsg );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
    // Unused.
    hPrevInstance;
    lpCmdLine;
    nShowCmd;

    writeErrorToFile( "begin.txt", "tralala" );

    try {
        GameApplication application;

        application.initialize( hInstance );
        application.show();
        application.run();
    } catch ( std::exception& e ) {
        OutputDebugStringW( StringUtil::widen( e.what() + std::string( "\n" ) ).c_str() );
        writeErrorToFile( "error.txt", ( e.what() + std::string( "\n" ) ) );
    }

    return 0;
}

void writeErrorToFile( std::string path, std::string errorMsg )
{
    std::ofstream file;

    // Open the file.
    file.open( path.c_str() );

    // Check if open succeeded.
    if ( !file.is_open() )	throw std::exception( "writeErrorToFile - Failed to open file." );

    try {
        file.write( errorMsg.c_str(), errorMsg.size() );
        file.close();
    } catch ( ... ) {
        file.close();
        throw std::exception( "writeErrorToFile - Error occurred." );
    }
}

