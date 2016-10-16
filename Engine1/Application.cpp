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

#include "CScene.h"

#include "Timer.h"

#include "BVHTree.h"
#include "BVHTreeBuffer.h"

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;

using namespace Engine1;

using Microsoft::WRL::ComPtr;

Application::Application() :
	m_rendererCore(),
	m_frameRenderer( m_rendererCore, m_profiler ),
    m_renderer( m_rendererCore, m_profiler ),
	m_initialized( false ),
	m_applicationInstance( nullptr ),
	m_windowHandle( nullptr ),
	m_deviceContext( nullptr ),
    m_windowPosition( 0, 0 ),
	m_fullscreen( false ),
	m_screenWidth( 1024 /*1920*/ ),
	m_screenHeight( 768 /*1080*/ ),
	m_verticalSync( false ),
	m_displayFrequency( 60 ),
	m_screenColorDepth( 32 ),
	m_zBufferDepth( 32 ),
	m_windowFocused( false ),
    m_debugRenderAlpha( false ),
    m_scenePath( "Assets/Scenes/new.scene" ),
    m_cameraPath( "Assets/Scenes/new.camera" ),
    m_scene( std::make_shared<CScene>() ),
    m_assetManager()
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->m_applicationInstance = applicationInstance;

	setupWindow();

	m_frameRenderer.initialize( m_windowHandle, m_screenWidth, m_screenHeight, m_fullscreen, m_verticalSync );
	m_rendererCore.initialize( *m_frameRenderer.getDeviceContext( ).Get() );
    m_assetManager.initialize( std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) * 2 : 1, m_frameRenderer.getDevice() );
    m_profiler.initialize( m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext() );

    createDebugFrames( m_screenWidth, m_screenHeight, m_frameRenderer.getDevice() );
    createUcharDisplayFrame( m_screenWidth, m_screenHeight, m_frameRenderer.getDevice() );

    // Load 'axises' model.
    //BlockMeshFileInfo axisMeshFileInfo( "Assets/Meshes/dx-coordinate-axises.obj", BlockMeshFileInfo::Format::OBJ, 0, false, false, false );
    //std::shared_ptr<BlockMesh> axisMesh = std::static_pointer_cast<BlockMesh>(assetManager.getOrLoad( axisMeshFileInfo ));
    //axisMesh->buildBvhTree();
    //axisMesh->loadCpuToGpu( *frameRenderer.getDevice().Get() );
    //axisMesh->loadBvhTreeToGpu( *frameRenderer.getDevice().Get() );

    //// Load 'light source' model.
	std::shared_ptr<BlockModel> lightModel;
	try
	{
		BlockModelFileInfo lightModelFileInfo("Assets/Models/light_bulb.blockmodel", BlockModelFileInfo::Format::BLOCKMODEL, 0);
		lightModel = std::static_pointer_cast<BlockModel>(m_assetManager.getOrLoad(lightModelFileInfo));
		lightModel->loadCpuToGpu(*m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext().Get());
	}
	catch (...) {}

    m_renderer.initialize( m_screenWidth, m_screenHeight, m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext(), nullptr /*axisMesh*/, lightModel );

	m_initialized = true;
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

void Application::createUcharDisplayFrame( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device )
{
    ucharDisplayFrame = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >
        ( *device.Get(), imageWidth, imageHeight, false, true, false, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM );
}

void Application::setupWindow() {
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

	if ( m_fullscreen ) {
		DEVMODE screen = { 0 };

		screen.dmSize = sizeof( DEVMODE );
		screen.dmPelsWidth = m_screenWidth;
		screen.dmPelsHeight = m_screenHeight;
		screen.dmBitsPerPel = m_screenColorDepth;
		screen.dmDisplayFrequency = m_displayFrequency;
		screen.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		ChangeDisplaySettings( &screen, CDS_FULLSCREEN );

		style |= WS_POPUP;
	} else {
		style |= WS_OVERLAPPEDWINDOW;
	}

	DWORD exStyle = WS_EX_ACCEPTFILES; // Allow drag&drop files.

    int windowWidth = m_screenWidth;
    int windowHeight = m_screenHeight;
    if ( !m_fullscreen )
    {
        RECT windowArea;
        windowArea.left   = 0;
        windowArea.top    = 0;
        windowArea.right  = m_screenWidth;
        windowArea.bottom = m_screenHeight;

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
		(BYTE)m_screenColorDepth,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)m_zBufferDepth,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	pixelFormat = ChoosePixelFormat( m_deviceContext, &pfd );
	SetPixelFormat( m_deviceContext, pixelFormat, &pfd );
}

void Application::show() {
	if ( !m_initialized ) throw std::exception( "Application::show called on uninitialized Application" );

	ShowWindow( m_windowHandle, SW_SHOW );
}

