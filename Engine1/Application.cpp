#include "Application.h"

#include <sstream>
#include <exception>
#include <iomanip>
#include <algorithm>

#include <Windows.h>

#include "Camera.h"

#include "MathUtil.h"
#include "StringUtil.h"
#include "TextureUtil.h"
#include "MeshUtil.h"
#include "ModelUtil.h"

#include "BlockMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

#include "BlockActor.h"
#include "SkeletonActor.h"

#include "PointLight.h"
#include "SpotLight.h"

#include "Scene.h"

#include "Timer.h"

#include "BVHTree.h"
#include "BVHTreeBuffer.h"

#include "AssetPathManager.h"

// Only for debugging.
#include "BlurShadowsComputeShader.h"
#include "HitDistanceSearchComputeShader.h"

#include "Settings.h"

using namespace Engine1;

namespace /*anonymous*/
{
    void setFullscreen(HWND windowHandle)
    {
        RECT windowRect;
        // Store the current window dimensions so they can be restored 
        // when switching out of fullscreen state.
        ::GetWindowRect(windowHandle, &windowRect);

        // Set the window style to a borderless window so the client area fills
        // the entire screen.
        UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

        ::SetWindowLongW(windowHandle, GWL_STYLE, windowStyle);

        // Query the name of the nearest display device for the window.
        // This is required to set the fullscreen dimensions of the window
        // when using a multi-monitor setup.
        HMONITOR hMonitor = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        ::GetMonitorInfo(hMonitor, &monitorInfo);

        ::SetWindowPos(windowHandle, HWND_TOP,
            monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.top,
            monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
            monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ::ShowWindow(windowHandle, SW_MAXIMIZE);
    }

    void setWindowed(HWND windowHandle, int2 position, int2 dimensions)
    {
        // Restore all the window decorators.
        ::SetWindowLong(windowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);

        ::SetWindowPos(windowHandle, HWND_NOTOPMOST,
            position.x,
            position.y,
            dimensions.x,
            dimensions.y,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ::ShowWindow(windowHandle, SW_NORMAL);
    }
}

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;

using Microsoft::WRL::ComPtr;

Application::Application() :
	m_frameRenderer( m_dx11RendererCore, m_profiler ),
    m_renderer( m_dx11RendererCore, m_profiler, m_renderTargetManager ),
	m_initialized( false ),
	m_applicationInstance( nullptr ),
	m_windowHandle( nullptr ),
	m_deviceContext( nullptr ),
    m_windowPosition( 0, 0 ),
	m_windowFocused( false ),
    m_assetManager(),
    m_sceneManager( m_assetManager ),
    m_controlPanel( m_sceneManager, m_assetManager, m_renderingTester ),
    m_benchmark( m_sceneManager, m_profiler ),
    m_renderingTester( m_sceneManager, m_assetManager, m_dx11RendererCore, m_renderer )
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->m_applicationInstance = applicationInstance;

	setupWindow();

    const int parallelThreadCount = std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) : 1;

	m_frameRenderer.initialize( m_windowHandle, settings().main.screenDimensions.x, settings().main.screenDimensions.y, settings().main.fullscreen, settings().main.verticalSync );

    auto device        = m_frameRenderer.getDevice();
    auto deviceContext = m_frameRenderer.getDeviceContext();

    Settings::modify().initialize( *device.Get() );

	m_dx11RendererCore.initialize( *deviceContext.Get() );
    m_dx12RendererCore.initialize( m_windowHandle );

    m_assetManager.initialize( parallelThreadCount, parallelThreadCount, device );
    m_profiler.initialize( device, deviceContext );
    m_renderTargetManager.initialize( device );

    m_sceneManager.initialize( device, deviceContext );

    createUcharDisplayFrame( settings().main.screenDimensions.x, settings().main.screenDimensions.y, device );
    createDebugFrames( settings().main.screenDimensions.x, settings().main.screenDimensions.y, device );

    // Load 'light source' model.
	std::shared_ptr<BlockModel> lightModel;
	try
	{
        const auto path = AssetPathManager::get().getPathForFileName( "light_bulb.blockmodel" );
		BlockModelFileInfo lightModelFileInfo( path, BlockModelFileInfo::Format::BLOCKMODEL, 0);
		lightModel = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( lightModelFileInfo ) );
		lightModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext().Get() );
	}
	catch (...) {}

    m_renderer.initialize( 
        settings().main.screenDimensions, 
        device, 
        deviceContext, 
        lightModel 
    );

    m_renderingTester.initialize();

    m_controlPanel.initialize( device, settings().main.screenDimensions );

	m_initialized = true;

    onStart();
}

void Application::createUcharDisplayFrame( int imageWidth, int imageHeight, ComPtr< ID3D11Device3 > device )
{
    ucharDisplayFrame = std::make_shared< RenderTargetTexture2D< unsigned char > >
        ( *device.Get(), imageWidth, imageHeight, false, true, false, 
			DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM );
}

void Application::setupWindow() 
{
	const LPCTSTR className = L"Engine1";
	const LPCTSTR wndCaption = L"Engine1";

	WNDCLASSEX wc;
	int style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	ZeroMemory( &wc, sizeof( WNDCLASSEX ) );
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = m_applicationInstance;
	wc.lpfnWndProc = Application::windowsMessageHandler;
	wc.hbrBackground = (HBRUSH)( COLOR_WINDOW );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hIconSm = wc.hIcon;
	wc.lpszClassName = className;
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClassEx( &wc );

	if ( settings().main.fullscreen ) {
		DEVMODE screen = { 0 };

		screen.dmSize             = sizeof( DEVMODE );
		screen.dmPelsWidth        = settings().main.screenDimensions.x;
		screen.dmPelsHeight       = settings().main.screenDimensions.y;
		screen.dmBitsPerPel       = settings().main.screenColorDepth;
		screen.dmDisplayFrequency = settings().main.displayFrequency;
		screen.dmFields           = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		ChangeDisplaySettings( &screen, CDS_FULLSCREEN );

		style |= WS_POPUP;
	} else {
		style |= WS_OVERLAPPEDWINDOW;
	}

	DWORD exStyle = WS_EX_ACCEPTFILES; // Allow drag&drop files.

    int windowWidth  = settings().main.screenDimensions.x;
    int windowHeight = settings().main.screenDimensions.y;
    if ( !settings().main.fullscreen )
    {
        RECT windowArea;
        windowArea.left   = 0;
        windowArea.top    = 0;
        windowArea.right  = settings().main.screenDimensions.x;
        windowArea.bottom = settings().main.screenDimensions.y;

        // Calculate the required window dimensions to accommodate the desried client area.
        AdjustWindowRect( &windowArea, style, false );

        windowWidth  = windowArea.right - windowArea.left;
        windowHeight = windowArea.bottom - windowArea.top;
    }

	m_windowHandle = CreateWindowEx( exStyle, className, wndCaption, style, 0, 0, windowWidth, windowHeight, NULL, NULL, m_applicationInstance, NULL );

	m_deviceContext = GetDC( m_windowHandle );

	int pixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof( PIXELFORMATDESCRIPTOR ),
		1,
		PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		(BYTE)settings().main.screenColorDepth,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)settings().main.zBufferDepth,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	pixelFormat = ChoosePixelFormat( m_deviceContext, &pfd );
	SetPixelFormat( m_deviceContext, pixelFormat, &pfd );
}

