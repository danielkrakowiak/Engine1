#pragma once

#include "InputManager.h"
#include "Direct3DRendererCore.h"
#include "Direct3DFrameRenderer.h"
#include "Direct3DDefferedRenderer.h"
#include "FreeCamera.h"

#include "ImageLibrary.h"
#include "FontLibrary.h"

class Application {
public:
	Application();
	~Application();

	void initialize( HINSTANCE applicationInstance );
	void show();
	void run();

	bool isFullscreen() { return fullscreen; }

	int getScreenWidth() { return screenWidth; }
	int getScreenHeight() { return screenHeight; }
	int getDisplayFrequency() { return displayFrequency; }
	int getScreenColorDepth() { return screenColorDepth; }
	int getZBufferDepth() { return zBufferDepth; }


private:

	static ImageLibrary imageLibrary;
	static FontLibrary fontLibrary;

	//initialization
	bool initialized;
	void setupWindow( );

	//Windows message handling
	static const unsigned int inputTimerId = 1;
	static const unsigned int inputTimerInterval = 5;
	static Application* windowsMessageReceiver;

	static LRESULT CALLBACK windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void onStart( );
	void onExit( );
	void onResize( int newWidth, int newHeight );
	void onFocusChange( bool windowFocused );

	//basic application handles
	HINSTANCE applicationInstance;
	HWND windowHandle;
	HDC deviceContext;

	InputManager inputManager;
	Direct3DRendererCore  direct3DRendererCore;
	Direct3DFrameRenderer direct3DFrameRenderer;
	Direct3DDefferedRenderer direct3DDefferedRenderer;

	bool fullscreen;
	int screenWidth, screenHeight;
	bool verticalSync;
	int displayFrequency;
	int screenColorDepth;
	int zBufferDepth;

	bool windowFocused;

	FreeCamera camera;

	// Copying is not allowed.
	Application( const Application& ) = delete;
	Application& operator=( const Application& ) = delete;
};