void Application::run() {
	if ( !m_initialized ) throw std::exception( "Application::run called on uninitialized Application" );

	bool run = true;
	MSG msg;

    // Setup the camera.
	m_camera.setUp( float3( 0.0f, 1.0f, 0.0f ) );
	m_camera.setPosition( float3( 30.0f, 4.0f, -53.0f ) );
    m_camera.rotate( float3( 0.0f, MathUtil::piHalf, 0.0f ) );
     
	Font font( uint2(m_screenWidth, m_screenHeight) );
	font.loadFromFile( "Assets/Fonts/consola.ttf", 35 );

    Font font2( uint2(m_screenWidth, m_screenHeight) );
    font2.loadFromFile( "Assets/Fonts/consola.ttf", 10 );

    // Profiling.
    const float profilingDisplayRefreshDelayMs = 200.0f;
    double frameTimeMs = 0.0;
    float totalFrameTimeGPU = 0.0f, totalFrameTimeCPU = 0.0f;
    float deferredRenderingTime = 0.0f;
    float mainMipmapGenerationForPositionAndNormalsTime = 0.0f;
    std::array< StageProfilingInfo, (int)Profiler::StageType::MAX_VALUE > stageProfilingInfo;

    Timer profilingLastRefreshTime;

	while ( run ) {
		Timer frameStartTime;

        m_profiler.beginFrameProfiling();
        m_profiler.beginEvent( Profiler::GlobalEventType::Frame );

        // Disable locking when connecting through Team Viewer.
        bool lockCursor = false;

		while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( WM_QUIT == msg.message ) run = false;
		}

        // Translate / rotate the selected actors.
        bool movingObjects = false;
        if ( m_windowFocused && ( !m_selectedBlockActors.empty() || !m_selectedSkeletonActors.empty() ) ) 
        {
            const float   translationSensitivity = 0.001f;//0.002f;
            const float   rotationSensitivity    = 0.0001f;
            const int2    mouseMove              = m_inputManager.getMouseMove();

            const float3 sensitivity(
                m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

            if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) 
            {
                for ( auto& actor : m_selectedBlockActors )
                    actor->getPose().rotate( (float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * rotationSensitivity );

                for ( auto& actor : m_selectedSkeletonActors )
                    actor->getPose().rotate( (float)( mouseMove.x - mouseMove.y ) * (float)frameTimeMs * sensitivity * rotationSensitivity );
                
                movingObjects = true;
            } 
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) 
            {
                for ( auto& actor : m_selectedBlockActors )
                    actor->getPose().translate( (float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * translationSensitivity );

                for ( auto& actor : m_selectedSkeletonActors )
                    actor->getPose().translate( (float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * translationSensitivity );

                movingObjects = true;
            }
        }

        // Translate the selected light.
        if ( m_windowFocused && !m_selectedLights.empty() ) 
        {
            const float   translationSensitivity = 0.001f;//0.002f;
            const int2    mouseMove              = m_inputManager.getMouseMove();

            const float3 sensitivity(
                m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

            if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) 
            {
                for ( auto& light : m_selectedLights )
                    light->setPosition( light->getPosition() + ((float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * translationSensitivity ) );
                
                movingObjects = true;
            }
        }

        // Update the camera.
        if ( m_windowFocused && !movingObjects && m_inputManager.isMouseButtonPressed( InputManager::MouseButtons::right ) ) { 
            const float cameraRotationSensitivity = 0.0001f;

            const float acceleration = 1.0f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::w ) ) m_camera.accelerateForward( (float)frameTimeMs * acceleration );
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::s ) ) m_camera.accelerateReverse( (float)frameTimeMs * acceleration );
            if ( m_inputManager.isKeyPressed( InputManager::Keys::d ) ) m_camera.accelerateRight( (float)frameTimeMs * acceleration );
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::a ) ) m_camera.accelerateLeft( (float)frameTimeMs * acceleration );
            if ( m_inputManager.isKeyPressed( InputManager::Keys::e ) ) m_camera.accelerateUp( (float)frameTimeMs * acceleration );
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::q ) ) m_camera.accelerateDown( (float)frameTimeMs * acceleration );

			int2 mouseMove = m_inputManager.getMouseMove( );
			m_camera.rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTimeMs * cameraRotationSensitivity );
		}

        m_inputManager.lockCursor( lockCursor );
        m_inputManager.updateMouseState();

		m_camera.updateState( (float)frameTimeMs );

        { // Update animations.
            const std::unordered_set< std::shared_ptr<Actor> >& sceneActors = m_scene->getActors();

            for ( const std::shared_ptr<Actor>& actor : sceneActors )
            {
                if ( actor->getType() != Actor::Type::SkeletonActor )
                    continue;

                const std::shared_ptr< SkeletonActor > skeletonActor = std::dynamic_pointer_cast< SkeletonActor >( actor );

                skeletonActor->updateAnimation( (float)frameTimeMs / 1000.0f );
            }
        }

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

        m_renderer.clear();

        if ( m_scene )
        {
            std::tie( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat ) 
                = m_renderer.renderScene( *m_scene, m_camera, m_selectedBlockActors, m_selectedSkeletonActors, m_selectedLights );
        }

        const int2 mousePos = m_inputManager.getMousePos();

        try 
        {
            if ( frameUchar ) 
            {
                m_rendererCore.copyTexture( *ucharDisplayFrame, *frameUchar, 0, 0, frameUchar->getWidth(), frameUchar->getHeight() );
		        m_frameRenderer.renderTexture( *ucharDisplayFrame, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );

                if ( m_inputManager.isMouseButtonPressed(0) ) 
                    debugDisplayTextureValue( *frameUchar, mousePos.x, mousePos.y );
            } 
            else if ( frameUchar4 )
            {
                if ( m_debugRenderAlpha )
		            m_frameRenderer.renderTextureAlpha( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );
                else
                    m_frameRenderer.renderTexture( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *frameUchar4, mousePos.x, mousePos.y );
            } 
            else if ( frameFloat4 )
            {
                m_frameRenderer.renderTexture( *frameFloat4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *frameFloat4, mousePos.x, mousePos.y );
            }
            else if ( frameFloat2 )
            {
                m_frameRenderer.renderTexture( *frameFloat2, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );
            }
            else if ( frameFloat ) 
            {
                m_frameRenderer.renderTexture( *frameFloat, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false );

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *frameFloat, mousePos.x, mousePos.y );
            }

            if ( m_inputManager.isMouseButtonPressed( 0 ) && m_renderer.getActiveViewType() == Renderer::View::CurrentRefractiveIndex )
            {
                debugDisplayTexturesValue( m_renderer.debugGetCurrentRefractiveIndexTextures(), mousePos.x, mousePos.y );
            }
        }
        catch( ... )
        {}

        m_renderer.clear2();

        Timer currentTime;
        if ( Timer::getElapsedTime( currentTime, profilingLastRefreshTime ) >= profilingDisplayRefreshDelayMs )
        {
            profilingLastRefreshTime.reset();

            { // Render profiling results.
                totalFrameTimeCPU                             = (float)frameTimeMs;
                totalFrameTimeGPU                             = m_profiler.getEventDuration( Profiler::GlobalEventType::Frame );
                deferredRenderingTime                         = m_profiler.getEventDuration( Profiler::GlobalEventType::DeferredRendering );
                mainMipmapGenerationForPositionAndNormalsTime = m_profiler.getEventDuration( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

                for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, (int)m_renderer.getMaxLevelCount() + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
                {
                    for ( int eventType = (int)Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals; eventType < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventType )
                    {
                        stageProfilingInfo[ stage ].event[ eventType ] = m_profiler.getEventDuration( (Profiler::StageType)stage, (Profiler::EventTypePerStage)eventType );
                    }

                    stageProfilingInfo[ stage ].shadowsTotal = 0.0f;
                    stageProfilingInfo[ stage ].shadingTotal = 0.0f;

                    for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
                    {
                        stageProfilingInfo[ stage ].shadowsPerLight[ lightIdx ] = m_profiler.getEventDuration( (Profiler::StageType)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shadows );
                        if ( stageProfilingInfo[ stage ].shadowsPerLight[ lightIdx ] >= 0.0f )
                            stageProfilingInfo[ stage ].shadowsTotal += stageProfilingInfo[ stage ].shadowsPerLight[ lightIdx ];
                        
                        stageProfilingInfo[ stage ].shadingPerLight[ lightIdx ] = m_profiler.getEventDuration( (Profiler::StageType)stage, lightIdx, Profiler::EventTypePerStagePerLight::Shading );
                        
                        if ( stageProfilingInfo[ stage ].shadingPerLight[ lightIdx ] >= 0.0f )
                            stageProfilingInfo[ stage ].shadingTotal += stageProfilingInfo[ stage ].shadingPerLight[ lightIdx ];
                    }
                }
            }

        }

        { // Render FPS.
            std::stringstream ss;
            ss << "FPS: " << (int)( 1000.0 / totalFrameTimeCPU ) << " / " << totalFrameTimeCPU << "ms";
            frameUchar4 = m_renderer.renderText( ss.str(), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render profiling results.
            std::stringstream ss;
            ss << std::fixed << std::setprecision( 2 );
            ss << "Profiling: \n";
            ss << "Total: " << totalFrameTimeGPU << " ms \n";

            // Print global events duration.
            ss << "DeferredRendering: " << deferredRenderingTime << "ms " << ( deferredRenderingTime / totalFrameTimeGPU ) * 100.0f << "% \n";
            ss << "CopyFrameToFinalRenderTarget: " << mainMipmapGenerationForPositionAndNormalsTime << "ms " << ( mainMipmapGenerationForPositionAndNormalsTime / totalFrameTimeGPU ) * 100.0f << "% \n";

            for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, (int)m_renderer.getMaxLevelCount() + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
            {
                // Print stage name.
                ss << Profiler::stageTypeToString( (Profiler::StageType)stage ) << "\n";

                // Print duration of events occurring at each stage.
                for ( int eventType = (int)Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals; eventType < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventType )
                {
                    const float eventDuration = stageProfilingInfo[ stage ].event[ eventType ];
                    if ( eventDuration >= 0.0f )
                        ss << "    " << Profiler::eventTypeToString( (Profiler::EventTypePerStage)eventType ) << ": " << eventDuration << " ms" << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";
                }

                // Print duration of shadow calculations (total + for each light).
                const float totalShadowsDuration = stageProfilingInfo[ stage ].shadowsTotal;
                if ( totalShadowsDuration > 0.0f )
                    ss << "    Shadows: " << totalShadowsDuration << "ms " << ( totalShadowsDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

                for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx )
                {
                    const float eventDuration = stageProfilingInfo[ stage ].shadowsPerLight[ lightIdx ];
                    if ( eventDuration > 0.0f )
                        ss << "        " << Profiler::eventTypeToString( Profiler::EventTypePerStagePerLight::Shadows, lightIdx ) << ": " << eventDuration << " ms" << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";
                }

                // Print duration of shading calculations (total + for each light).
                const float totalShadingDuration = stageProfilingInfo[ stage ].shadingTotal;
                if ( totalShadingDuration > 0.0f )
                    ss << "    Shading: " << totalShadingDuration << "ms " << ( totalShadingDuration / totalFrameTimeGPU ) * 100.0f << "% \n";

                for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
                {
                    const float eventDuration = stageProfilingInfo[ stage ].shadingPerLight[ lightIdx ];
                    if ( eventDuration > 0.0f )
                        ss << "        " << Profiler::eventTypeToString( Profiler::EventTypePerStagePerLight::Shading, lightIdx ) << ": " << eventDuration << " ms" << ( eventDuration / totalFrameTimeGPU ) * 100.0f << "% \n";
                }
            }

            frameUchar4 = m_renderer.renderText( ss.str(), font2, float2( -500.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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

        m_frameRenderer.renderTexture( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, true );

		m_frameRenderer.displayFrame();

        m_profiler.endEvent( Profiler::GlobalEventType::Frame );
        m_profiler.endFrameProfiling();

		Timer frameEndTime;
		frameTimeMs = Timer::getElapsedTime( frameEndTime, frameStartTime );
	}
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< unsigned char >& texture, const int x, const int y )
{
    m_rendererCore.copyTexture( *m_debugFrameU1, texture, x, y, 1, 1 );
    m_debugFrameU1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), x, y, 1, 1 );

    const unsigned char pixelColor = m_debugFrameU1->getPixel( x, y );

    const float floatVal = (float)pixelColor / 255.0f;
    setWindowTitle( "uchar: " + std::to_string( pixelColor ) + ", float: " + std::to_string( floatVal ) + ", ior: " + std::to_string( 1.0f + floatVal * 2.0f ) );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< uchar4 >& texture, const int x, const int y )
{
    m_rendererCore.copyTexture( *m_debugFrameU4, texture, x, y, 1, 1 );
    m_debugFrameU4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), x, y, 1, 1 );

    const uchar4 pixelColor = m_debugFrameU4->getPixel( x, y );

    const float4 floatVal = float4( (float4)pixelColor / 255.0f );
    setWindowTitle( "uchar: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")"
                    + ", float: (" + std::to_string( floatVal.x ) + ", " + std::to_string( floatVal.y ) + ", " + std::to_string( floatVal.z ) + ", " + std::to_string( floatVal.w ) + ")" );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< float >& texture, const int x, const int y )
{
    m_rendererCore.copyTexture( *m_debugFrameF1, texture, x, y, 1, 1 );
    m_debugFrameF1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), x, y, 1, 1 );

    const float pixelColor = m_debugFrameF1->getPixel( x, y );

    setWindowTitle( "float: " + std::to_string( pixelColor ) );
}