void Application::show() 
{
	if ( !m_initialized ) 
        throw std::exception( "Application::show called on uninitialized Application" );

	ShowWindow( m_windowHandle, SW_SHOW );
}

void Application::run() 
{
	if ( !m_initialized ) throw std::exception( "Application::run called on uninitialized Application" );

    /////////////////////////////////////////////////////////////////////////////////
    /*for (auto i = 0; i < 10000; ++i)
    {
        m_dx12RendererCore.render(false);
    }
    return;*/
    //////////////////////////////////////////////////////////////////////////////////////

	bool run = true;
	MSG msg;

	Font font( settings().main.screenDimensions );
	font.loadFromFile( AssetPathManager::get().getPathForFileName( "consola.ttf" ), 35 );

    Font font2( settings().main.screenDimensions );
    font2.loadFromFile( AssetPathManager::get().getPathForFileName( "consola.ttf" ), 13 );

    // Profiling.
    const float profilingDisplayRefreshDelayMs = 200.0f;
    double frameTimeMs = 0.0;
    float totalFrameTimeGPU = 0.0f, totalFrameTimeCPU = 0.0f;
    StageProfilingInfos stageProfilingInfos;

    Timer profilingLastRefreshTime;

    // Used to deduce when to progress physics animation.
    float physicsTimeAccumulator = 0.0f;
    bool physicsStepFinished = true;

    m_renderer.renderShadowMaps( *m_sceneManager.getScene() );

    //onDragAndDropFile( AssetPathManager::get().getPathForFileName("benchmark4 - floor.scene"), false );
    //onDragAndDropFile( AssetPathManager::get().getPathForFileName("benchmark4 - floor.camera"), false );
    //Settings::modify().debug.renderFps = false;
    //Settings::modify().debug.renderText = false;

    setupBenchmark4();

    bool updateProfiling = true;

	while ( run ) {
		Timer frameStartTime;

        if ( Timer::getElapsedTime( frameStartTime, profilingLastRefreshTime ) >= profilingDisplayRefreshDelayMs ) 
        {
            updateProfiling = true;
            profilingLastRefreshTime.reset();
            m_profiler.beginFrameProfiling();
        }

        m_profiler.beginEvent( Profiler::GlobalEventType::Frame );

		while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( WM_QUIT == msg.message ) 
                run = false;
		}

        if ( !physicsStepFinished )
            physicsStepFinished = PhysicsLibrary::getScene().fetchResults( false );

        physicsTimeAccumulator += (float)(frameTimeMs * 1000.0);
        if ( physicsTimeAccumulator > settings().physics.fixedStepDuration && physicsStepFinished )
        {
            PhysicsLibrary::getScene().simulate( settings().physics.fixedStepDuration );
            
            physicsStepFinished = false;
            physicsTimeAccumulator -= settings().physics.fixedStepDuration;
        }

        bool modifyingScene = false;

        // Disable locking when connecting through Team Viewer.
        bool lockCursor = false;

        modifyingScene = onFrame( frameTimeMs, lockCursor );

        //if ( modifyingScene )
        //    m_renderer.renderShadowMaps( *m_sceneManager.getScene() );

        m_profiler.beginEvent( Profiler::GlobalEventType::RenderSceneToFrame );

        Renderer::Output output;
        output = m_renderer.renderScene( 
            *m_sceneManager.getScene(), 
            *m_sceneManager.getCamera(), 
            settings().debug.debugWireframeMode, 
            m_sceneManager.getSelection(), 
            m_sceneManager.getSelectionVolumeMesh() 
        );

        m_profiler.endEvent( Profiler::GlobalEventType::RenderSceneToFrame );

        const int2 mousePos = m_inputManager.getMousePos();

        if ( m_inputManager.isMouseButtonPressed( 0 ) ) {
            displayPixelColorAsWindowTitle( output, mousePos );
        }

        if ( updateProfiling )
        {
            profilingLastRefreshTime.reset();

            { // Accumulate some profiling results.
                totalFrameTimeCPU = (float)frameTimeMs;
                totalFrameTimeGPU = m_profiler.getEventDuration( Profiler::GlobalEventType::Frame );

                accumulateStageProfilingData( stageProfilingInfos );
            }
        }

        m_profiler.beginEvent( Profiler::GlobalEventType::RenderTextToFrame );

        auto renderTarget = output.uchar4Image;

        if ( renderTarget )
        {
            renderActiveViewText( renderTarget, font2 );
            renderGPUNameText( renderTarget, font2 );
            renderFPSText( totalFrameTimeCPU, totalFrameTimeGPU, renderTarget, font );
            renderProfilingText( totalFrameTimeGPU, stageProfilingInfos, renderTarget, font2 );
            renderSceneStatisticsText( renderTarget, font2 );
        }

        m_profiler.endEvent( Profiler::GlobalEventType::RenderTextToFrame );
        m_profiler.beginEvent( Profiler::GlobalEventType::RenderFrameToScreen );

        displayFinalFrame( output );

        m_profiler.endEvent( Profiler::GlobalEventType::RenderFrameToScreen );
        m_profiler.beginEvent( Profiler::GlobalEventType::RenderControlPanelToScreen );

        m_controlPanel.draw();

        m_profiler.endEvent( Profiler::GlobalEventType::RenderControlPanelToScreen );

		m_frameRenderer.displayFrame();

        m_profiler.endEvent( Profiler::GlobalEventType::Frame );
        m_profiler.endFrameProfiling();

        updateProfiling = false;
        m_profiler.pauseProfiling();

		Timer frameEndTime;
		frameTimeMs = Timer::getElapsedTime( frameEndTime, frameStartTime );

        m_benchmark.onFrameEnd();

        if (settings().main.limitFPS)
        {
            const double targetFrameTimeMs = 1000.0 / 10.0;
            const long sleepTime = (long)std::max( 0.0, targetFrameTimeMs - frameTimeMs );
            std::this_thread::sleep_for( std::chrono::milliseconds(sleepTime) );

            Timer frameDelayedEndTime;
            frameTimeMs = Timer::getElapsedTime( frameDelayedEndTime, frameStartTime );
        }
	}
}

