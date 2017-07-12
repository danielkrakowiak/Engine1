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

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;
PhysicsLibrary Application::physicsLibrary;

using Microsoft::WRL::ComPtr;

Application::Application() :
	m_rendererCore(),
	m_frameRenderer( m_rendererCore, m_profiler ),
    m_renderer( m_rendererCore, m_profiler, m_renderTargetManager ),
	m_initialized( false ),
	m_applicationInstance( nullptr ),
	m_windowHandle( nullptr ),
	m_deviceContext( nullptr ),
    m_windowPosition( 0, 0 ),
	m_windowFocused( false ),
    m_assetManager(),
    m_sceneManager( m_assetManager ),
    m_controlPanel( m_sceneManager )
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->m_applicationInstance = applicationInstance;

	setupWindow();

    const int parallelThreadCount = std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) : 1;

	m_frameRenderer.initialize( m_windowHandle, settings().main.screenDimensions.x, settings().main.screenDimensions.y, settings().main.fullscreen, settings().main.verticalSync );
	m_rendererCore.initialize( *m_frameRenderer.getDeviceContext( ).Get() );
    m_assetManager.initialize( parallelThreadCount, parallelThreadCount, m_frameRenderer.getDevice() );
    m_profiler.initialize( m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext() );
    m_renderTargetManager.initialize( m_frameRenderer.getDevice() );

    m_sceneManager.initialize( m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext() );

    createUcharDisplayFrame( settings().main.screenDimensions.x, settings().main.screenDimensions.y, m_frameRenderer.getDevice() );
    createDebugFrames( settings().main.screenDimensions.x, settings().main.screenDimensions.y, m_frameRenderer.getDevice() );

    // Load 'light source' model.
	std::shared_ptr<BlockModel> lightModel;
	try
	{
        const auto path = AssetPathManager::getPathForFileName( "light_bulb.blockmodel" );
		BlockModelFileInfo lightModelFileInfo( path, BlockModelFileInfo::Format::BLOCKMODEL, 0);
		lightModel = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( lightModelFileInfo ) );
		lightModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext().Get() );
	}
	catch (...) {}

    m_renderer.initialize( 
        settings().main.screenDimensions, 
        m_frameRenderer.getDevice(), 
        m_frameRenderer.getDeviceContext(), 
        lightModel 
    );

    m_controlPanel.initialize( m_frameRenderer.getDevice(), settings().main.screenDimensions );

	m_initialized = true;

    onStart();
}