void Application::debugDisplayTextureValue( const Texture2DGeneric< float4 >& texture, const int x, const int y )
{
    m_rendererCore.copyTexture( *m_debugFrameF4, texture, x, y, 1, 1 );
    m_debugFrameF4->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), x, y, 1, 1 );

    const float4 pixelColor = m_debugFrameF4->getPixel( x, y );

    setWindowTitle( "float: (" + std::to_string( pixelColor.x ) + ", " + std::to_string( pixelColor.y ) + ", " + std::to_string( pixelColor.z ) + ", " + std::to_string( pixelColor.w ) + ")" );
}

void Application::debugDisplayTexturesValue( const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& textures, const int x, const int y )
{
    std::string debugString = "ior: ";

    for ( auto& texture : textures ) {
        m_rendererCore.copyTexture( *m_debugFrameU1, *texture, x, y, 1, 1 );
        m_debugFrameU1->loadGpuToCpu( *m_frameRenderer.getDeviceContext().Get(), x, y, 1, 1 );

        const unsigned char pixelColor = m_debugFrameU1->getPixel( x, y );
        const float floatVal = (float)pixelColor / 255.0f;

        debugString += std::to_string( 1.0f + floatVal * 2.0f ) + " -> ";
    }
    
    setWindowTitle( debugString );
}

void Application::setWindowTitle( const std::string& title )
{
    SetWindowText( m_windowHandle, StringUtil::widen( title ).c_str() );
}

LRESULT CALLBACK Application::windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch (msg) {
	case WM_CREATE:
		SetTimer(hWnd, inputTimerId, inputTimerInterval, 0);

		if (windowsMessageReceiver) windowsMessageReceiver->onStart();
		//ShowCursor( true );
		break;

	case WM_DESTROY:
		KillTimer(hWnd, inputTimerId);

		if (windowsMessageReceiver) windowsMessageReceiver->onExit();

		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (windowsMessageReceiver) {
			int newWidth = LOWORD(lParam);
			int newHeight = HIWORD(lParam);
			windowsMessageReceiver->onResize(newWidth, newHeight);
		}
		break;
	case WM_MOVE:
		if (windowsMessageReceiver) {
			int posX = LOWORD(lParam);
			int posY = HIWORD(lParam);
			windowsMessageReceiver->onMove(posX, posY);
		}
		break;
	case WM_SETFOCUS:
		if (windowsMessageReceiver) windowsMessageReceiver->onFocusChange(true);
		break;
	case WM_KILLFOCUS:
		if (windowsMessageReceiver) windowsMessageReceiver->onFocusChange(false);
		break;
	case WM_KEYDOWN:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onKeyboardButton((int)wParam, true);
			windowsMessageReceiver->onKeyPress((int)wParam);
		}
		break;
	case WM_KEYUP:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onKeyboardButton((int)wParam, false);
		}
		break;
	case WM_MOUSEMOVE:
		//if ( windowsMessageReceiver ) {
		//}
		break;
	case WM_LBUTTONDOWN:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onMouseButton(0, true);
			windowsMessageReceiver->onMouseButtonPress(0);
		}
		break;
	case WM_LBUTTONUP:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onMouseButton(0, false);
		}
		break;
	case WM_MBUTTONDOWN:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onMouseButton(1, true);
			windowsMessageReceiver->onMouseButtonPress(1);
		}
		break;
	case WM_MBUTTONUP:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onMouseButton(1, false);
		}
		break;
	case WM_RBUTTONDOWN:
		if (windowsMessageReceiver) {
			windowsMessageReceiver->m_inputManager.onMouseButton(2, true);
			windowsMessageReceiver->onMouseButtonPress(2);
		}
		break;
	case WM_RBUTTONUP:
		if (windowsMessageReceiver) {
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

            try {
                windowsMessageReceiver->onDragAndDropFile( path );
            } catch ( ... ) {}
        }
			
		break;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