void Application::accumulateStageProfilingData( StageProfilingInfos& stageProfilingInfos )
{
    for ( int stage = (int)RenderingStage::Main; stage < pow( 2, settings().rendering.reflectionsRefractions.maxLevel + 1 ) && stage < (int)RenderingStage::MAX_VALUE; ++stage )
    {
        stageProfilingInfos[ stage ].shadowsTotal = 0.0f;
        stageProfilingInfos[ stage ].shadingTotal = 0.0f;

        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            const float shadowPerLight  = m_profiler.getEventDuration( (RenderingStage)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
            const float shadingPerLight = m_profiler.getEventDuration( (RenderingStage)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
                        
            if ( shadowPerLight >= 0.0f )
                stageProfilingInfos[ stage ].shadowsTotal += shadowPerLight;

            if ( shadingPerLight )
                stageProfilingInfos[ stage ].shadingTotal += shadingPerLight;
        }
    }
}

void Application::renderActiveViewText( 
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
    Font& font )
{
    if ( !settings().debug.renderText )
        return;

    std::stringstream ss;

    ss << "Active view type: " + Renderer::viewToString( m_renderer.getActiveViewType() );

    ss << "\n\n";

    m_renderer.renderText( 
        ss.str(), 
        font, 
        float2( 150.0f, 300.0f ), 
        float4( 1.0f, 1.0f, 1.0f, 1.0f ),
        renderTarget
    );
}

void Application::renderGPUNameText( 
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
    Font& font )
{
    if ( !settings().debug.renderFps )
        return;

    std::stringstream ss;
    ss << "GPU: " << m_frameRenderer.getGPUName();
            
    m_renderer.renderText( 
        ss.str(), 
        font, 
        float2( -496.0f, 335.0f ), 
        float4( 1.0f, 1.0f, 1.0f, 1.0f ),
        renderTarget
    );
}

void Application::renderFPSText( 
    float totalFrameTimeCPU,
    float totalFrameTimeGPU,
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
    Font& font )
{
    if ( !settings().debug.renderFps )
        return;

    std::stringstream ss;
    ss << "FPS: " << (int)( 1000.0 / totalFrameTimeCPU ) << " / " << totalFrameTimeCPU << "ms";
            
    // Render GPU FPS only if it is significantly higher than CPU FPS - to warn about CPU bottleneck.
    // Note: GPU FPS is slightly delayed (3-4 frames of delay), so may cause fake warnings when sudden frame drop occurs.
    if ( totalFrameTimeGPU < (0.98f * totalFrameTimeCPU) ) {
        ss << "\nGPU FPS: " << (int)( 1000.0 / totalFrameTimeGPU ) << " / " << totalFrameTimeGPU << "ms";
    }
            
    m_renderer.renderText( 
        ss.str(), 
        font, 
        float2( -500.0f, 300.0f ), 
        float4( 1.0f, 1.0f, 1.0f, 1.0f ),
        renderTarget
    );
}

void Application::renderProfilingText( 
    float totalFrameTimeGPU,
    const StageProfilingInfos& stageProfilingInfos,
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
    Font& font )
{
    if ( !settings().profiling.display.enabled ) 
        return;

    std::stringstream ss, ss2;
    ss << std::fixed << std::setprecision( 2 );
    ss2 << std::fixed << std::setprecision( 2 );
    ss << "Profiling: \n";
    ss << "Total: " << totalFrameTimeGPU << " ms \n";

    const float colorRedForDurationMs = 1.0f;

    // Print global events duration.
    for (int globalEventType = (int)Profiler::GlobalEventType::DeferredRendering; globalEventType < (int)Profiler::GlobalEventType::MAX_VALUE; ++globalEventType)
    {
        const std::string eventName     = Profiler::eventTypeToString( (Profiler::GlobalEventType)globalEventType );
        const float       eventDuration = m_profiler.getEventDuration( (Profiler::GlobalEventType)globalEventType );

        float greenBlueColor = 1.0f - std::min(1.0f, eventDuration / colorRedForDurationMs );
        ss << "<color " << 1.0f << "," << greenBlueColor << "," << greenBlueColor << ">";

        ss << eventName << ": " << eventDuration << "ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

        ss << "<color/>";
    }

    for ( int stage = (int)settings().profiling.display.startWithStage; stage < pow( 2, settings().rendering.reflectionsRefractions.maxLevel + 1 ) && stage < (int)RenderingStage::MAX_VALUE; ++stage )
    {
        // Print stage name.
        const std::string stageName = renderingStageToString( (RenderingStage)stage );
        ss << stageName << "\n";

        // Print total duration of shadow calculations.
        const float totalShadowsDuration = stageProfilingInfos[ stage ].shadowsTotal;
        if ( totalShadowsDuration > 0.0f )
            ss << "Total shadows: " << totalShadowsDuration << "ms " << ( totalShadowsDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

        // Print total duration of shading calculations.
        const float totalShadingDuration = stageProfilingInfos[ stage ].shadingTotal;
        if ( totalShadingDuration > 0.0f )
            ss << "Total shading: " << totalShadingDuration << "ms " << ( totalShadingDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

        // Print duration of events occurring at each stage.
        for ( int eventType = (int)Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals; eventType < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventType )
        {
            const float       eventDuration = m_profiler.getEventDuration( (RenderingStage)stage, (Profiler::EventTypePerStage)eventType );
            const std::string eventName     = Profiler::eventTypeToString( (Profiler::EventTypePerStage)eventType );

            if ( eventDuration >= 0.0f )
            {
                float greenBlueColor = 1.0f - std::min(1.0f, eventDuration / colorRedForDurationMs );
                ss << "<color " << 1.0f << "," << greenBlueColor << "," << greenBlueColor << ">";

                ss << "    " << eventName << ": " << eventDuration << " ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

                ss << "<color/>";
            }
        }

        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx )
        {
            bool display = false;
            ss2.str( "" ); // Clear stream.
            ss2.clear();   // Clear stream errors.

            ss2 << "        Light " << lightIdx << "\n";

            for ( int eventType = (int)Profiler::EventTypePerStagePerLight::ShadowsMapping; eventType < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventType )
            {
                const float       eventDuration = m_profiler.getEventDuration( ( RenderingStage)stage, lightIdx, ( Profiler::EventTypePerStagePerLight )eventType );
                const std::string eventName     = Profiler::eventTypeToString( ( Profiler::EventTypePerStagePerLight )eventType );

                if ( eventDuration > 0.0f )
                {
                    float greenBlueColor = 1.0f - std::min(1.0f, eventDuration / colorRedForDurationMs );
                    ss2 << "<color " << 1.0f << "," << greenBlueColor << "," << greenBlueColor << ">";

                    ss2 << "                " << eventName << ": " << eventDuration << " ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "%\n";

                    ss2 << "<color/>";
                    display = true;
                }
            }

            ss2 << "\n";

            // If accumulated events duration for that light is non-zero - display it's info.
            if ( display )
                ss << ss2.str();
        }
    }

    m_renderer.renderText( 
        ss.str(), 
        font, 
        float2( -500.0f, 250.0f ), 
        float4( 1.0f, 1.0f, 1.0f, 1.0f ),
        renderTarget
    );
}

void Application::renderSceneStatisticsText( 
    std::shared_ptr< RenderTargetTexture2D< uchar4 > > renderTarget, 
    Font& font )
{
    // Render scene stats and selection stats.
    if ( !settings().debug.renderText )
        return;

    int selectedVertexCount = 0;
    int selectedTriangleCount = 0;
    int selectedMeshesCount = (int)(m_sceneManager.getSelectedBlockActors().size() + m_sceneManager.getSelectedSkeletonActors().size());
    int selectedLightsCount = (int)m_sceneManager.getSelectedLights().size();
    int totalVertexCount = 0;
    int totalTriangleCount = 0;
    int totalActors = (int)m_sceneManager.getScene()->getActors().size();


    std::tie( selectedVertexCount, selectedTriangleCount ) = m_sceneManager.getSelectedActorsVertexAndTriangleCount();
    std::tie( totalVertexCount, totalTriangleCount ) = m_sceneManager.getSceneVertexAndTriangleCount();

    std::stringstream ss;
    ss << "Selected: " << selectedVertexCount << " verts / " << selectedTriangleCount << " tris " << selectedMeshesCount << " actors \n";
    ss << "Scene:    " << totalVertexCount << " verts / " << totalTriangleCount << " tris " << totalActors << " actors \n";

    if ( selectedMeshesCount >= 1 )
    {
        std::string modelPath, meshPath;
        int bvhNodes = 0, bvhNodesExtents = 0, bvhTriangles = 0;

        if ( !m_sceneManager.getSelectedBlockActors().empty() &&
            m_sceneManager.getSelectedBlockActors()[ 0 ]->getModel() &&
            m_sceneManager.getSelectedBlockActors()[ 0 ]->getModel()->getMesh() )
        {
            const auto& model = m_sceneManager.getSelectedBlockActors()[ 0 ]->getModel();

            modelPath = model->getFileInfo().getPath();
            meshPath = model->getMesh()->getFileInfo().getPath();

            if ( model->getMesh()->getBvhTree() )
            {
                bvhNodes = (int)model->getMesh()->getBvhTree()->getNodes().size();
                bvhNodesExtents = (int)model->getMesh()->getBvhTree()->getNodesExtents().size();
                bvhTriangles = (int)model->getMesh()->getBvhTree()->getTriangles().size();
            }
        }
        else if ( !m_sceneManager.getSelectedSkeletonActors().empty() &&
            m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel() &&
            m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel()->getMesh() )
        {
            const auto& model = m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel();

            modelPath = model->getFileInfo().getPath();
            meshPath = model->getMesh()->getFileInfo().getPath();
        }

        const size_t modelPathStartIndex = modelPath.rfind( "\\" );
        const size_t meshPathStartIndex = meshPath.rfind( "\\" );

        modelPath = modelPath.substr( modelPathStartIndex != std::string::npos ? modelPathStartIndex : 0 );
        meshPath = meshPath.substr( meshPathStartIndex != std::string::npos ? meshPathStartIndex : 0 );

        ss << modelPath << "\n" << meshPath << "\n";

        ss << "BVH nodes: " << bvhNodes << ", extents: " << bvhNodesExtents << " triangles: " << bvhTriangles;
    }

    if ( selectedLightsCount >= 1 && selectedMeshesCount == 0 )
    {
        auto& light = m_sceneManager.getSelectedLights().front();

        if ( light->getType() == Light::Type::PointLight )
        {
            ss << "Point light - emitter radius: " << light->getEmitterRadius() << "\n"
                << ", color: (" << light->getColor().x << ", " << light->getColor().y << ", " << light->getColor().z << "), " << "\n"
                << (light->isCastingShadows() ? "casts shadow" : "does not cast shadow");
        }
        else if ( light->getType() == Light::Type::SpotLight )
        {
            SpotLight& spotLight = static_cast<SpotLight&>(*light);

            ss << "Spot light - emitter radius: " << light->getEmitterRadius() << "\n"
                << ", color: (" << light->getColor().x << ", " << light->getColor().y << ", " << light->getColor().z << "), " << "\n"
                << (light->isCastingShadows() ? "casts shadow" : "does not cast shadow")
                << ", cone angle: " << MathUtil::radiansToDegrees( spotLight.getConeAngle() ) << " deg.";
        }
    }

    if ( m_sceneManager.isSelectionEmpty() )
    {
        ss << "\nrendering.combining.positionDiffMul: " << settings().rendering.combining.positionDiffMul << " [C + P]";
        ss << "\nrendering.combining.normalDiffMul: " << settings().rendering.combining.normalDiffMul << " [C + N]";
        ss << "\nrendering.combining.positionNormalThreshold: " << settings().rendering.combining.positionNormalThreshold << " [C + T]";
        ss << "\nHitDistanceSearchComputeShader::s_positionDiffMul: " << HitDistanceSearchComputeShader::s_positionDiffMul << " [R + P]";
        ss << "\nHitDistanceSearchComputeShader::s_normalDiffMul: " << HitDistanceSearchComputeShader::s_normalDiffMul << " [R + N]";
        ss << "\nHitDistanceSearchComputeShader::s_positionNormalThreshold: " << HitDistanceSearchComputeShader::s_positionNormalThreshold << " [R + T]";
        ss << "\nHitDistanceSearchComputeShader::s_minSampleWeightBasedOnDistance: " << HitDistanceSearchComputeShader::s_minSampleWeightBasedOnDistance << " [R + W]";

        const float screenPixelCount = (float)(settings().main.screenDimensions.x * settings().main.screenDimensions.y);
        const float bytesInMegabyte = 1024.0f * 1024.0f;

        const int renderTargetFloat4Count = m_renderTargetManager.getTotalRenderTargetCount< float4 >();
        const int renderTargetFloatCount = m_renderTargetManager.getTotalRenderTargetCount< float >();
        const int renderTargetUchar4Count = m_renderTargetManager.getTotalRenderTargetCount< uchar4 >();
        const int renderTargetUcharCount = m_renderTargetManager.getTotalRenderTargetCount< unsigned char >();
        const int renderTargetUchar4DepthCount = m_renderTargetManager.getTotalRenderTargetDepthCount();

        ss << "\n\nRender target usage:";
        ss << "\n float4: " << renderTargetFloat4Count << ", approx.: " << (float)renderTargetFloat4Count * (screenPixelCount * 16.0f) / bytesInMegabyte << " MB";
        ss << "\n float:  " << renderTargetFloatCount << ", approx.: " << (float)renderTargetFloatCount * (screenPixelCount * 4.0f) / bytesInMegabyte << " MB";
        ss << "\n uchar4: " << renderTargetUchar4Count << ", approx.: " << (float)renderTargetUchar4Count * (screenPixelCount * 4.0f) / bytesInMegabyte << " MB";
        ss << "\n uchar:  " << renderTargetUcharCount << ", approx.: " << (float)renderTargetUcharCount * (screenPixelCount * 1.0f) / bytesInMegabyte << " MB";
        ss << "\n uchar4 depth: " << renderTargetUchar4DepthCount << ", approx.: " << (float)renderTargetUchar4DepthCount * (screenPixelCount * 4.0f) / bytesInMegabyte << " MB";
    }

    // Print model textures.
    if ( m_sceneManager.getSelection().containsOnlyOneBlockActor() )
    {
        ss << "\n\nModel textures:";

        BlockActor& selectedBlockActor = *m_sceneManager.getSelection().getBlockActors().front();

        for ( int textureType = 0; textureType < (int)Model::TextureType::COUNT; ++textureType )
        {
            const auto textures = selectedBlockActor.getModel()->getTextures( (Model::TextureType)textureType );
            for ( auto& texture : textures )
            {
                const auto& tex = std::get< 0 >( texture );

                if ( tex && !tex->getFileInfo().getPath().empty() )
                    ss << "\n" << tex->getFileInfo().getPath();
            }
        }
    }

    { // Render animation details.
        ss << "\n\nCamera anim keyframes: " << m_sceneManager.getCameraAnimator().getKeyframeCount( m_sceneManager.getCamera() );

        if ( m_sceneManager.getSelection().getBlockActors().size() == 1 ) {
            ss << "\nActor anim keyframes: " << m_sceneManager.getActorAnimator().getKeyframeCount( m_sceneManager.getSelectedBlockActors().front() );
        }

        if ( m_sceneManager.getSelection().getSpotLights().size() == 1 ) {
            ss << "\nLight anim keyframes: " << m_sceneManager.getLightAnimator().getKeyframeCount( m_sceneManager.getSelection().getSpotLights().front() );
        }
    }

    { // Render camera state.
        //std::stringstream ss;
        //ss << "Cam pos: " << camera.getPosition( ).x << ", " << camera.getPosition( ).y << ", " << camera.getPosition( ).z;
        //deferredRenderer.render( ss.str( ), font2, float2( -500.0f, 200.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    }

    m_renderer.renderText(
        ss.str(),
        font,
        float2( 0.0f, 200.0f ),
        float4::ONE,
        renderTarget
    );
}

void Application::displayFinalFrame( Renderer::Output &output )
{
    try
    {
        if ( output.ucharImage )
        {
            m_dx11RendererCore.copyTextureGpu(
                *ucharDisplayFrame, 0u, *output.ucharImage, 0u
            );

            m_frameRenderer.renderTexture(
                *output.ucharImage, 0.0f, 0.0f,
                (float)settings().main.screenDimensions.x,
                (float)settings().main.screenDimensions.y,
                false,
                settings().debug.debugDisplayedMipmapLevel
            );
        }
        else if ( output.uchar4Image )
        {
            if ( settings().debug.debugRenderAlpha )
            {
                m_frameRenderer.renderTextureAlpha(
                    *output.uchar4Image, 0.0f, 0.0f,
                    (float)settings().main.screenDimensions.x,
                    (float)settings().main.screenDimensions.y,
                    false,
                    settings().debug.debugDisplayedMipmapLevel
                );
            }
            else
            {
                m_frameRenderer.renderTexture(
                    *output.uchar4Image, 0.0f, 0.0f,
                    (float)settings().main.screenDimensions.x,
                    (float)settings().main.screenDimensions.y,
                    false,
                    settings().debug.debugDisplayedMipmapLevel
                );
            }
        }
        else if ( output.float4Image )
        {
            m_frameRenderer.renderTexture(
                *output.float4Image, 0.0f, 0.0f,
                (float)settings().main.screenDimensions.x,
                (float)settings().main.screenDimensions.y,
                false,
                settings().debug.debugDisplayedMipmapLevel
            );
        }
        else if ( output.float2Image )
        {
            m_frameRenderer.renderTexture(
                *output.float2Image, 0.0f, 0.0f,
                (float)settings().main.screenDimensions.x,
                (float)settings().main.screenDimensions.y,
                false,
                settings().debug.debugDisplayedMipmapLevel
            );
        }
        else if ( output.floatImage )
        {
            m_frameRenderer.renderTexture(
                *output.floatImage, 0.0f, 0.0f,
                (float)settings().main.screenDimensions.x,
                (float)settings().main.screenDimensions.y,
                false,
                settings().debug.debugDisplayedMipmapLevel
            );
        }
    }
    catch ( ... )
    {
    }
}

void Application::displayPixelColorAsWindowTitle( Renderer::Output &output, const int2 mousePos )
{
    try
    {
        if ( output.ucharImage ) {
            debugDisplayTextureValue( *output.ucharImage, mousePos );
        } else if ( output.uchar4Image ) {
            debugDisplayTextureValue( *output.uchar4Image, mousePos );
        } else if ( output.float4Image ) {
            debugDisplayTextureValue( *output.float4Image, mousePos );
        } else if ( output.float2Image ) {
            // #TODO: Implement.
        } else if ( output.floatImage ) {
            debugDisplayTextureValue( *output.floatImage, mousePos );
        }

        if ( m_renderer.getActiveViewType() == Renderer::View::CurrentRefractiveIndex ) {
            debugDisplayTexturesValue( m_renderer.debugGetCurrentRefractiveIndexTextures(), mousePos );
        }
    }
    catch ( ... )
    {
    }
}

void Application::setWindowTitle( const std::string& title )
{
    SetWindowText( m_windowHandle, StringUtil::widen( title ).c_str() );
}

LRESULT CALLBACK Application::windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
    if (windowsMessageReceiver)
    {
        const int result = windowsMessageReceiver->m_controlPanel.processInput( hWnd, msg, wParam, lParam );

        if ( result )
            return 0; // Message handled by the Control Panel.
    }

	switch (msg) {
	case WM_CREATE:
		SetTimer(hWnd, inputTimerId, inputTimerInterval, 0);

		break;

	case WM_DESTROY:
		KillTimer(hWnd, inputTimerId);

		if (windowsMessageReceiver) 
            windowsMessageReceiver->onExit();

		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (windowsMessageReceiver) 
        {
			int newWidth = LOWORD(lParam);
			int newHeight = HIWORD(lParam);
			windowsMessageReceiver->onResize(newWidth, newHeight);
		}
		break;
	case WM_MOVE:
		if (windowsMessageReceiver) 
        {
			int posX = LOWORD(lParam);
			int posY = HIWORD(lParam);
			windowsMessageReceiver->onMove(posX, posY);
		}
		break;
	case WM_SETFOCUS:
		if (windowsMessageReceiver) 
            windowsMessageReceiver->onFocusChange(true);
		break;
	case WM_KILLFOCUS:
		if (windowsMessageReceiver) 
            windowsMessageReceiver->onFocusChange(false);
		break;
	case WM_KEYDOWN:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onKeyboardButton((int)wParam, true);
			windowsMessageReceiver->onKeyPress((int)wParam);
		}
		break;
	case WM_KEYUP:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onKeyboardButton((int)wParam, false);
		}
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(0, true);
			windowsMessageReceiver->onMouseButtonPress(0);
		}
		break;
	case WM_LBUTTONUP:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(0, false);
		}
		break;
	case WM_MBUTTONDOWN:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(1, true);
			windowsMessageReceiver->onMouseButtonPress(1);
		}
		break;
	case WM_MBUTTONUP:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(1, false);
		}
		break;
	case WM_RBUTTONDOWN:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(2, true);
			windowsMessageReceiver->onMouseButtonPress(2);
		}
		break;
	case WM_RBUTTONUP:
		if (windowsMessageReceiver) 
        {
			windowsMessageReceiver->m_inputManager.onMouseButton(2, false);
		}
		break;
	case WM_TIMER:
		//if ( inputTimerId == wParam ) {
		//	if ( windowsMessageReceiver ) {
		//		windowsMessageReceiver->inputManager.updateMouseState();
		//	}
		//}
		break;
	case WM_DROPFILES:
    {
		HDROP dropInfo = (HDROP)wParam;
        const int fileCount = (int)DragQueryFileW(dropInfo, (UINT)-1, nullptr, 0);

        for ( int fileIdx = 0; fileIdx < fileCount; ++fileIdx )
        {
            const DWORD charCount = DragQueryFileW( dropInfo, fileIdx, nullptr, 0 ) + 1;
            std::vector<wchar_t> pathBufferW;
            pathBufferW.resize( charCount );

            DragQueryFileW( dropInfo, fileIdx, (LPWSTR)pathBufferW.data(), charCount );
            std::wstring pathW( pathBufferW.data(), charCount - 1 );
            std::string path = StringUtil::narrow( pathW );

			const bool replaceAssets = settings().debug.replaceSelected;

            try {
                windowsMessageReceiver->onDragAndDropFile( path, replaceAssets );
            } catch ( ... ) {}
        }
			
		break;
    }

    // The default window procedure will play a system notification sound 
    // when pressing the Alt+Enter keyboard combination if this message is 
    // not handled.
    case WM_SYSCHAR:
        return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

