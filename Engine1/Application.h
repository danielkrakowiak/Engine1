#pragma once

#include "InputManager.h"
#include "SceneManager.h"
#include "DX11RendererCore.h"
#include "Renderer.h"
#include "DX11FrameRenderer.h"
#include "FreeCamera.h"
#include "Profiler.h"
#include "Benchmark.h"
#include "ImageLibrary.h"
#include "FontLibrary.h"
#include "PhysicsLibrary.h"
#include "AssetManager.h"
#include "ControlPanel.h"
#include "StagingTexture2D.h"
#include "RenderTargetManager.h"
#include "RenderingTester.h"

namespace Engine1
{
    class Scene;
    class Actor;
    class BlockActor;
    class SkeletonActor;
    class Model;

    class Application {
    public:
	    Application();
	    ~Application();

	    void initialize( HINSTANCE applicationInstance );
	    void show();
	    void run();

        void setWindowTitle( const std::string& title );

    protected:

	    static ImageLibrary imageLibrary;
	    static FontLibrary fontLibrary;
        static PhysicsLibrary physicsLibrary;

	    // Initialization
	    bool m_initialized;
	    void setupWindow( );

	    // Windows message handling.
	    static const unsigned int inputTimerId = 1;
	    static const unsigned int inputTimerInterval = 5;
	    static Application* windowsMessageReceiver;

	    static LRESULT CALLBACK windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	    virtual void onStart( );
	    virtual void onExit( );
	    virtual void onResize( int newWidth, int newHeight );
        virtual void onMove( int newPosX, int newPosY );
	    virtual void onFocusChange( bool windowFocused );
        virtual void onKeyPress( int key );
        virtual void onMouseButtonPress( int button );
	    virtual void onDragAndDropFile( std::string filePath, bool replaceSelected );

        virtual bool onFrame( const double frameTimeMs, const bool lockCursor ); // returns: modifyingScene
        virtual void onSelectionChanged();

        struct StageProfilingInfo
        {
            // Time in milliseconds.
            float shadowsTotal;
            float shadingTotal;
        };

        typedef std::array< StageProfilingInfo, (int)RenderingStage::MAX_VALUE > StageProfilingInfos;

        void accumulateStageProfilingData( StageProfilingInfos& stageProfilingInfos );

        void renderActiveViewText( 
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
            Font& font );

        void renderGPUNameText( 
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
            Font& font );

        void renderFPSText( 
            float totalFrameTimeCPU,
            float totalFrameTimeGPU,
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
            Font& font );

        void renderProfilingText( 
            float totalFrameTimeGPU,
            const StageProfilingInfos& stageProfilingInfos,
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
            Font& font );

        void renderSceneStatisticsText( 
            std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
            Font& font );

        void displayFinalFrame( Renderer::Output &output );
        void displayPixelColorAsWindowTitle( Renderer::Output &output, const int2 mousePos );

        void createDebugFrames( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device );

        void debugDisplayTextureValue( const Texture2D< unsigned char >& texture, const int2 screenCoords );
        void debugDisplayTextureValue( const Texture2D< uchar4 >& texture, const int2 screenCoords );
        void debugDisplayTextureValue( const Texture2D< float >& texture, const int2 screenCoords );
        void debugDisplayTextureValue( const Texture2D< float4 >& texture, const int2 screenCoords );
        void debugDisplayTexturesValue( const std::vector< std::shared_ptr< Texture2D< unsigned char > > >& textures, const int2 screenCoords );

        int2 screenPosToWindowPos( int2 screenPos ) const;

        void setupBenchmark1();
        void setupBenchmark2();
        void setupBenchmark3();
        void setupBenchmark4();
        void setupBenchmarkScenes();

	    // Basic application handles.
	    HINSTANCE m_applicationInstance;
	    HWND      m_windowHandle;
	    HDC       m_deviceContext;

        // Window state.
        int2 m_windowPosition;

	    InputManager     m_inputManager;
        AssetManager     m_assetManager;
        SceneManager     m_sceneManager;

	    DX11RendererCore      m_rendererCore;
        DX11FrameRenderer     m_frameRenderer;
        Renderer                  m_renderer;
        Profiler                  m_profiler;
        Benchmark                 m_benchmark;
        RenderTargetManager       m_renderTargetManager;

        ControlPanel m_controlPanel;

        RenderingTester m_renderingTester;

	    bool m_windowFocused;

        // Debug uchar render target.
        void createUcharDisplayFrame( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device );

        // Needed to display uchar textures using usual texture shader (unorm view is required - integer as 0-1 float).
        std::shared_ptr< RenderTargetTexture2D< unsigned char > > ucharDisplayFrame;

        std::shared_ptr< StagingTexture2D< unsigned char > > m_debugFrameU1;
        std::shared_ptr< StagingTexture2D< uchar4 > >        m_debugFrameU4;
        std::shared_ptr< StagingTexture2D< float > >         m_debugFrameF1;
        std::shared_ptr< StagingTexture2D< float4 > >        m_debugFrameF4;

	    // Copying is not allowed.
	    Application( const Application& ) = delete;
	    Application& operator=( const Application& ) = delete;
    };
};