void Application::onStart( ) {
}

void Application::onExit( ) {
}

void Application::onResize( int newWidth, int newHeight ) {
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
	this->m_windowFocused = windowFocused;
}

void Application::onKeyPress( int key )
{
    if ( key == InputManager::Keys::l ) 
    {
        if ( m_scene )
        {
            float3 lightPosition = m_camera.getPosition() + m_camera.getDirection();

            std::shared_ptr< Light > light = std::make_shared<PointLight>( lightPosition );
            light->setColor( float3( 1.0f, 1.0f, 1.0f ) );
            m_scene->addLight( light );

            m_selectedLights.clear();
            m_selectedLights.push_back( light );
        }
    } 
    
    if ( key == InputManager::Keys::ctrl || key == InputManager::Keys::s ) 
    {
        if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::s ) )
        {
            if ( m_selectedBlockActors.empty() && m_selectedSkeletonActors.empty() )
            {
                if ( m_scene && !m_scenePath.empty() ) {
                    saveScene( m_scenePath );
                }

                if ( !m_cameraPath.empty() ) {
                    saveCamera( m_cameraPath );
                }
            }
            else
            {
                saveSelectedModels();
            }
        }
    }

    // Ctrl + A - Select all actors and lights.
    if ( key == InputManager::Keys::ctrl || key == InputManager::Keys::a ) 
    {
        if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::a ) ) 
        {
            m_selectedBlockActors.clear();
            m_selectedSkeletonActors.clear();
            m_selectedLights.clear();

            for ( auto& actor : m_scene->getActors() )
            {
                if ( actor->getType() == Actor::Type::BlockActor )
                    m_selectedBlockActors.push_back( std::dynamic_pointer_cast< BlockActor >( actor ) );
                else if ( actor->getType() == Actor::Type::SkeletonActor )
                    m_selectedSkeletonActors.push_back( std::dynamic_pointer_cast< SkeletonActor >( actor ) );
            }

            for ( auto& light : m_scene->getLights() )
                m_selectedLights.push_back( light );
        }
    }

    if ( key == InputManager::Keys::delete_ ) 
    {
        if ( m_scene ) 
        {
            for ( auto& actor : m_selectedBlockActors )
                m_scene->removeActor( actor );

            for ( auto& actor : m_selectedSkeletonActors )
                m_scene->removeActor( actor );

            for ( auto& light : m_selectedLights )
            {
                m_scene->removeLight( light );
            }

            m_selectedBlockActors.clear();
            m_selectedSkeletonActors.clear();
            m_selectedLights.clear();
        }
    }

    // Clone the actors, but share their models with the original actors.
    if ( key == InputManager::Keys::c && m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) 
    {
        if ( m_scene ) 
        {
            std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
            std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
            newBlockActors.reserve( m_selectedBlockActors.size() );
            newSkeletonActors.reserve( m_selectedSkeletonActors.size() );

            for ( auto& actor : m_selectedBlockActors )
            {
                newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); // Clone the actor.
                m_scene->addActor( newBlockActors.back() );
            }
            
            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); // Clone the actor.
                m_scene->addActor( newSkeletonActors.back() );
            }

            // Select new actors.
            m_selectedBlockActors    = std::move( newBlockActors );
            m_selectedSkeletonActors = std::move( newSkeletonActors );
        }
    }

    // Clone the actors and clone their models.
    if ( key == InputManager::Keys::c && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
    {
        if ( m_scene ) 
        {
            std::vector< std::shared_ptr< BlockActor > >    newBlockActors;
            std::vector< std::shared_ptr< SkeletonActor > > newSkeletonActors;
            newBlockActors.reserve( m_selectedBlockActors.size() );
            newSkeletonActors.reserve( m_selectedSkeletonActors.size() );

            for ( auto& actor : m_selectedBlockActors ) 
            {
                newBlockActors.push_back( std::make_shared< BlockActor >( *actor ) ); // Clone the actor.
                newBlockActors.back()->setModel( std::make_shared< BlockModel >( *actor->getModel() ) ); // Clone it's model.
                m_scene->addActor( newBlockActors.back() );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                newSkeletonActors.push_back( std::make_shared< SkeletonActor >( *actor ) ); // Clone the actor.
                newSkeletonActors.back()->setModel( std::make_shared< SkeletonModel >( *actor->getModel() ) ); // Clone it's model.
                m_scene->addActor( newSkeletonActors.back() );
            }
        }
    }

    // Merge selected actors/models/meshes etc.
    if ( key == InputManager::Keys::m && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
    {
        mergeSelectedActors();
    }

    /*static float normalThresholdChange = 0.01f;
    if ( inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::n ) )
    {
        if ( key == InputManager::Keys::plus )
            combiningRenderer.setNormalThreshold( combiningRenderer.getNormalThreshold() + normalThresholdChange );
        else if ( key == InputManager::Keys::minus )
            combiningRenderer.setNormalThreshold( combiningRenderer.getNormalThreshold() - normalThresholdChange );
    }

    static float positionThresholdChange = 0.01f;
    if ( inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::p ) )
    {
        if ( key == InputManager::Keys::plus )
            combiningRenderer.setPositionThreshold( combiningRenderer.getPositionThreshold() + positionThresholdChange );
        else if ( key == InputManager::Keys::minus )
            combiningRenderer.setPositionThreshold( combiningRenderer.getPositionThreshold() - positionThresholdChange );
    }*/

    // Enable/disable casting shadow for an actor.
    if ( key == InputManager::Keys::h && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) {
        if ( m_scene ) 
        {
            bool castShadows = true;
            if ( !m_selectedBlockActors.empty() )
                castShadows = !m_selectedBlockActors.front()->isCastingShadows();
            else if ( !m_selectedSkeletonActors.empty() ) 
                castShadows = !m_selectedSkeletonActors.front()->isCastingShadows();

            for ( auto& actor : m_selectedBlockActors )
                actor->setCastingShadows( castShadows );

            for ( auto& actor : m_selectedSkeletonActors )
                actor->setCastingShadows( castShadows );
        }
    }

    if ( key == InputManager::Keys::tilde )
        m_renderer.setActiveViewType( Renderer::View::Final );
    else if ( key == InputManager::Keys::one )
        m_renderer.setActiveViewType( Renderer::View::Shaded );
    else if ( key == InputManager::Keys::two )
        m_renderer.setActiveViewType( Renderer::View::Depth );
    else if ( key == InputManager::Keys::three )
        m_renderer.setActiveViewType( Renderer::View::Position );
    else if ( key == InputManager::Keys::four )
        m_renderer.setActiveViewType( Renderer::View::Emissive );
    else if ( key == InputManager::Keys::five )
        m_renderer.setActiveViewType( Renderer::View::Albedo );
    else if ( key == InputManager::Keys::six )
        m_renderer.setActiveViewType( Renderer::View::Normal );
    else if ( key == InputManager::Keys::seven )
        m_renderer.setActiveViewType( Renderer::View::Metalness );
    else if ( key == InputManager::Keys::eight )
        m_renderer.setActiveViewType( Renderer::View::Roughness );
    else if ( key == InputManager::Keys::nine )
        m_renderer.setActiveViewType( Renderer::View::IndexOfRefraction );
    else if ( key == InputManager::Keys::zero )
        m_renderer.setActiveViewType( Renderer::View::RayDirections );
    else if ( key == InputManager::Keys::f1 )
        m_renderer.setActiveViewType( Renderer::View::Contribution );
    else if ( key == InputManager::Keys::f2 )
        m_renderer.setActiveViewType( Renderer::View::CurrentRefractiveIndex );
	else if ( key == InputManager::Keys::f12 )
		m_renderer.setActiveViewType( Renderer::View::Test );


    if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::shift ) )
        m_renderer.setMaxLevelCount( std::min( 10, m_renderer.getMaxLevelCount() + 1 ) );
    else if ( key == InputManager::Keys::minus && m_inputManager.isKeyPressed( InputManager::Keys::shift ) )
        m_renderer.setMaxLevelCount( std::max( 0, m_renderer.getMaxLevelCount() - 1 ) );
    else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::r ) )
        m_renderer.activateNextViewLevel( true );
    else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::t )  )
        m_renderer.activateNextViewLevel( false );
    else if ( key == InputManager::Keys::minus )
        m_renderer.activatePrevViewLevel();

    if ( key == InputManager::Keys::enter ) 
        m_debugRenderAlpha = !m_debugRenderAlpha;
}