void Application::onStart( ) 
{
}

void Application::onExit( ) 
{
}

void Application::onResize( int newWidth, int newHeight ) 
{
    // Unused.    
    newWidth;
    newHeight;
}

void Application::onMove( int newPosX, int newPosY )
{
    m_windowPosition.x = newPosX;
    m_windowPosition.y = newPosY;
}

void Application::onFocusChange( bool windowFocused )
{
	m_windowFocused = windowFocused;

    Settings::modify().main.limitFPS = !windowFocused;
}

void Application::onKeyPress( int /*key*/ )
{
}

void Application::onMouseButtonPress( int /*button*/ )
{
}

void Application::onDragAndDropFile( std::string /*filePath*/, bool /*replaceSelected*/ )
{
}

bool Application::onFrame( const double /*frameTimeMs*/, const bool /*lockCursor*/ )
{
    return false;
}

void Application::onSelectionChanged()
{
}

void Application::createDebugFrames( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device )
{
    m_debugFrameU1 = std::make_shared< StagingTexture2D< unsigned char > >
        ( *device.Get(), imageWidth, imageHeight, DXGI_FORMAT_R8_UINT );

    m_debugFrameU4 = std::make_shared< StagingTexture2D< uchar4 > >
        ( *device.Get(), imageWidth, imageHeight, DXGI_FORMAT_R8G8B8A8_UINT );

    m_debugFrameF1 = std::make_shared< StagingTexture2D< float > >
        ( *device.Get(), imageWidth, imageHeight, DXGI_FORMAT_R32_FLOAT );

    m_debugFrameF4 = std::make_shared< StagingTexture2D< float4 > >
        ( *device.Get(), imageWidth, imageHeight, DXGI_FORMAT_R32G32B32A32_FLOAT );
}

