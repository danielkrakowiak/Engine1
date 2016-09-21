#pragma once

#include "InputManager.h"
#include "Direct3DRendererCore.h"
#include "Renderer.h"
#include "Direct3DFrameRenderer.h"
#include "FreeCamera.h"

#include "ImageLibrary.h"
#include "FontLibrary.h"

#include "AssetManager.h"

#include "StagingTexture2D.h"

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

	bool isFullscreen() { return m_fullscreen; }

	int getScreenWidth() { return m_screenWidth; }
	int getScreenHeight() { return m_screenHeight; }
	int getDisplayFrequency() { return m_displayFrequency; }
	int getScreenColorDepth() { return m_screenColorDepth; }
	int getZBufferDepth() { return m_zBufferDepth; }

    void setWindowTitle( const std::string& title );

private:

	static ImageLibrary imageLibrary;
	static FontLibrary fontLibrary;

	// Initialization
	bool m_initialized;
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

    void loadCamera( std::string path );
    void saveCamera( std::string path );

    void loadScene( std::string path );
    void saveScene( std::string path );

    void debugDisplayTextureValue( const Texture2DGeneric< unsigned char >& texture, const int x, const int y );
    void debugDisplayTextureValue( const Texture2DGeneric< uchar4 >& texture, const int x, const int y );
    void debugDisplayTextureValue( const Texture2DGeneric< float >& texture, const int x, const int y );
    void debugDisplayTextureValue( const Texture2DGeneric< float4 >& texture, const int x, const int y );
    void debugDisplayTexturesValue( const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& textures, const int x, const int y );

	// Basic application handles.
	HINSTANCE m_applicationInstance;
	HWND      m_windowHandle;
	HDC       m_deviceContext;

    // Window state.
    int2 m_windowPosition;

	InputManager m_inputManager;

	Direct3DRendererCore      m_rendererCore;
    Direct3DFrameRenderer     m_frameRenderer;
    Renderer                  m_renderer;

	bool m_fullscreen;
	int  m_screenWidth;
    int  m_screenHeight;
	bool m_verticalSync;
	int  m_displayFrequency;
	char m_screenColorDepth;
	char m_zBufferDepth;

	bool m_windowFocused;

    std::string m_cameraPath;
	FreeCamera m_camera;

    AssetManager m_assetManager;

	// For creation of new assets.
    std::shared_ptr< Light >         m_selectedLight;
	std::shared_ptr< BlockActor >    m_selectedBlockActor;
	std::shared_ptr< SkeletonActor > m_selectedSkeletonActor;

    bool m_debugRenderAlpha;

    std::shared_ptr< StagingTexture2D< unsigned char > > m_debugFrameU1;
    std::shared_ptr< StagingTexture2D< uchar4 > >        m_debugFrameU4;
    std::shared_ptr< StagingTexture2D< float > >         m_debugFrameF1;
    std::shared_ptr< StagingTexture2D< float4 > >        m_debugFrameF4;

    void createDebugFrames( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device );

    // Debug uchar render target.
    void createUcharDisplayFrame( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device );

    // Needed to display uchar textures using usual texture shader (unorm view is required - integer as 0-1 float).
    std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > > ucharDisplayFrame;

    std::string m_scenePath;
    std::shared_ptr<CScene> m_scene;

	// Copying is not allowed.
	Application( const Application& ) = delete;
	Application& operator=( const Application& ) = delete;
};