void Application::onMouseButtonPress( int button )
{
    if ( button == 0 ) // On left button press.
    {
        // Calculate mouse pos relative to app window top-left corner.
        int2 mousePos = m_inputManager.getMousePos();
        mousePos -= m_windowPosition; 

        // TODO: FOV shouldn't be hardcoded.
        const float fieldOfView = (float)MathUtil::pi / 4.0f;

        std::shared_ptr< Actor > pickedActor;
        std::shared_ptr< Light > pickedLight;
        std::tie( pickedActor, pickedLight ) = pickActorOrLight( *m_scene, m_camera, float2( (float)mousePos.x, (float)mousePos.y ), (float)m_screenWidth, (float)m_screenHeight, fieldOfView );

        if ( pickedActor && pickedActor->getType() == Actor::Type::BlockActor ) 
        {
            const auto it = std::find( m_selectedBlockActors.begin(), m_selectedBlockActors.end(), pickedActor );
            
            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) // Add picked actor to selection (if not selected already).
            {
                if ( it == m_selectedBlockActors.end() )
                    m_selectedBlockActors.push_back( std::static_pointer_cast< BlockActor >( pickedActor ) );
            }
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) // Remove picked actor from selection.
            {
                if ( it != m_selectedBlockActors.end() )
                    m_selectedBlockActors.erase( it );
            }
            else // Clear selection and select the picked actor.
            {
                m_selectedBlockActors.clear();
                m_selectedSkeletonActors.clear();
                m_selectedLights.clear();

                m_selectedBlockActors.push_back( std::static_pointer_cast< BlockActor >( pickedActor ) );
            }
            
        } 
        else if ( pickedActor && pickedActor->getType() == Actor::Type::SkeletonActor ) 
        {
            const auto it = std::find( m_selectedSkeletonActors.begin(), m_selectedSkeletonActors.end(), pickedActor );

            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) // Add picked actor to selection (if not selected already).
            {
                if ( it == m_selectedSkeletonActors.end() )
                    m_selectedSkeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( pickedActor ) );
            } 
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) // Remove picked actor from selection.
            {
                if ( it != m_selectedSkeletonActors.end() )
                    m_selectedSkeletonActors.erase( it );
            } 
            else // Clear selection and select the picked actor.
            {
                m_selectedBlockActors.clear();
                m_selectedSkeletonActors.clear();
                m_selectedLights.clear();

                m_selectedSkeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( pickedActor ) );
            }
        } 
        else if ( pickedLight ) 
        {
            const auto it = std::find( m_selectedLights.begin(), m_selectedLights.end(), pickedLight );

            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) // Add picked light to selection (if not selected already).
            {
                if ( it == m_selectedLights.end() )
                    m_selectedLights.push_back( pickedLight );
            } 
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) // Remove picked actor from selection.
            {
                if ( it != m_selectedLights.end() )
                    m_selectedLights.erase( it );
            }
            else // Clear selection and select the picked light.
            {
                m_selectedBlockActors.clear();
                m_selectedSkeletonActors.clear();
                m_selectedLights.clear();

                m_selectedLights.push_back( pickedLight );
            }
        }
    }
}

std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light > > 
        Application::pickActorOrLight( const CScene& scene, const Camera& camera, const float2& targetPixel,
                                       const float screenWidth, const float screenHeight, const float fieldOfView )
{
    float43 cameraPose;
    cameraPose.setRow1( camera.getRight() );
    cameraPose.setRow2( camera.getUp() );
    cameraPose.setRow3( camera.getDirection() );
    cameraPose.setTranslation( camera.getPosition() );

    const float2 screenDimensions( screenWidth, screenHeight );

    const float3 rayOriginWorld = camera.getPosition();
    const float3 rayDirWorld    = MathUtil::getRayDirectionAtPixel( cameraPose, targetPixel, screenDimensions, fieldOfView );

    float                    minHitDistance = FLT_MAX;
    std::shared_ptr< Actor > hitActor;
    std::shared_ptr< Light > hitLight;

    bool  hitOccurred = false;
    float hitDistance = FLT_MAX;

    for ( const std::shared_ptr< Light >& light : scene.getLights() )
    {
        float3 boxMinLocal( -0.25f, -0.25f, -0.25f );
        float3 boxMaxLocal(  0.25f,  0.25f,  0.25f );

        float43 pose = float43::IDENTITY;
        pose.setTranslation( light->getPosition() );

        std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBoundingBox( rayOriginWorld, rayDirWorld, pose, boxMinLocal, boxMaxLocal );

        if ( hitOccurred && hitDistance < minHitDistance ) {
            minHitDistance = hitDistance;
            hitLight       = light;
        }
    }

    for( const std::shared_ptr< Actor >& actor : scene.getActors() )
    {
        if ( actor->getType() == Actor::Type::BlockActor )
        {
            const std::shared_ptr< BlockActor >& blockActor = std::static_pointer_cast< BlockActor >( actor );
            if ( !blockActor->getModel() || !blockActor->getModel()->getMesh() )
                continue;

            float3 boxMinLocal, boxMaxLocal;
            std::tie( boxMinLocal, boxMaxLocal ) = blockActor->getModel()->getMesh()->getBoundingBox();
        
            std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBoundingBox( rayOriginWorld, rayDirWorld, blockActor->getPose(), boxMinLocal, boxMaxLocal );

            if ( hitOccurred && hitDistance < minHitDistance ) {
                minHitDistance = hitDistance;
                hitActor       = blockActor;
            }
        }
        else if ( actor->getType() == Actor::Type::SkeletonActor )
        {
            const std::shared_ptr< SkeletonActor >& skeletonActor = std::static_pointer_cast< SkeletonActor >( actor );
            if ( !skeletonActor->getModel() || !skeletonActor->getModel()->getMesh() )
                continue;

            float3 boxMinLocal, boxMaxLocal;
            std::tie( boxMinLocal, boxMaxLocal ) = skeletonActor->getModel()->getMesh()->getBoundingBox();
        
            std::tie( hitOccurred, hitDistance ) = MathUtil::intersectRayWithBoundingBox( rayOriginWorld, rayDirWorld, skeletonActor->getPose(), boxMinLocal, boxMaxLocal );

            if ( hitOccurred && hitDistance < minHitDistance ) {
                minHitDistance = hitDistance;
                hitActor       = skeletonActor;
            }
        }
    }

    if ( hitActor )
        return std::make_tuple( hitActor, nullptr );
    else
        return std::make_tuple( nullptr, hitLight );
}

