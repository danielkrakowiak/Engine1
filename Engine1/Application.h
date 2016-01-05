#pragma once

#include "InputManager.h"
#include "Direct3DRendererCore.h"
#include "Renderer.h"
#include "Direct3DFrameRenderer.h"
#include "Direct3DDefferedRenderer.h"
#include "RaytraceRenderer.h"
#include "FreeCamera.h"

#include "ImageLibrary.h"
#include "FontLibrary.h"

#include "AssetManager.h"

using namespace Engine1;

namespace Engine1
{
    class CScene;
    class BlockActor;
    class SkeletonActor;
}

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

	// Initialization
	bool initialized;
	void setupWindow( );

	// Windows message handling.
	static const unsigned int inputTimerId = 1;
	static const unsigned int inputTimerInterval = 5;
	static Application* windowsMessageReceiver;

	static LRESULT CALLBACK windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void onStart( );
	void onExit( );
	void onResize( int newWidth, int newHeight );
	void onFocusChange( bool windowFocused );
    void onKeyPress( int key );
	void onDragAndDropFile( std::string filePath );

    void loadScene( std::string path );
    void saveScene( std::string path );

	// Basic application handles.
	HINSTANCE applicationInstance;
	HWND      windowHandle;
	HDC       deviceContext;

	InputManager inputManager;

	Direct3DRendererCore     rendererCore;
    Direct3DDefferedRenderer defferedRenderer;
    RaytraceRenderer         raytraceRenderer;
    Direct3DFrameRenderer    frameRenderer;
    Renderer                 renderer;

	bool fullscreen;
	int  screenWidth;
    int  screenHeight;
	bool verticalSync;
	int  displayFrequency;
	char screenColorDepth;
	char zBufferDepth;

	bool windowFocused;

	FreeCamera camera;

    AssetManager assetManager;

	// For creation of new assets.
	std::shared_ptr<BlockActor>    defaultBlockActor;
	std::shared_ptr<SkeletonActor> defaultSkeletonActor;

    std::string scenePath;
    std::shared_ptr<CScene> scene;

	// Copying is not allowed.
	Application( const Application& ) = delete;
	Application& operator=( const Application& ) = delete;
};

