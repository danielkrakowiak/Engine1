

#include <Windows.h>

#include "Application.h"

#include "StringUtil.h"


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	try {
		Application application;

		application.initialize( hInstance );
		application.show();
		application.run();
	} catch ( std::exception& e ) {
		OutputDebugStringW( StringUtil::widen( e.what() + std::string("\n") ).c_str() );
	}

	return 0;
}