void Application::onDragAndDropFile( std::string filePath )
{
	std::string currentPath;
	{
		const DWORD charCount = GetCurrentDirectoryW( 0, nullptr );
		std::vector<wchar_t> currentPathBufferW;
		currentPathBufferW.resize( charCount );
		GetCurrentDirectoryW( charCount, (LPWSTR)currentPathBufferW.data( ) );
		std::wstring currentPathW( currentPathBufferW.data( ), charCount - 1 );
		currentPath = StringUtil::narrow( currentPathW );
	}

	// Transform absolute path into relative path.
	if ( filePath.find( currentPath ) == 0 )
		filePath = filePath.substr( currentPath.size( ) );

	// Remove "\\" from the beginning of the path.
	if ( filePath.find( "\\" ) == 0 )
		filePath = filePath.substr( 1 );

	const size_t dotIndex = filePath.rfind( "." );
	if ( dotIndex == std::string::npos )
		return;

    std::string filePathWithoutExtension = StringUtil::toLowercase( filePath.substr( 0, dotIndex ) );
	std::string extension                = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

	std::array< const std::string, 4 > blockMeshExtensions     = { "obj", "dae", "fbx", "blockmesh" };
	std::array< const std::string, 1 > skeletonkMeshExtensions = { "dae" };
	std::array< const std::string, 9 > textureExtensions       = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff", "tif" };
    std::array< const std::string, 1 > blockModelExtensions    = { "blockmodel" };
    std::array< const std::string, 1 > skeletonModelExtensions = { "skeletonmodel" };
    std::array< const std::string, 1 > animationExtensions     = { "xaf" };
    std::array< const std::string, 1 > sceneExtensions         = { "scene" };
    std::array< const std::string, 1 > cameraExtensions        = { "camera" };

	bool isBlockMesh     = false;
	bool isSkeletonMesh  = false;
	bool isTexture       = false;
    bool isBlockModel    = false;
    bool isSkeletonModel = false;
    bool isAnimation     = false;
    bool isScene         = false;
    bool isCamera        = false;

	for ( const std::string& blockMeshExtension : blockMeshExtensions ) {
		if ( extension.compare( blockMeshExtension ) == 0 )
			isBlockMesh = true;
	}

	for ( const std::string& skeletonkMeshExtension : skeletonkMeshExtensions ) {
		if ( extension.compare( skeletonkMeshExtension ) == 0 )
			isSkeletonMesh = true;
	}
	
	for ( const std::string& textureExtension : textureExtensions ) {
		if ( extension.compare( textureExtension ) == 0 )
			isTexture = true;
	}

    for ( const std::string& blockModelExtension : blockModelExtensions ) {
        if ( extension.compare( blockModelExtension ) == 0 )
            isBlockModel = true;
    }

    for ( const std::string& skeletonModelExtension : skeletonModelExtensions ) {
        if ( extension.compare( skeletonModelExtension ) == 0 )
            isSkeletonModel = true;
    }

    for ( const std::string& animationExtension : animationExtensions ) {
        if ( extension.compare( animationExtension ) == 0 )
            isAnimation = true;
    }

    for ( const std::string& sceneExtension : sceneExtensions ) {
        if ( extension.compare( sceneExtension ) == 0 )
            isScene = true;
    }

    for ( const std::string& cameraExtension : cameraExtensions ) {
        if ( extension.compare( cameraExtension ) == 0 )
            isCamera = true;
    }

    const bool replaceAsset = m_inputManager.isKeyPressed( InputManager::Keys::ctrl );

    float43 pose = float43::IDENTITY;
    pose.setTranslation( m_camera.getPosition() + m_camera.getDirection() );

	if ( isBlockMesh ) {
		BlockMeshFileInfo::Format format = BlockMeshFileInfo::Format::OBJ;

		if (      extension.compare( "obj" ) == 0 )       format = BlockMeshFileInfo::Format::OBJ;
		else if ( extension.compare( "dae" ) == 0 )       format = BlockMeshFileInfo::Format::DAE;
        else if ( extension.compare( "fbx" ) == 0 )       format = BlockMeshFileInfo::Format::FBX;
        else if ( extension.compare( "blockmesh" ) == 0 ) format = BlockMeshFileInfo::Format::BLOCKMESH;

        // Note: Bad support for multiple meshes in a single file. Because of how AssetManager works, they have to be parsed one by one from the same file.
        // So the file has to be re-parsed for each mesh it contains. Not a big problem normally, because only one mesh should be in a single file.

        // We try to parse meshes with increasing index in file until it fails.
        for (int indexInFile = 0; true; ++indexInFile)
        {
            try 
            {
                BlockMeshFileInfo fileInfo( filePath, format, indexInFile, false, false, false );
                std::shared_ptr< BlockMesh > mesh = std::static_pointer_cast< BlockMesh >( m_assetManager.getOrLoad( fileInfo ) );

                if ( !mesh->getBvhTree() )
                    mesh->buildBvhTree();

                if ( !mesh->isInGpuMemory( ) ) {
                    mesh->loadCpuToGpu( *m_frameRenderer.getDevice().Get() );
                    mesh->loadBvhTreeToGpu( *m_frameRenderer.getDevice().Get() );
                }

                if ( replaceAsset ) {
                    // Replace a mesh of an existing model.
                    if ( m_selectedBlockActors.size() == 1 && m_selectedBlockActors[ 0 ]->getModel() ) {
                        m_selectedBlockActors[ 0 ]->getModel( )->setMesh( mesh );
                        
                        break; // If replacing a mesh - read only the one with index 0 in file.
                    }
                } else {
                    // Add new actor to the scene.
                    m_selectedBlockActors.clear();
                    m_selectedBlockActors.push_back( std::make_shared< BlockActor >( std::make_shared< BlockModel >(), pose ) );
                    m_selectedBlockActors[ 0 ]->getModel( )->setMesh( mesh );
                    m_scene->addActor( m_selectedBlockActors[ 0 ] );
                }

                // Save mesh to .blockmesh format for future use.
                if ( format != BlockMeshFileInfo::Format::BLOCKMESH )
                    mesh->saveToFile( filePathWithoutExtension + (indexInFile != 0 ? "_" + std::to_string( indexInFile ) : "") + ".blockmesh", BlockMeshFileInfo::Format::BLOCKMESH );
            }
            catch ( ... )
            {
                break;
            }
        }
	}

	if ( isSkeletonMesh ) {
		SkeletonMeshFileInfo::Format format = SkeletonMeshFileInfo::Format::DAE;

		if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

        // Note: Bad support for multiple meshes in a single file. Because of how AssetManager works, they have to be parsed one by one from the same file.
        // So the file has to be re-parsed for each mesh it contains. Not a big problem normally, because only one mesh should be in a single file.

        // We try to parse meshes with increasing index in file until it fails.
        for ( int indexInFile = 0; true; ++indexInFile ) {
            try {
                SkeletonMeshFileInfo fileInfo( filePath, format, 0, false, false, false );
                std::shared_ptr<SkeletonMesh> mesh = std::static_pointer_cast<SkeletonMesh>(m_assetManager.getOrLoad( fileInfo ));  
                if ( !mesh->isInGpuMemory( ) )
                    mesh->loadCpuToGpu( *m_frameRenderer.getDevice( ).Get() );

                // #TODO: add replacing mesh? How to deal with non-matching animation?
                // Add new actor to the scene.
                m_selectedSkeletonActors.clear();
                m_selectedSkeletonActors.push_back( std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>( ), pose ) );
                m_selectedSkeletonActors[ 0 ]->getModel( )->setMesh( mesh );
                m_selectedSkeletonActors[ 0 ]->resetSkeletonPose();
                m_scene->addActor( m_selectedSkeletonActors[ 0 ] );
            } catch ( ... ) {
                break;
            }
        }
	}

	if ( isTexture ) {
		Texture2DFileInfo::Format format = Texture2DFileInfo::Format::BMP;

		if ( extension.compare( "bmp" ) == 0 )       format = Texture2DFileInfo::Format::BMP;
		else if ( extension.compare( "dds" ) == 0 )  format = Texture2DFileInfo::Format::DDS;
		else if ( extension.compare( "jpg" ) == 0 )  format = Texture2DFileInfo::Format::JPEG;
        else if ( extension.compare( "jpeg" ) == 0 ) format = Texture2DFileInfo::Format::JPEG;
		else if ( extension.compare( "png" ) == 0 )  format = Texture2DFileInfo::Format::PNG;
		else if ( extension.compare( "raw" ) == 0 )  format = Texture2DFileInfo::Format::RAW;
		else if ( extension.compare( "tga" ) == 0 )  format = Texture2DFileInfo::Format::TGA;
		else if ( extension.compare( "tiff" ) == 0 ) format = Texture2DFileInfo::Format::TIFF;
        else if ( extension.compare( "tif" ) == 0 )  format = Texture2DFileInfo::Format::TIFF;
        
        Texture2DFileInfo::PixelType pixelType;
        if ( filePath.find( "_A." ) != std::string::npos ||
             filePath.find( "_N." ) != std::string::npos ||
             filePath.find( "_E." ) != std::string::npos )
        {
            pixelType = Texture2DFileInfo::PixelType::UCHAR4;
        }
        else if ( filePath.find( "_AL." ) != std::string::npos ||
                  filePath.find( "_M." ) != std::string::npos ||
                  filePath.find( "_R." ) != std::string::npos ||
                  filePath.find( "_I." ) != std::string::npos)
        {
            pixelType = Texture2DFileInfo::PixelType::UCHAR;
        }
        else
            return; // Unrecognized texture type.

        Texture2DFileInfo fileInfo( filePath, format, pixelType );
        std::shared_ptr< Asset > textureAsset = m_assetManager.getOrLoad( fileInfo );

		if ( filePath.find( "_A." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {  
                if ( replaceAsset )
                    actor->getModel( )->removeAllAlbedoTextures();

                actor->getModel( )->addAlbedoTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllAlbedoTextures();

                actor->getModel( )->addAlbedoTexture( modelTexture );
            }
        } 
        else if ( filePath.find( "_AL." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllAlphaTextures();

                actor->getModel( )->addAlphaTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllAlphaTextures();

                actor->getModel( )->addAlphaTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_M." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {    
                if ( replaceAsset )
                    actor->getModel( )->removeAllMetalnessTextures();

                actor->getModel( )->addMetalnessTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllMetalnessTextures();

                actor->getModel( )->addMetalnessTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_N." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {  
                if ( replaceAsset )
                    actor->getModel( )->removeAllNormalTextures();

                actor->getModel( )->addNormalTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllNormalTextures();

                actor->getModel( )->addNormalTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_R." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {   
                if ( replaceAsset )
                    actor->getModel( )->removeAllRoughnessTextures();

                actor->getModel( )->addRoughnessTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllRoughnessTextures();

                actor->getModel( )->addRoughnessTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_E." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

           for ( auto& actor : m_selectedBlockActors ) 
           {    
                if ( replaceAsset )
                    actor->getModel( )->removeAllEmissiveTextures();

                actor->getModel( )->addEmissiveTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllEmissiveTextures();

                actor->getModel( )->addEmissiveTexture( modelTexture );
            }
        } 
        else if ( filePath.find( "_I." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            for ( auto& actor : m_selectedBlockActors ) 
            {   
                if ( replaceAsset )
                    actor->getModel( )->removeAllRefractiveIndexTextures();

                actor->getModel( )->addRefractiveIndexTexture( modelTexture );
            }

            for ( auto& actor : m_selectedSkeletonActors ) 
            {
                if ( replaceAsset )
                    actor->getModel( )->removeAllRefractiveIndexTextures();

                actor->getModel( )->addRefractiveIndexTexture( modelTexture );
            }
		}
	}

    if ( isBlockModel ) {

        BlockModelFileInfo fileInfo( filePath, BlockModelFileInfo::Format::BLOCKMODEL, 0 );
        std::shared_ptr<BlockModel> model = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( fileInfo ) );

        if ( model->getMesh() && !model->getMesh()->getBvhTree() )
        {
            model->getMesh()->buildBvhTree();
            model->getMesh()->loadBvhTreeToGpu( *m_frameRenderer.getDevice( ).Get() );
        }

        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( *m_frameRenderer.getDevice( ).Get(), *m_frameRenderer.getDeviceContext( ).Get() );

        // Add new actor to the scene.
        m_selectedBlockActors.clear();
        m_selectedBlockActors.push_back( std::make_shared<BlockActor>( model, pose ) );
        m_scene->addActor( m_selectedBlockActors[ 0 ] );
    }

    if ( isSkeletonModel ) 
    {
        SkeletonModelFileInfo fileInfo( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, 0 );
        std::shared_ptr<SkeletonModel> model = std::static_pointer_cast<SkeletonModel>(m_assetManager.getOrLoad( fileInfo ));
        if ( !model->isInGpuMemory( ) )
            model->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext( ).Get() );

        // Add new actor to the scene.
        m_selectedSkeletonActors.clear();
        m_selectedSkeletonActors.push_back( std::make_shared<SkeletonActor>( model, pose ) );
        m_scene->addActor( m_selectedSkeletonActors[ 0 ] );
    }

    if ( isAnimation ) 
    {
        for ( auto& actor : m_selectedSkeletonActors )
        {
            if ( actor->getModel() && actor->getModel()->getMesh() ) {
                SkeletonMeshFileInfo&     referenceMeshFileInfo = actor->getModel()->getMesh()->getFileInfo();
                SkeletonAnimationFileInfo fileInfo( filePath, SkeletonAnimationFileInfo::Format::XAF, referenceMeshFileInfo, false );

                std::shared_ptr< SkeletonAnimation > animation = std::static_pointer_cast<SkeletonAnimation>( m_assetManager.getOrLoad( fileInfo ) );

                if ( animation )
                    actor->startAnimation( animation );
            }
        }
    }

    if ( (isBlockMesh || isTexture) && m_selectedBlockActors.size() == 1 && m_selectedBlockActors[ 0 ]->getModel( ) && m_selectedBlockActors[ 0 ]->getModel( )->isInCpuMemory( ) )
        m_selectedBlockActors[ 0 ]->getModel()->saveToFile( "Assets/Models/new.blockmodel" );

    if ( (isSkeletonMesh || isTexture) && m_selectedSkeletonActors.size() == 1 && m_selectedSkeletonActors[ 0 ]->getModel( ) && m_selectedSkeletonActors[ 0 ]->getModel( )->isInCpuMemory( ) )
        m_selectedSkeletonActors[ 0 ]->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );

    if ( isScene ) {
        loadScene( filePath );
        m_scenePath = filePath;
    }

    if ( isCamera ) {
        loadCamera( filePath );
        m_cameraPath = filePath;
    }
}

