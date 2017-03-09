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

using namespace Engine1;

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;

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
    m_debugDisplayedMipmapLevel( 0 ),
    m_debugWireframeMode( false ),
    m_renderText( true ),
    m_renderFps( true ),
    m_slowmotionMode( false ),
    m_snappingMode( false ),
    m_assetManager(),
    m_sceneManager( m_assetManager )
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->m_applicationInstance = applicationInstance;

	setupWindow();

    const int parallelThreadCount = std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) : 1;

	m_frameRenderer.initialize( m_windowHandle, m_screenWidth, m_screenHeight, m_fullscreen, m_verticalSync );
	m_rendererCore.initialize( *m_frameRenderer.getDeviceContext( ).Get() );
    m_assetManager.initialize( parallelThreadCount, parallelThreadCount, m_frameRenderer.getDevice() );
    m_profiler.initialize( m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext() );

    m_sceneManager.initialize( m_frameRenderer.getDevice(), m_frameRenderer.getDeviceContext() );

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
        const auto path = AssetPathManager::getPathForFileName( "light_bulb.blockmodel" );
		BlockModelFileInfo lightModelFileInfo( path, BlockModelFileInfo::Format::BLOCKMODEL, 0);
		lightModel = std::static_pointer_cast<BlockModel>( m_assetManager.getOrLoad( lightModelFileInfo ) );
		lightModel->loadCpuToGpu( *m_frameRenderer.getDevice().Get(), *m_frameRenderer.getDeviceContext().Get() );
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

	Font font( uint2(m_screenWidth, m_screenHeight) );
	font.loadFromFile( AssetPathManager::getPathForFileName( "consola.ttf" ), 35 );

    Font font2( uint2(m_screenWidth, m_screenHeight) );
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

        // Disable locking when connecting through Team Viewer.
        bool lockCursor = false;

		while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( WM_QUIT == msg.message ) run = false;
		}

        ///////////////////////////////////////////////////////////////////////////////
        bool modifyingScene = false;
        { // Bunch of stupid code to be moved elsewhere.
            // Translate / rotate the selected actors.
            if ( m_windowFocused && ( !m_sceneManager.getSelectedBlockActors().empty() || !m_sceneManager.getSelectedSkeletonActors().empty() ) ) 
            {
                const int2 mouseMove = m_inputManager.getMouseMove();

                const float3 sensitivity(
                    m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                    m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                    m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

                // Move along horizontal and vertical axes added together.
                float mouseTotalMove = (float)(mouseMove.x - mouseMove.y);

                if ( m_snappingMode )
                {
                    const float rotationSnapAngleDegrees = m_slowmotionMode ? 1.0f : 5.0f;
                    const float translationSnapDist      = m_slowmotionMode ? 0.01f : 0.1f;

                    if ( fabs( mouseTotalMove ) > 1.0f )
                    {
                        if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) 
                        {
                            for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                                actor->getPose().rotate( MathUtil::sign( mouseTotalMove ) * sensitivity * ( rotationSnapAngleDegrees / 360.0f ) * MathUtil::piTwo );

                            for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                                actor->getPose().rotate( MathUtil::sign( mouseTotalMove ) * sensitivity * ( rotationSnapAngleDegrees / 360.0f ) * MathUtil::piTwo );

                            modifyingScene = true;
                        } 
                        else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) 
                        {
                            for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                                actor->getPose().translate( mouseTotalMove * translationSnapDist * sensitivity );

                            for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                                actor->getPose().translate( mouseTotalMove * translationSnapDist * sensitivity );

                            modifyingScene = true;
                        }
                    }
                }
                else
                {
                    const float translationSensitivity = m_slowmotionMode ? 0.00001f : 0.0002f;
                    const float rotationSensitivity    = m_slowmotionMode ? 0.00001f : 0.0001f;

                    if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) 
                    {
                        for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                            actor->getPose().rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                        for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                            actor->getPose().rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                        modifyingScene = true;
                    } 
                    else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) 
                    {
                        for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                            actor->getPose().translate( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity );

                        for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                            actor->getPose().translate( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity );

                        modifyingScene = true;
                    }
                }
            }

            // Translate / rotate the selected light.
            if ( m_windowFocused && !m_sceneManager.getSelectedLights().empty() ) 
            {
                const float   translationSensitivity = m_slowmotionMode ? 0.00005f : 0.0002f;
                const float   rotationSensitivity    = m_slowmotionMode ? 0.00001f : 0.0001f;
                const int2    mouseMove              = m_inputManager.getMouseMove();

                float mouseTotalMove = (float)(mouseMove.x - mouseMove.y);

                const float3 sensitivity(
                    m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                    m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                    m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                    );

                if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) 
                {
                    for ( auto& light : m_sceneManager.getSelectedLights() )
                        light->setPosition( light->getPosition() + ( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity ) );
                
                    modifyingScene = true;
                }
                else if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) )
                {
                    for ( auto& light : m_sceneManager.getSelectedLights() )
                    {
                        if ( light->getType() != Light::Type::SpotLight )
                            continue;

                        auto& spotLight = static_cast< SpotLight& >( *light );

                        float3 direction = spotLight.getDirection();
                        direction.rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                        spotLight.setDirection( direction );
                    }

                    modifyingScene = true;
                }
            }

            // Set camera to align with a spot light.
            if ( m_windowFocused && m_sceneManager.getSelectedLights().size() == 1 && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::l ) )
            {
                const Light& light = *m_sceneManager.getSelectedLights()[ 0 ];

                if ( light.getType() == Light::Type::SpotLight )
                {
                    const SpotLight& spotLight = static_cast< const SpotLight& >( light );

                    m_sceneManager.getCamera().setFieldOfView( spotLight.getConeAngle() );
                    m_sceneManager.getCamera().setPosition( spotLight.getPosition() );
                    m_sceneManager.getCamera().setDirection( spotLight.getDirection() );
                }
            }

            // Update the camera.
            if ( m_windowFocused && !modifyingScene && m_inputManager.isMouseButtonPressed( InputManager::MouseButtons::right ) ) 
            { 
                const float cameraRotationSensitivity = m_slowmotionMode ? 0.00002f : 0.0001f;
                const float acceleration              = m_slowmotionMode ? 0.02f : 0.25f;

                if ( m_inputManager.isKeyPressed( InputManager::Keys::w ) )      m_sceneManager.getCamera().accelerateForward( (float)frameTimeMs * acceleration );
                else if ( m_inputManager.isKeyPressed( InputManager::Keys::s ) ) m_sceneManager.getCamera().accelerateReverse( (float)frameTimeMs * acceleration );
                if ( m_inputManager.isKeyPressed( InputManager::Keys::d ) )      m_sceneManager.getCamera().accelerateRight( (float)frameTimeMs * acceleration );
                else if ( m_inputManager.isKeyPressed( InputManager::Keys::a ) ) m_sceneManager.getCamera().accelerateLeft( (float)frameTimeMs * acceleration );
                if ( m_inputManager.isKeyPressed( InputManager::Keys::e ) )      m_sceneManager.getCamera().accelerateUp( (float)frameTimeMs * acceleration );
                else if ( m_inputManager.isKeyPressed( InputManager::Keys::q ) ) m_sceneManager.getCamera().accelerateDown( (float)frameTimeMs * acceleration );

			    int2 mouseMove = m_inputManager.getMouseMove();
			    m_sceneManager.getCamera().rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTimeMs * cameraRotationSensitivity );
		    }

            m_inputManager.lockCursor( lockCursor );
            m_inputManager.updateMouseState();

		    m_sceneManager.getCamera().updateState( (float)frameTimeMs );

            { // Update animations.
                const std::unordered_set< std::shared_ptr<Actor> >& sceneActors = m_sceneManager.getScene().getActors();

                for ( const std::shared_ptr<Actor>& actor : sceneActors )
                {
                    if ( actor->getType() != Actor::Type::SkeletonActor )
                        continue;

                    const std::shared_ptr< SkeletonActor > skeletonActor = std::dynamic_pointer_cast< SkeletonActor >( actor );

                    skeletonActor->updateAnimation( (float)frameTimeMs / 1000.0f );
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////////////

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >  frameUchar;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >         frameUchar4;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >         frameFloat4;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float2  > >        frameFloat2;
        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float  > >         frameFloat;

        m_renderer.clear();

        if ( modifyingScene )
            m_renderer.renderShadowMaps( m_sceneManager.getScene() );

        std::tie( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat )
            = m_renderer.renderScene( m_sceneManager.getScene(), m_sceneManager.getCamera(), m_debugWireframeMode, m_sceneManager.getSelectedBlockActors(),
                                      m_sceneManager.getSelectedSkeletonActors(), m_sceneManager.getSelectedLights(), m_sceneManager.getSelectionVolumeMesh() );

        const int2 mousePos = m_inputManager.getMousePos();

        try 
        {
            if ( frameUchar ) 
            {
                m_rendererCore.copyTexture( *ucharDisplayFrame, *frameUchar, 0, 0, frameUchar->getWidth(), frameUchar->getHeight() );
		        m_frameRenderer.renderTexture( *ucharDisplayFrame, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false, m_debugDisplayedMipmapLevel );

                if ( m_inputManager.isMouseButtonPressed(0) ) 
                    debugDisplayTextureValue( *frameUchar, mousePos.x, mousePos.y );
            } 
            else if ( frameUchar4 )
            {
                if ( m_debugRenderAlpha )
		            m_frameRenderer.renderTextureAlpha( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false, m_debugDisplayedMipmapLevel );
                else
                    m_frameRenderer.renderTexture( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false, m_debugDisplayedMipmapLevel );

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *frameUchar4, mousePos.x, mousePos.y );
            } 
            else if ( frameFloat4 )
            {
                m_frameRenderer.renderTexture( *frameFloat4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false, m_debugDisplayedMipmapLevel );

                if ( m_inputManager.isMouseButtonPressed( 0 ) )
                    debugDisplayTextureValue( *frameFloat4, mousePos.x, mousePos.y );
            }
            else if ( frameFloat2 )
            {
                m_frameRenderer.renderTexture( *frameFloat2, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, false, m_debugDisplayedMipmapLevel );
            }
            else if ( frameFloat ) 
            {
                m_frameRenderer.renderTexture( *frameFloat, 0.0f, 0.0f, (float)frameFloat->getWidth(), (float)frameFloat->getHeight(), false, m_debugDisplayedMipmapLevel );

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

        if ( updateProfiling )
        {
            profilingLastRefreshTime.reset();

            { // Render profiling results.
                totalFrameTimeCPU                             = (float)frameTimeMs;
                totalFrameTimeGPU                             = m_profiler.getEventDuration( Profiler::GlobalEventType::Frame );
                deferredRenderingTime                         = m_profiler.getEventDuration( Profiler::GlobalEventType::DeferredRendering );
                mainMipmapGenerationForPositionAndNormalsTime = m_profiler.getEventDuration( Profiler::StageType::Main, Profiler::EventTypePerStage::MipmapGenerationForPositionAndNormals );

                for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, (int)m_renderer.getMaxLevelCount() + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
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

        { // Render FPS.
            std::stringstream ss;
            ss << "FPS: " << (int)( 1000.0 / totalFrameTimeCPU ) << " / " << totalFrameTimeCPU << "ms";
            
            if ( m_renderFps )
                frameUchar4 = m_renderer.renderText( ss.str(), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render current view.
            std::stringstream ss;
            ss << "View: " << Renderer::viewToString( m_renderer.getActiveViewType() );

            if ( m_renderFps )
                frameUchar4 = m_renderer.renderText( ss.str(), font2, float2( -500.0f, 350.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render some debug options.
            std::stringstream ss;
            ss << "Use separable shadow blur: " << ( m_renderer.debugIsUsingSeparableShadowsBlur() ? "enabled" : "disabled" );

            if ( m_renderFps )
                frameUchar4 = m_renderer.renderText( ss.str(), font2, float2( 150.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        { // Render profiling results.
            std::stringstream ss, ss2;
            ss << std::fixed << std::setprecision( 2 );
            ss2 << std::fixed << std::setprecision( 2 );
            ss << "Profiling: \n";
            ss << "Total: " << totalFrameTimeGPU << " ms \n";

            // Print global events duration.
            ss << "DeferredRendering: " << deferredRenderingTime << "ms " << ( deferredRenderingTime / totalFrameTimeGPU ) * 100.0f << "% \n";
            ss << "CopyFrameToFinalRenderTarget: " << mainMipmapGenerationForPositionAndNormalsTime << "ms " << ( mainMipmapGenerationForPositionAndNormalsTime / totalFrameTimeGPU ) * 100.0f << "% \n";

            for ( int stage = (int)Profiler::StageType::Main; stage < pow( 2, (int)m_renderer.getMaxLevelCount() + 1 ) && stage < (int)Profiler::StageType::MAX_VALUE; ++stage )
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

            if ( m_renderText )
                frameUchar4 = m_renderer.renderText( ss.str(), font2, float2( -500.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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

            if ( selectedMeshesCount == 1 )
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

            if ( selectedLightsCount == 1 && selectedMeshesCount == 0 )
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

            if ( m_renderText )
                frameUchar4 = m_renderer.renderText( ss.str(), font2, float2( 120.0f, 250.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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

        if ( frameUchar4 )
            m_frameRenderer.renderTexture( *frameUchar4, 0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, true );

		m_frameRenderer.displayFrame();

        m_profiler.endEvent( Profiler::GlobalEventType::Frame );
        m_profiler.endFrameProfiling();

        updateProfiling = false;
        m_profiler.pauseProfiling();

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
    // [\] - Hide/show text.
    if ( key == InputManager::Keys::backslash )
    {
        m_renderText = !m_renderText;
    }

    // [/] - Hide/show FPS counter.
    if ( key == InputManager::Keys::slash ) {
        m_renderFps = !m_renderFps;
    }

    // [ L + P ] - Add point light.
    // [ L + S ] - Add spot light.
    if ( key == InputManager::Keys::l && m_inputManager.isKeyPressed( InputManager::Keys::p ) ) 
    {
        m_sceneManager.addPointLight();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }
    else if ( key == InputManager::Keys::l && m_inputManager.isKeyPressed( InputManager::Keys::s ) )
    {
        m_sceneManager.addSpotLight();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }
    
    // [Ctrl + S] - save scene or selected models.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::s ) &&
       ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::s ) ) )
    {
        m_sceneManager.saveSceneOrSelectedModels();
    }

    // [Ctrl + A] - Select all actors and lights.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::a ) &&
       ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::a ) ) )
    {
        m_sceneManager.selectAll();
    }

    // [Ctrl + D] - Unselect all.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::d ) &&
         ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::d ) ) ) {
        m_sceneManager.clearSelection();
    }

    // [Shift + A] - Select all actors inside the selection volume.
    if ( ( key == InputManager::Keys::shift || key == InputManager::Keys::a ) &&
         ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) && m_inputManager.isKeyPressed( InputManager::Keys::a ) ) ) {
        m_sceneManager.selectAllInsideSelectionVolume();
    }

    // [Delete] - Delete selected actors and lights.
    if ( key == InputManager::Keys::delete_ ) 
    {
        m_sceneManager.deleteSelected();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }

    // [+] or [-] - Change light brightness.
    if ( !m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && !m_inputManager.isKeyPressed( InputManager::Keys::shift ) 
         && !m_inputManager.isKeyPressed( InputManager::Keys::p ) )
    {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) 
        {
            const float3 colorChange =
                ( key == InputManager::Keys::plus ) ?
                float3( 0.05f, 0.05f, 0.05f ) :
                float3( -0.05f, -0.05f, -0.05f );

            m_sceneManager.modifySelectedLightsColor( colorChange );
        }
    }

    // [Shift] and ( [+] or [-] ) - Modify spot light cone angle.
    if ( !m_sceneManager.getSelectedLights().empty() && m_inputManager.isKeyPressed( InputManager::Keys::shift ) 
         && !m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
    {
        const float sensitivity = 0.001f;

        float change = 0.0f;
        if ( m_inputManager.isKeyPressed( InputManager::Keys::plus ) ) {
            change = sensitivity;
        } else if ( m_inputManager.isKeyPressed( InputManager::Keys::minus ) ) {
            change = -sensitivity;
        }

        if ( change != 0.0f ) {
            for ( auto& light : m_sceneManager.getSelectedLights() ) {
                if ( light->getType() != Light::Type::SpotLight )
                    continue;

                auto& spotLight = static_cast<SpotLight&>( *light );

                spotLight.setConeAngle( std::min( MathUtil::pi, std::max( 0.01f, spotLight.getConeAngle() + change ) ) );
            }
        }
    }

    // [Shift] and [Ctrl] and ( [+] or [-] ) - Modify light emitter radius.
    if ( !m_sceneManager.getSelectedLights().empty() && m_inputManager.isKeyPressed( InputManager::Keys::shift ) 
        && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
    {
        const float sensitivity = 0.005f;

        float change = 0.0f;
        if ( m_inputManager.isKeyPressed( InputManager::Keys::plus ) ) {
            change = sensitivity;
        } else if ( m_inputManager.isKeyPressed( InputManager::Keys::minus ) ) {
            change = -sensitivity;
        }

        if ( change != 0.0f ) 
        {
            for ( auto& light : m_sceneManager.getSelectedLights() ) {
                light->setEmitterRadius( std::min( 1.0f, std::max( 0.0f, light->getEmitterRadius() + change ) ) );
            }
        }
    }

    // [P] and ( [+] or [-] ) - Modify shadow blur position threshold and normal threshold.
    if ( m_sceneManager.getSelectedLights().size() == 1 ) {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) 
        {
            const float positionThresholdChange = 
                ( key == InputManager::Keys::plus ) ?
                0.001f : - 0.001f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::p ) )
                BlurShadowsComputeShader::s_positionThreshold += positionThresholdChange;
        }
    }

    // [Enter] - Enable/disable light sources.
    if ( key == InputManager::Keys::enter && !m_inputManager.isKeyPressed( InputManager::Keys::shift ) )
        m_sceneManager.enableDisableSelectedLights();

    // [Shift + Enter] - Enable/disable casting shadows for lights and actors.
    if ( key == InputManager::Keys::enter && m_inputManager.isKeyPressed( InputManager::Keys::shift ) )
    {
        m_sceneManager.enableDisableCastingShadowsForSelected();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }

    // [Shift + C] - Clone the actors, but share their models with the original actors.
    if ( key == InputManager::Keys::c && m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) 
        m_sceneManager.cloneInstancesOfSelectedActors();

    // [Ctrl + C] - Clone the actors and clone their models.
    if ( key == InputManager::Keys::c && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
        m_sceneManager.cloneSelectedActors();

    // [Ctrl + M] - Merge selected actors/models/meshes etc.
    if ( key == InputManager::Keys::m && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
        m_sceneManager.mergeSelectedActors();

    // [Enter] - Render alpha.
    if ( key == InputManager::Keys::enter )
        m_debugRenderAlpha = !m_debugRenderAlpha;

    // [Page up/Page down] - Switch displayed mipmap.
    if ( key == InputManager::Keys::pageUp )
        m_debugDisplayedMipmapLevel = std::max( 0, m_debugDisplayedMipmapLevel - 1 );
    else if ( key == InputManager::Keys::pageDown )
        m_debugDisplayedMipmapLevel = m_debugDisplayedMipmapLevel + 1;

    // [Backspace] - Render in wireframe mode.
    if ( key == InputManager::Keys::backspace )
        m_debugWireframeMode = !m_debugWireframeMode;

    // [Caps Lock] - Enable slowmotion mode.
    if ( key == InputManager::Keys::capsLock )
        m_slowmotionMode = !m_slowmotionMode;

    // [Ctrl + B] - Rebuild bounding box and BVH.
    if ( key == InputManager::Keys::b && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) )
        m_sceneManager.rebuildBoundingBoxAndBVH();

    // [Spacebar] - Enable/disable snapping when rotating/translating actors.
    if ( key == InputManager::Keys::spacebar )
        m_snappingMode = !m_snappingMode;

    // [left/right] - Select next/prev actor or light.
    if ( key == InputManager::Keys::right )
        m_sceneManager.selectNext();
    else if ( key == InputManager::Keys::left )
        m_sceneManager.selectPrev();

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

    if ( key == InputManager::Keys::tilde ) {
        m_renderer.setActiveViewType( Renderer::View::Final );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::one ) {
        m_renderer.setActiveViewType( Renderer::View::Shaded );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::two ) {
        m_renderer.setActiveViewType( Renderer::View::Depth );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::three ) {
        m_renderer.setActiveViewType( Renderer::View::Position );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::four ) {
        m_renderer.setActiveViewType( Renderer::View::Emissive );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::five ) {
        m_renderer.setActiveViewType( Renderer::View::Albedo );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::six ) {
        m_renderer.setActiveViewType( Renderer::View::Normal );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::seven ) {
        m_renderer.setActiveViewType( Renderer::View::Metalness );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::eight ) {
        m_renderer.setActiveViewType( Renderer::View::Roughness );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::nine ) {
        m_renderer.setActiveViewType( Renderer::View::IndexOfRefraction );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::zero ) {
        m_renderer.setActiveViewType( Renderer::View::RayDirections );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f1 ) {
        m_renderer.setActiveViewType( Renderer::View::Contribution );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f2 ) {
        m_renderer.setActiveViewType( Renderer::View::CurrentRefractiveIndex );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f3 ) {
        m_renderer.setActiveViewType( Renderer::View::Preillumination );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f4 ) {
        m_renderer.setActiveViewType( Renderer::View::HardIllumination );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f5 ) {
        m_renderer.setActiveViewType( Renderer::View::SoftIllumination );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f6 ) {
        m_renderer.setActiveViewType( Renderer::View::BlurredIllumination );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f7 ) {
        m_renderer.setActiveViewType( Renderer::View::SpotlightDepth );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f8 ) {
        m_renderer.setActiveViewType( Renderer::View::DistanceToOccluder );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f9 ) {
        m_renderer.setActiveViewType( Renderer::View::FinalDistanceToOccluder );
        m_debugDisplayedMipmapLevel = 0;
    } else if ( key == InputManager::Keys::f11 ) {
        m_renderer.setActiveViewType( Renderer::View::BloomBrightPixels );
        m_debugDisplayedMipmapLevel = 0;
    } /*else if ( key == InputManager::Keys::f12 ) {
        m_renderer.setActiveViewType( Renderer::View::Test );
        m_debugDisplayedMipmapLevel = 0;
    }*/

    if ( m_sceneManager.isSelectionEmpty() )
    {
        if ( key == InputManager::Keys::up && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) )
            m_renderer.setMaxLevelCount( std::min( 10, m_renderer.getMaxLevelCount() + 1 ) );
        else if ( key == InputManager::Keys::down && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) )
            m_renderer.setMaxLevelCount( std::max( 0, m_renderer.getMaxLevelCount() - 1 ) );
        else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::r ) )
            m_renderer.activateNextViewLevel( true );
        else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::t ) )
            m_renderer.activateNextViewLevel( false );
        else if ( key == InputManager::Keys::minus )
            m_renderer.activatePrevViewLevel();
        else if ( key == InputManager::Keys::b )
            m_renderer.debugSetUseSeparableShadowsBlur( !m_renderer.debugIsUsingSeparableShadowsBlur() );
    }
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
        std::tie( pickedActor, pickedLight ) = m_sceneManager.pickActorOrLight( float2( (float)mousePos.x, (float)mousePos.y ), (float)m_screenWidth, (float)m_screenHeight, fieldOfView );

        if ( pickedActor )
        {
            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) {       // Add picked actor to selection.
                m_sceneManager.selectActor( pickedActor );
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) { // Remove picked actor from selection.
                m_sceneManager.unselectActor( pickedActor );
            } else {                                                                // Clear selection and select the picked actor.
                m_sceneManager.clearSelection();
                m_sceneManager.selectActor( pickedActor );
            }
        }
        else if ( pickedLight ) 
        {
            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) {       // Add picked light to selection.
                m_sceneManager.selectLight( pickedLight );
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) { // Remove picked light from selection.
                m_sceneManager.unselectLight( pickedLight );
            } else {                                                                // Clear selection and select the picked light.
                m_sceneManager.clearSelection();
                m_sceneManager.selectLight( pickedLight );
            }
        }
    }
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

    // Temporarily always replace assets. Holding Ctrl is too hard...
    const bool replaceSelected = !m_sceneManager.getSelectedBlockActors().empty() || !m_sceneManager.getSelectedSkeletonActors().empty(); //m_inputManager.isKeyPressed( InputManager::Keys::ctrl );
    const bool invertZ         = !m_inputManager.isKeyPressed( InputManager::Keys::shift );

    m_sceneManager.loadAsset( filePath, replaceSelected, invertZ );

    m_renderer.renderShadowMaps( m_sceneManager.getScene() );
}

int2 Application::screenPosToWindowPos( int2 screenPos ) const
{
    POINT pos;
    pos.x = screenPos.x;
    pos.y = screenPos.y;

    ScreenToClient(m_windowHandle, &pos );

    return int2( pos.x, pos.y );
}