void Application::debugDisplayTextureValue( const Texture2D< unsigned char >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_dx11RendererCore.copyTextureGpu( *m_debugFrameU1, texture, textureCoords, int2::ONE );
    m_debugFrameU1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const unsigned char pixelColor = m_debugFrameU1->getPixel( textureCoords );

    const float floatVal = (float)pixelColor / 255.0f;
    setWindowTitle( "uchar: " + std::to_string( pixelColor ) + ", float: " + std::to_string( floatVal ) + ", ior: " + std::to_string( 1.0f + floatVal * 2.0f ) );
}

void Application::debugDisplayTextureValue( const Texture2D< uchar4 >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_dx11RendererCore.copyTextureGpu( *m_debugFrameU4, texture, textureCoords, int2::ONE );
    m_debugFrameU4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const uchar4 pixelColor = m_debugFrameU4->getPixel( textureCoords );

    const float4 floatVal = float4( (float4)pixelColor / 255.0f );
    setWindowTitle( "uchar: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")"
                    + ", float: (" + std::to_string( floatVal.x ) + ", " + std::to_string( floatVal.y ) + ", " + std::to_string( floatVal.z ) + ", " + std::to_string( floatVal.w ) + ")" );
}

void Application::debugDisplayTextureValue( const Texture2D< float >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_dx11RendererCore.copyTextureGpu( *m_debugFrameF1, texture, textureCoords, int2::ONE );
    m_debugFrameF1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const float pixelColor = m_debugFrameF1->getPixel( textureCoords );

    setWindowTitle( "float: " + std::to_string( pixelColor ) );
}