void Application::loadCamera( std::string path )
{
    m_camera = *FreeCamera::createFromFile( path );
}

void Application::saveCamera( std::string path )
{
    m_camera.saveToFile( path );
}

void Application::loadScene( std::string path )
{
    std::shared_ptr< std::vector < std::shared_ptr< FileInfo > > > fileInfos;

    std::tie( m_scene, fileInfos ) = CScene::createFromFile( path );

    // Load all assets.
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        m_assetManager.loadAsync( *fileInfo );

    // Wait for all assets to be loaded.
    const float timeout = 60.0f;
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        m_assetManager.getWhenLoaded( fileInfo->getAssetType(), fileInfo->getPath(), fileInfo->getIndexInFile(), timeout );

    // Swap actors' empty models with the loaded models. Create BVH trees. Load models to GPU.
    const std::unordered_set< std::shared_ptr< Actor > > sceneActors = m_scene->getActors();
    for ( const std::shared_ptr< Actor >& actor : sceneActors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) 
        {

            const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>(actor);
            if ( blockActor->getModel() ) 
            {
                const BlockModelFileInfo& fileInfo = blockActor->getModel()->getFileInfo();
                std::shared_ptr<BlockModel> blockModel = std::static_pointer_cast<BlockModel>(m_assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( blockModel ) 
                {
                    // Build BVH tree and load it to GPU.
                    if ( blockModel->getMesh() ) 
                    {
                        if ( !blockModel->getMesh()->getBvhTree() )
                            blockModel->getMesh()->buildBvhTree();

                        blockModel->getMesh()->loadBvhTreeToGpu( *m_frameRenderer.getDevice().Get() );
                    }

                    blockModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext( ).Get() );
                    blockActor->setModel( blockModel ); // Swap an empty model with a loaded model.
                } 
                else 
                {
                    throw std::exception( "Application::onStart - failed to load one of the scene's models." );
                }
            }
        } 
        else if ( actor->getType() == Actor::Type::SkeletonActor ) 
        {
            const std::shared_ptr<SkeletonActor>& skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
            if ( skeletonActor->getModel() ) {
                const SkeletonModelFileInfo& fileInfo = skeletonActor->getModel()->getFileInfo();
                std::shared_ptr<SkeletonModel> skeletonModel = std::static_pointer_cast<SkeletonModel>(m_assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( skeletonModel ) 
                {
                    skeletonModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext( ).Get() );
                    skeletonActor->setModel( skeletonModel ); // Swap an empty model with a loaded model.
                } 
                else 
                {
                    throw std::exception( "Application::onStart - failed to load one of the scene's models." );
                }
            }
        }
    }
}

