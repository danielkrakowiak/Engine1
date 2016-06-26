#pragma once

#include "InputManager.h"
#include "Direct3DRendererCore.h"
#include "Renderer.h"
#include "Direct3DFrameRenderer.h"
#include "Direct3DDeferredRenderer.h"
#include "RaytraceRenderer.h"
#include "ShadingRenderer.h"
#include "EdgeDetectionRenderer.h"
#include "CombiningRenderer.h"
#include "TextureRescaleRenderer.h"
#include "FreeCamera.h"

#include "ImageLibrary.h"
#include "FontLibrary.h"

#include "AssetManager.h"

using namespace Engine1;

namespace Engine1
{
    class CScene;
    class Actor;
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
    void onMove( int newPosX, int newPosY );
	void onFocusChange( bool windowFocused );
    void onKeyPress( int key );
    void onMouseButtonPress( int button );
	void onDragAndDropFile( std::string filePath );

    std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light > > 
        pickActorOrLight( const CScene& scene, const Camera& camera, const float2& targetPixel,
                          const float screenWidth, const float screenHeight, const float fieldOfView );

    void loadScene( std::string path );
    void saveScene( std::string path );

	// Basic application handles.
	HINSTANCE applicationInstance;
	HWND      windowHandle;
	HDC       deviceContext;

    // Window state.
    int2 windowPosition;

	InputManager inputManager;

	Direct3DRendererCore     rendererCore;
    Direct3DDeferredRenderer deferredRenderer;
    RaytraceRenderer         raytraceRenderer;
    ShadingRenderer          shadingRenderer;
    EdgeDetectionRenderer    edgeDetectionRenderer;
    CombiningRenderer        combiningRenderer;
    Direct3DFrameRenderer    frameRenderer;
    TextureRescaleRenderer   textureRescaleRenderer;
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
    std::shared_ptr< Light >         selectedLight;
	std::shared_ptr< BlockActor >    selectedBlockActor;
	std::shared_ptr< SkeletonActor > selectedSkeletonActor;

    // Debug uchar render target.
    void createUcharDisplayFrame( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device );

    // Needed to display uchar textures using usual texture shader (unorm view is required - integer as 0-1 float).
    std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > > ucharDisplayFrame;

    std::string scenePath;
    std::shared_ptr<CScene> scene;

	// Copying is not allowed.
	Application( const Application& ) = delete;
	Application& operator=( const Application& ) = delete;
};