void Application::debugDisplayTextureValue( const Texture2D< float4 >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_dx11RendererCore.copyTextureGpu( *m_debugFrameF4, texture, textureCoords, int2::ONE );
    m_debugFrameF4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const float4 pixelColor = m_debugFrameF4->getPixel( textureCoords );

    setWindowTitle( "float: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")" );
}

void Application::debugDisplayTexturesValue( const std::vector< std::shared_ptr< Texture2D< unsigned char > > >& textures, const int2 screenCoords )
{
    if ( textures.empty() )
        return;

    const float2 textureToScreenSizeRatio = (float2)textures[ 0 ]->getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    std::string debugString = "ior: ";

    for ( auto& texture : textures ) {
        m_dx11RendererCore.copyTextureGpu( *m_debugFrameU1, *texture, textureCoords, int2::ONE );
        m_debugFrameU1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

        const unsigned char pixelColor = m_debugFrameU1->getPixel( textureCoords );
        const float floatVal = (float)pixelColor / 255.0f;

        debugString += std::to_string( 1.0f + floatVal * 2.0f ) + " -> ";
    }

    setWindowTitle( debugString );
}

int2 Application::screenPosToWindowPos( int2 screenPos ) const
{
    POINT pos;
    pos.x = screenPos.x;
    pos.y = screenPos.y;

    ScreenToClient(m_windowHandle, &pos );

    return int2( pos.x, pos.y );
}