void Application::saveScene( std::string path )
{
    m_scene->saveToFile( path );
}

void Application::mergeSelectedActors()
{
    std::vector< std::shared_ptr< BlockModel > > models;
    models.reserve( m_selectedBlockActors.size() );

    for ( auto& actor : m_selectedBlockActors )
        models.push_back( actor->getModel() );

    std::shared_ptr< BlockModel > mergedModel = ModelUtil::mergeModels( models, *m_frameRenderer.getDevice().Get() );

    if ( mergedModel->getMesh() )
    {
        mergedModel->getMesh()->recalculateBoundingBox();

        if (!mergedModel->getMesh()->getBvhTree() ) 
        {
            mergedModel->getMesh()->buildBvhTree();
            mergedModel->getMesh()->loadBvhTreeToGpu( *m_frameRenderer.getDevice().Get() );
        }
    }

    if ( !mergedModel->isInGpuMemory() )
        mergedModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext().Get() );

    { // Save merged model, mesh and textures to files.
        std::string meshPath = "Assets/Meshes/merged.obj";
        std::string texturePath = "Assets/Textures/merged";

        if ( mergedModel->getMesh() ) {
            mergedModel->getMesh()->saveToFile( meshPath, BlockMeshFileInfo::Format::OBJ );
            
            mergedModel->getMesh()->getFileInfo().setPath( meshPath );
            mergedModel->getMesh()->getFileInfo().setFormat( BlockMeshFileInfo::Format::OBJ );
            mergedModel->getMesh()->getFileInfo().setIndexInFile( 0 );
        }

        int textureIndex = 0;
        for ( auto& texture : mergedModel->getAlphaTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_AL.tiff";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );
            
            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getEmissiveTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_E.png";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getAlbedoTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_A.png";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getMetalnessTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_M.tiff";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getRoughnessTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_R.tiff";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getNormalTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_N.png";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::PNG );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::PNG );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR4 );
        }

        textureIndex = 0;
        for ( auto& texture : mergedModel->getRefractiveIndexTextures() ) {
            std::string path = texturePath + "_" + std::to_string( textureIndex++ ) + "_I.tiff";
            texture.getTexture()->saveToFile( path, Texture2DFileInfo::Format::TIFF );

            texture.getTexture()->getFileInfo().setPath( path );
            texture.getTexture()->getFileInfo().setFormat( Texture2DFileInfo::Format::TIFF );
            texture.getTexture()->getFileInfo().setPixelType( Texture2DFileInfo::PixelType::UCHAR );
        }
        
        mergedModel->saveToFile( "Assets/Models/merged.blockmodel" );
    }

    float43 pose( float43::IDENTITY );
    pose.setTranslation( m_camera.getPosition() );

    // Add new actor to the scene.
    m_selectedBlockActors.clear();
    m_selectedBlockActors.push_back( std::make_shared< BlockActor >( mergedModel, pose ) );
    m_scene->addActor( m_selectedBlockActors[ 0 ] );
}

void Application::saveSelectedModels()
{
    for ( auto& actor : m_selectedBlockActors )
    {
        const std::shared_ptr< BlockModel > model = actor->getModel();
        if ( model )
        {
            std::string path = model->getFileInfo().getPath();

            // If there is no existing path for model - use mesh's path.
            if ( path.empty() && model->getMesh() )
            {
                path = model->getMesh()->getFileInfo().getPath();
                const int dotPos = (int)path.rfind(".");
                if ( dotPos != std::string::npos )
                    path = path.replace( path.begin() + dotPos, path.end(), ".blockmodel" );
            }

            if ( !path.empty() )
                model->saveToFile( path );
        }
    }
}