void Application::createUcharDisplayFrame( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device )
{
    ucharDisplayFrame = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >
        ( *device.Get(), imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM );
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

	bool run = true;
	MSG msg;

	Font font( settings().main.screenDimensions );
	font.loadFromFile( AssetPathManager::getPathForFileName( "consola.ttf" ), 35 );

    Font font2( settings().main.screenDimensions );
    font2.loadFromFile( AssetPathManager::getPathForFileName( "consola.ttf" ), 13 );

    // Profiling.
    const float profilingDisplayRefreshDelayMs = 200.0f;
    double frameTimeMs = 0.0;
    float totalFrameTimeGPU = 0.0f, totalFrameTimeCPU = 0.0f;
    float deferredRenderingTime = 0.0f;
    float mainMipmapGenerationForPositionAndNormalsTime = 0.0f;
    std::array< StageProfilingInfo, (int)Profiler::StageType::MAX_VALUE > stageProfilingInfo;

    Timer profilingLastRefreshTime;

    m_renderer.renderShadowMaps( m_sceneManager.getScene() );

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
        bool modifyingScene = false;

        // Disable locking when connecting through Team Viewer.
        bool lockCursor = false;

        modifyingScene = onFrame( frameTimeMs, lockCursor );

        m_renderer.clear();

        if ( modifyingScene )
            m_renderer.renderShadowMaps( m_sceneManager.getScene() );

        Renderer::Output output;
        output = m_renderer.renderScene( 
            m_sceneManager.getScene(), 
            m_sceneManager.getCamera(), 
            settings().debug.debugWireframeMode, 
            m_sceneManager.getSelection(), 
            m_sceneManager.getSelectionVolumeMesh() 
        );

        const int2 mousePos = m_inputManager.getMousePos();

        try 
        {
            if ( output.ucharImage ) 
            {
                m_rendererCore.copyTexture( 
                    *ucharDisplayFrame, *output.ucharImage, 
                    int2( 0, 0 ), output.ucharImage->getDimensions() 
                );

		        m_frameRenderer.renderTexture( 
                    *ucharDisplayFrame, 0.0f, 0.0f, 
                    (float)settings().main.screenDimensions.x, 
                    (float)settings().main.screenDimensions.y, 
                    false, 
                    settings().debug.debugDisplayedMipmapLevel 
                );

                if ( m_inputManager.isMouseButtonPressed(0) ) 
                    debugDisplayTextureValue( *output.ucharImage, mousePos );
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

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *output.uchar4Image, mousePos );
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

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *output.float4Image, mousePos );
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

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *output.floatImage, mousePos );
            }

            if ( m_inputManager.isMouseButtonPressed( 0 ) && m_renderer.getActiveViewType() == Renderer::View::CurrentRefractiveIndex )
            {
                debugDisplayTexturesValue( m_renderer.debugGetCurrentRefractiveIndexTextures(), mousePos );
            }
        }
        catch( ... )
        {}

        m_renderer.clear2();

        if ( updateProfiling )
        {
            profilingLastRefreshTime.reset();

            { // Render profiling results.
                totalFrameTimeCPU                             = (float)frameTimeMs;
                totalFrameTimeGPU                             = m_profiler.getEventDuration( Profiler::GlobalEventType::Frame );
                deferredRenderingTime                         = m_profiler.getEventDuration( Profiler::GlobalEventType::DeferredRendering );
                mainMipmapGenerationForPositionAndNormalsTime = m_profiler.getEventDuration( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

                for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, settings().rendering.reflectionsRefractions.maxLevel + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
                {
                    stageProfilingInfo[ stage ].shadowsTotal = 0.0f;
                    stageProfilingInfo[ stage ].shadingTotal = 0.0f;

                    for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
                    {
                        const float shadowPerLight  = m_profiler.getEventDuration( (Profiler::StageType)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
                        const float shadingPerLight = m_profiler.getEventDuration( (Profiler::StageType)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
                        
                        if ( shadowPerLight >= 0.0f )
                            stageProfilingInfo[ stage ].shadowsTotal += shadowPerLight;

                        if ( shadingPerLight )
                            stageProfilingInfo[ stage ].shadingTotal += shadingPerLight;
                    }
                }
            }

        }

        // Drop references to render targets before starting text rendering.
        // (reference counting of render targets).
        output.reset();

        { // Render FPS.
            std::stringstream ss;
            ss << "FPS: " << (int)( 1000.0 / totalFrameTimeCPU ) << " / " << totalFrameTimeCPU << "ms";
            
            if ( settings().debug.renderFps )
                output = m_renderer.renderText( ss.str(), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render current view.
            std::stringstream ss;
            ss << "View: " << Renderer::viewToString( m_renderer.getActiveViewType() );

            if ( settings().debug.renderFps )
                output = m_renderer.renderText( ss.str(), font2, float2( -500.0f, 350.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render some debug options.
            std::stringstream ss;
            ss << "Use separable shadow blur: " << ( settings().rendering.shadows.useSeparableShadowBlur ? "enabled" : "disabled" );

            ss << "\n\n";

            ss << "Exposure: " << m_renderer.getExposure();

            if ( settings().debug.renderFps )
                output = m_renderer.renderText( ss.str(), font2, float2( 150.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render profiling results.
            std::stringstream ss, ss2;
            ss << std::fixed << std::setprecision( 2 );
            ss2 << std::fixed << std::setprecision( 2 );
            ss << "Profiling: \n";
            ss << "Total: " << totalFrameTimeGPU << " ms \n";

            // Print global events duration.
            for (int globalEventType = (int)Profiler::GlobalEventType::DeferredRendering; globalEventType < (int)Profiler::GlobalEventType::MAX_VALUE; ++globalEventType)
            {
                const std::string eventName     = Profiler::eventTypeToString( (Profiler::GlobalEventType)globalEventType );
                const float       eventDuration = m_profiler.getEventDuration( (Profiler::GlobalEventType)globalEventType );

                ss << eventName << ": " << eventDuration << "ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";
            }

            for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, settings().rendering.reflectionsRefractions.maxLevel + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
            {
                // Print stage name.
                const std::string stageName = Profiler::stageTypeToString( (Profiler::StageType)stage );
                ss << stageName << "\n";

                // Print total duration of shadow calculations.
                const float totalShadowsDuration = stageProfilingInfo[ stage ].shadowsTotal;
                if ( totalShadowsDuration > 0.0f )
                    ss << "Total shadows: " << totalShadowsDuration << "ms " << ( totalShadowsDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

                // Print total duration of shading calculations.
                const float totalShadingDuration = stageProfilingInfo[ stage ].shadingTotal;
                if ( totalShadingDuration > 0.0f )
                    ss << "Total shading: " << totalShadingDuration << "ms " << ( totalShadingDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

                // Print duration of events occurring at each stage.
                for ( int eventType = (int)Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals; eventType < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventType )
                {
                    const float       eventDuration = m_profiler.getEventDuration( (Profiler::StageType)stage, (Profiler::EventTypePerStage)eventType );
                    const std::string eventName     = Profiler::eventTypeToString( (Profiler::EventTypePerStage)eventType );

                    if ( eventDuration >= 0.0f )
                        ss << "    " << eventName << ": " << eventDuration << " ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";
                }

                for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx )
                {
                    bool display = false;
                    ss2.str( "" ); // Clear stream.
                    ss2.clear();   // Clear stream errors.

                    ss2 << "        Light " << lightIdx << "\n";

                    for ( int eventType = (int)Profiler::EventTypePerStagePerLight::ShadowsMapping; eventType < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventType )
                    {
                        const float       eventDuration = m_profiler.getEventDuration( ( Profiler::StageType )stage, lightIdx, ( Profiler::EventTypePerStagePerLight )eventType );
                        const std::string eventName     = Profiler::eventTypeToString( ( Profiler::EventTypePerStagePerLight )eventType );

                        if ( eventDuration > 0.0f )
                        {
                            ss2 << "                " << eventName << ": " << eventDuration << " ms " << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "%\n";
                            display = true;
                        }
                    }

                    ss2 << "\n";

                    // If accumulated events duration for that light is non-zero - display it's info.
                    if ( display )
                        ss << ss2.str();
                }
            }

            if ( settings().debug.renderText )
                output = m_renderer.renderText( ss.str(), font2, float2( -500.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render scene stats and selection stats.
            int selectedVertexCount   = 0;
            int selectedTriangleCount = 0;
            int selectedMeshesCount   = (int)(m_sceneManager.getSelectedBlockActors().size() + m_sceneManager.getSelectedSkeletonActors().size()); 
            int selectedLightsCount   = (int)m_sceneManager.getSelectedLights().size();
            int totalVertexCount      = 0;
            int totalTriangleCount    = 0;
            int totalActors           = (int)m_sceneManager.getScene().getActors().size();
            

            std::tie( selectedVertexCount, selectedTriangleCount ) = m_sceneManager.getSelectedActorsVertexAndTriangleCount();
            std::tie( totalVertexCount, totalTriangleCount )       = m_sceneManager.getSceneVertexAndTriangleCount();

            std::stringstream ss;
            ss << "Selected: " << selectedVertexCount << " verts / " << selectedTriangleCount << " tris " << selectedMeshesCount << " actors \n";
            ss << "Scene:    " << totalVertexCount    << " verts / " << totalTriangleCount    << " tris " << totalActors         << " actors \n";

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
                    meshPath  = model->getMesh()->getFileInfo().getPath();

                    if ( model->getMesh()->getBvhTree() )
                    {
                        bvhNodes        = (int)model->getMesh()->getBvhTree()->getNodes().size();
                        bvhNodesExtents = (int)model->getMesh()->getBvhTree()->getNodesExtents().size();
                        bvhTriangles    = (int)model->getMesh()->getBvhTree()->getTriangles().size();
                    }
                }
                else if ( !m_sceneManager.getSelectedSkeletonActors().empty() &&
                     m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel() &&
                     m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel()->getMesh() )
                {
                    const auto& model = m_sceneManager.getSelectedSkeletonActors()[ 0 ]->getModel();

                    modelPath = model->getFileInfo().getPath();
                    meshPath  = model->getMesh()->getFileInfo().getPath();
                }

                const size_t modelPathStartIndex = modelPath.rfind( "\\" );
                const size_t meshPathStartIndex  = meshPath.rfind( "\\" );

                modelPath = modelPath.substr( modelPathStartIndex != std::string::npos ? modelPathStartIndex : 0 );
                meshPath  = meshPath.substr( meshPathStartIndex != std::string::npos ? meshPathStartIndex : 0 );

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
                        << ( light->isCastingShadows() ? "casts shadow" : "does not cast shadow" );
                }
                else if ( light->getType() == Light::Type::SpotLight )
                {
                    SpotLight& spotLight = static_cast< SpotLight& >( *light );

                    ss << "Spot light - emitter radius: " << light->getEmitterRadius() << "\n"
                       << ", color: (" << light->getColor().x << ", " << light->getColor().y << ", " << light->getColor().z << "), " << "\n"
                       << ( light->isCastingShadows() ? "casts shadow" : "does not cast shadow" )
                       << ", cone angle: " << MathUtil::radiansToDegrees( spotLight.getConeAngle() ) << " deg.";
                }

                ss << "\nBlurShadowsComputeShader::s_positionThreshold: " << BlurShadowsComputeShader::s_positionThreshold;
            }

            if ( m_sceneManager.isSelectionEmpty() )
            {
                ss << "\nrendering.combining.positionDiffMul: "                       << settings().rendering.combining.positionDiffMul << " [C + P]";
                ss << "\nrendering.combining.normalDiffMul: "                         << settings().rendering.combining.normalDiffMul << " [C + N]";
                ss << "\nrendering.combining.positionNormalThreshold: "               << settings().rendering.combining.positionNormalThreshold << " [C + T]";
                ss << "\nHitDistanceSearchComputeShader::s_positionDiffMul: "                << HitDistanceSearchComputeShader::s_positionDiffMul << " [R + P]";
                ss << "\nHitDistanceSearchComputeShader::s_normalDiffMul: "                  << HitDistanceSearchComputeShader::s_normalDiffMul << " [R + N]";
                ss << "\nHitDistanceSearchComputeShader::s_positionNormalThreshold: "        << HitDistanceSearchComputeShader::s_positionNormalThreshold << " [R + T]";
                ss << "\nHitDistanceSearchComputeShader::s_minSampleWeightBasedOnDistance: " << HitDistanceSearchComputeShader::s_minSampleWeightBasedOnDistance << " [R + W]";

                const float screenPixelCount = (float)(settings().main.screenDimensions.x * settings().main.screenDimensions.y);
                const float bytesInMegabyte = 1024.0f * 1024.0f;

                const int renderTargetFloat4Count      = m_renderTargetManager.getTotalRenderTargetCount< float4 >();
                const int renderTargetFloatCount       = m_renderTargetManager.getTotalRenderTargetCount< float >();
                const int renderTargetUchar4Count      = m_renderTargetManager.getTotalRenderTargetCount< uchar4 >();
                const int renderTargetUcharCount       = m_renderTargetManager.getTotalRenderTargetCount< unsigned char >();
                const int renderTargetUchar4DepthCount = m_renderTargetManager.getTotalRenderTargetDepthCount();

                ss << "\n\nRender target usage:";
                ss << "\n float4: " << renderTargetFloat4Count << ", approx.: " << (float)renderTargetFloat4Count * ( screenPixelCount * 16.0f ) / bytesInMegabyte << " MB";
                ss << "\n float:  " << renderTargetFloatCount << ", approx.: " << (float)renderTargetFloatCount * ( screenPixelCount * 4.0f ) / bytesInMegabyte << " MB";
                ss << "\n uchar4: " << renderTargetUchar4Count << ", approx.: " << (float)renderTargetUchar4Count * ( screenPixelCount * 4.0f ) / bytesInMegabyte << " MB";
                ss << "\n uchar:  " << renderTargetUcharCount << ", approx.: " << (float)renderTargetUcharCount * ( screenPixelCount * 1.0f ) / bytesInMegabyte << " MB";
                ss << "\n uchar4 depth: " << renderTargetUchar4DepthCount << ", approx.: " << (float)renderTargetUchar4DepthCount * ( screenPixelCount * 4.0f ) / bytesInMegabyte << " MB";
            }

            if ( settings().debug.renderText )
                output = m_renderer.renderText( ss.str(), font2, float2( 0.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render camera state.
            //std::stringstream ss;
            //ss << "Cam pos: " << camera.getPosition( ).x << ", " << camera.getPosition( ).y << ", " << camera.getPosition( ).z;
            //deferredRenderer.render( ss.str( ), font2, float2( -500.0f, 200.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render keyboard state.
            //std::stringstream ss;
            //ss << "";
            //if ( inputManager.isKeyPressed( InputManager::Keys::w ) ) ss << "W";
            //if ( inputManager.isKeyPressed( InputManager::Keys::s ) ) ss << "S";
            //if ( inputManager.isKeyPressed( InputManager::Keys::a ) ) ss << "A";
            //if ( inputManager.isKeyPressed( InputManager::Keys::d ) ) ss << "D";

            //direct3DRenderer.renderText( ss.str(), font, float2( -500.0f, 270.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render mouse state.
            //std::stringstream ss;
            //ss << "Mouse pos: " << inputManager.getMousePos().x << ", " << inputManager.getMousePos().y << ", ";
            //deferredRenderer.render( ss.str( ), font2, float2( -500.0f, 100.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render combining renderer position/normal thresholds.
            //std::stringstream ss;
            //ss << "Combining renderer normal threshold:   " << combiningRenderer.getNormalThreshold() << std::endl;
            //ss << "Combining renderer position threshold: " << combiningRenderer.getPositionThreshold() << std::endl;
            //deferredRenderer.render( ss.str( ), font2, float2( -500.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        // Required to be able to display depth-stencil buffer.
        // TODO: Should  be refactored somehow. Such method should not be called here.
        //deferredRenderer.disableRenderTargets();

        if ( output.uchar4Image )
            m_frameRenderer.renderTexture( *output.uchar4Image, 0.0f, 0.0f, (float)settings().main.screenDimensions.x, (float)settings().main.screenDimensions.y, true );

        m_controlPanel.draw();

		m_frameRenderer.displayFrame();

        m_profiler.endEvent( Profiler::GlobalEventType::Frame );
        m_profiler.endFrameProfiling();

        updateProfiling = false;
        m_profiler.pauseProfiling();

		Timer frameEndTime;
		frameTimeMs = Timer::getElapsedTime( frameEndTime, frameStartTime );

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

			const bool replaceAssets = (fileCount == 1);

            try {
                windowsMessageReceiver->onDragAndDropFile( path, replaceAssets );
            } catch ( ... ) {}
        }
			
		break;
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

void Application::createDebugFrames( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device )
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

void Application::debugDisplayTextureValue( const Texture2DGeneric< unsigned char >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_rendererCore.copyTexture( *m_debugFrameU1, texture, textureCoords, int2::ONE );
    m_debugFrameU1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const unsigned char pixelColor = m_debugFrameU1->getPixel( textureCoords );

    const float floatVal = (float)pixelColor / 255.0f;
    setWindowTitle( "uchar: " + std::to_string( pixelColor ) + ", float: " + std::to_string( floatVal ) + ", ior: " + std::to_string( 1.0f + floatVal * 2.0f ) );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< uchar4 >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_rendererCore.copyTexture( *m_debugFrameU4, texture, textureCoords, int2::ONE );
    m_debugFrameU4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const uchar4 pixelColor = m_debugFrameU4->getPixel( textureCoords );

    const float4 floatVal = float4( (float4)pixelColor / 255.0f );
    setWindowTitle( "uchar: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")"
                    + ", float: (" + std::to_string( floatVal.x ) + ", " + std::to_string( floatVal.y ) + ", " + std::to_string( floatVal.z ) + ", " + std::to_string( floatVal.w ) + ")" );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< float >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_rendererCore.copyTexture( *m_debugFrameF1, texture, textureCoords, int2::ONE );
    m_debugFrameF1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const float pixelColor = m_debugFrameF1->getPixel( textureCoords );

    setWindowTitle( "float: " + std::to_string( pixelColor ) );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< float4 >& texture, const int2 screenCoords )
{
    const float2 textureToScreenSizeRatio = (float2)texture.getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    m_rendererCore.copyTexture( *m_debugFrameF4, texture, textureCoords, int2::ONE );
    m_debugFrameF4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), textureCoords, int2::ONE );

    const float4 pixelColor = m_debugFrameF4->getPixel( textureCoords );

    setWindowTitle( "float: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")" );
}

void Application::debugDisplayTexturesValue( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > >& textures, const int2 screenCoords )
{
    if ( textures.empty() )
        return;

    const float2 textureToScreenSizeRatio = (float2)textures[ 0 ]->getDimensions( 0 ) / (float2)settings().main.screenDimensions;
    const int2   textureCoords = (int2)( (float2)screenCoords * textureToScreenSizeRatio );

    std::string debugString = "ior: ";

    for ( auto& texture : textures ) {
        m_rendererCore.copyTexture( *m_debugFrameU1, *texture, textureCoords, int2::ONE );
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