void Application::setupBenchmark1()
{
    return;

    Settings initialSettings( settings() );
    m_benchmark.addSettingsToTest( initialSettings );

    for ( int shadowsEnabled = 0; shadowsEnabled <= 1; ++shadowsEnabled ) {
        for ( int reflectionsEnabled = 0; reflectionsEnabled <= 1; ++reflectionsEnabled ) {
            for ( int refractionsEnabled = 0; refractionsEnabled <= 1; ++refractionsEnabled ) {
                for ( int lightBounceCount = 0; lightBounceCount <= 2; ++lightBounceCount  ) 
                {
                    // Skip repetitive settings configurations.
                    if ((lightBounceCount > 0 && !reflectionsEnabled && !refractionsEnabled) ||
                        (lightBounceCount == 0 && (reflectionsEnabled || refractionsEnabled)))
                        continue;

                    Settings testSettings( initialSettings );

                    testSettings.rendering.shadows.enabled                           = (shadowsEnabled != 0);
                    testSettings.rendering.reflectionsRefractions.reflectionsEnabled = (reflectionsEnabled != 0);
                    testSettings.rendering.reflectionsRefractions.refractionsEnabled = (refractionsEnabled != 0);
                    testSettings.rendering.reflectionsRefractions.maxLevel           = lightBounceCount;

                    m_benchmark.addSettingsToTest( testSettings );
                }
            }
        }
    }

    setupBenchmarkScenes();

    m_benchmark.performTests( 12.0f );
}

void Application::setupBenchmark2()
{
    Settings initialSettings( settings() );

    initialSettings.rendering.shadows.enabled                 = true;
    initialSettings.rendering.shadows.useSeparableShadowBlur  = true;
    initialSettings.rendering.reflectionsRefractions.maxLevel = 1;
    initialSettings.rendering.reflectionsRefractions.reflectionsEnabled = true;
    initialSettings.rendering.reflectionsRefractions.refractionsEnabled = true;

    initialSettings.rendering.optimization.useHalfFloatsForRayDirections              = false;
    initialSettings.rendering.optimization.useHalfFloatsForNormals                    = false;
    initialSettings.rendering.optimization.useHalfFLoatsForDistanceToOccluder         = false;
    initialSettings.rendering.optimization.useHalfFloatsForHitDistance                = false;
    initialSettings.rendering.optimization.distToOccluderPositionSampleMipmapLevel    = 0;
    initialSettings.rendering.optimization.distToOccluderNormalSampleMipmapLevel      = 0;

    initialSettings.rendering.hitDistanceSearch.resolutionDivider = 1;

    m_benchmark.addSettingsToTest( initialSettings );

    Settings testSettings1( initialSettings );

    testSettings1.rendering.optimization.useHalfFloatsForRayDirections              = true;
    testSettings1.rendering.optimization.useHalfFloatsForNormals                    = true;
    testSettings1.rendering.optimization.useHalfFLoatsForDistanceToOccluder         = true;
    testSettings1.rendering.optimization.useHalfFloatsForHitDistance                = true;
    testSettings1.rendering.optimization.distToOccluderPositionSampleMipmapLevel    = 1;
    testSettings1.rendering.optimization.distToOccluderNormalSampleMipmapLevel      = 1;

    testSettings1.rendering.hitDistanceSearch.resolutionDivider = 4;

    m_benchmark.addSettingsToTest( testSettings1 );

    /*for ( int separableShadowBlurEnabled = 0; separableShadowBlurEnabled <= 1; ++separableShadowBlurEnabled ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.shadows.useSeparableShadowBlur = (separableShadowBlurEnabled != 0);

        m_benchmark.addSettingsToTest( testSettings );
    }*/

    /*
    for ( float combiningSamplingQuality = 0.4f; combiningSamplingQuality <= 0.8f; combiningSamplingQuality += 0.1f ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.reflectionsRefractions.samplingQuality = combiningSamplingQuality;

        m_benchmark.addSettingsToTest( testSettings );
    }*/

    for ( int useHalfFloatsForRayDirections = 0; useHalfFloatsForRayDirections <= 1; ++useHalfFloatsForRayDirections ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.optimization.useHalfFloatsForRayDirections = (useHalfFloatsForRayDirections != 0);

        m_benchmark.addSettingsToTest( testSettings );
    }

    for ( int useHalfFloatsForNormals = 0; useHalfFloatsForNormals <= 1; ++useHalfFloatsForNormals ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.optimization.useHalfFloatsForNormals = (useHalfFloatsForNormals != 0);

        m_benchmark.addSettingsToTest( testSettings );
    }

    for ( int useHalfFLoatsForDistanceToOccluder = 0; useHalfFLoatsForDistanceToOccluder <= 1; ++useHalfFLoatsForDistanceToOccluder ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.optimization.useHalfFLoatsForDistanceToOccluder = (useHalfFLoatsForDistanceToOccluder != 0);

        m_benchmark.addSettingsToTest( testSettings );
    }

    for ( int useHalfFloatsForHitDistance = 0; useHalfFloatsForHitDistance <= 1; ++useHalfFloatsForHitDistance ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.optimization.useHalfFloatsForHitDistance = (useHalfFloatsForHitDistance != 0);

        m_benchmark.addSettingsToTest( testSettings );
    }

    for ( int distToOccluderPositionSampleMipmapLevel = 0; distToOccluderPositionSampleMipmapLevel <= 1; ++distToOccluderPositionSampleMipmapLevel ) 
    {
        for ( int distToOccluderNormalSampleMipmapLevel = 0; distToOccluderNormalSampleMipmapLevel <= 1; ++distToOccluderNormalSampleMipmapLevel) 
        {
            Settings testSettings( initialSettings );

            testSettings.rendering.optimization.distToOccluderPositionSampleMipmapLevel = distToOccluderPositionSampleMipmapLevel;
            testSettings.rendering.optimization.distToOccluderNormalSampleMipmapLevel   = distToOccluderNormalSampleMipmapLevel;

            m_benchmark.addSettingsToTest( testSettings );
        }
    }

    for ( int resolutionDivider = 1; resolutionDivider <= 4; ++resolutionDivider) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.hitDistanceSearch.resolutionDivider = resolutionDivider;

        m_benchmark.addSettingsToTest( testSettings );
    }

    setupBenchmarkScenes();

    m_benchmark.performTests( 12.0f );
}

void Application::setupBenchmark3()
{
    return;

    Settings initialSettings( settings() );

    initialSettings.rendering.shadows.enabled                 = true;
    initialSettings.rendering.shadows.useSeparableShadowBlur  = true;
    initialSettings.rendering.reflectionsRefractions.maxLevel = 1;
    initialSettings.rendering.reflectionsRefractions.reflectionsEnabled = true;
    initialSettings.rendering.reflectionsRefractions.refractionsEnabled = true;

    m_benchmark.addSettingsToTest( initialSettings );

    for ( float combiningSamplingQuality = 0.4f; combiningSamplingQuality <= 1.01f; combiningSamplingQuality += 0.1f ) 
    {
        Settings testSettings( initialSettings );

        testSettings.rendering.reflectionsRefractions.samplingQuality = combiningSamplingQuality;

        m_benchmark.addSettingsToTest( testSettings );
    }

    setupBenchmarkScenes();

    m_benchmark.performTests( 55.0f );
}

void Application::setupBenchmark4()
{
    return;

    Settings initialSettings( settings() );

    m_benchmark.addSettingsToTest( initialSettings );

    // All layers use same mipmap level - same as hard layer, but make larger steps.
    Settings settings2( settings() );

    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight       = 10.0f;//5.0f
    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight         = 1.0f;//1.0f
    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow      = 8.0f;//2.0f
    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow        = 1.0f;//1.0f
    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel          = 2;//2
    settings2.rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider   = 4;//4

    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight     = 20.0f;//10.0 
    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight       = 2.0f;//1.0
    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow    = 14.0f;//5.0
    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow      = 2.0f;//1.0
    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel        = 2;//3
    settings2.rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider = 4;//8

    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight       = 40.0f;//10.0
    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight         = 4.0f;//1.0
    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow      = 28.0f;//7.0
    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow        = 4.0f;//1.0
    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel          = 2;//4
    settings2.rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider   = 4;//16

    m_benchmark.addSettingsToTest( settings2 );

    // All layers use same mipmap level - same as hard layer, and make step size = 1.
    Settings settings3( settings() );

    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight       = 10.0f;//5.0f
    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight         = 1.0f;//1.0f
    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow      = 8.0f;//2.0f
    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow        = 1.0f;//1.0f
    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel          = 2;//2
    settings3.rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider   = 4;//4

    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight     = 20.0f;//10.0 
    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight       = 1.0f;//1.0
    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow    = 14.0f;//5.0
    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow      = 1.0f;//1.0
    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel        = 2;//3
    settings3.rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider = 4;//8

    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight       = 40.0f;//10.0
    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight         = 1.0f;//1.0
    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow      = 28.0f;//7.0
    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow        = 1.0f;//1.0
    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel          = 2;//4
    settings3.rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider   = 4;//16

    m_benchmark.addSettingsToTest( settings3 );

    // All layers use zero level mipmap, and make step size = 1.
    Settings settings4( settings() );

    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInLight       = 40.0f;//5.0f
    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInLight         = 1.0f;//1.0f
    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.searchRadiusInShadow      = 32.0f;//2.0f
    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.searchStepInShadow        = 1.0f;//1.0f
    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.inputMipmapLevel          = 0;//2
    settings4.rendering.shadows.distanceToOccluderSearch.hardShadows.outputDimensionsDivider   = 1;//4

    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInLight     = 80.0f;//10.0 
    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInLight       = 1.0f;//1.0
    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchRadiusInShadow    = 56.0f;//5.0
    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.searchStepInShadow      = 1.0f;//1.0
    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.inputMipmapLevel        = 0;//3
    settings4.rendering.shadows.distanceToOccluderSearch.mediumShadows.outputDimensionsDivider = 1;//8

    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInLight       = 160.0f;//10.0
    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInLight         = 1.0f;//1.0
    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.searchRadiusInShadow      = 112.0f;//7.0
    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.searchStepInShadow        = 1.0f;//1.0
    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.inputMipmapLevel          = 0;//4
    settings4.rendering.shadows.distanceToOccluderSearch.softShadows.outputDimensionsDivider   = 1;//16

    m_benchmark.addSettingsToTest( settings4 );

    setupBenchmarkScenes();

    m_benchmark.performTests( 55.0f );
}

void Application::setupBenchmarkScenes()
{
    m_benchmark.addSceneToTest( 
        AssetPathManager::get().getPathForFileName( "benchmark0 - cornelbox.scene" ),
        AssetPathManager::get().getPathForFileName( "benchmark0 - cornelbox.cameraanim" )
    );

    /*m_benchmark.addSceneToTest( 
        AssetAssetPathManager::get().getPathForFileName( "benchmark1 - sponza.scene" ),
        AssetAssetPathManager::get().getPathForFileName( "benchmark1 - sponza.cameraanim" )
    );*/

    /*m_benchmark.addSceneToTest( 
        AssetAssetPathManager::get().getPathForFileName( "benchmark2 - office.scene" ),
        AssetAssetPathManager::get().getPathForFileName( "benchmark2 - office.cameraanim" )
    );*/

    /*m_benchmark.addSceneToTest( 
        AssetAssetPathManager::get().getPathForFileName( "benchmark3 - loft.scene" ),
        AssetAssetPathManager::get().getPathForFileName( "benchmark3 - loft.cameraanim" )
    );*/
}

