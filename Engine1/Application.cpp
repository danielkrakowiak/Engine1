#include "Application.h"

#include <sstream>
#include <exception>

#include <Windows.h>

#include "Camera.h"

#include "MathUtil.h"
#include "StringUtil.h"

#include "BlockMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

#include "BlockActor.h"
#include "SkeletonActor.h"
#include "Scene.h"

#include "RenderTargetTexture2D.h"

#include "Timer.h"

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;

Application::Application() :
	direct3DRendererCore(),
	direct3DFrameRenderer( direct3DRendererCore ),
	direct3DDefferedRenderer( direct3DRendererCore ),
	initialized( false ),
	applicationInstance( nullptr ),
	windowHandle( nullptr ),
	deviceContext( nullptr ),
	fullscreen( false ),
	screenWidth( 1024 ),
	screenHeight( 768 ),
	verticalSync( false ),
	displayFrequency( 60 ),
	screenColorDepth( 32 ),
	zBufferDepth( 32 ),
	windowFocused( false ),
    scene( std::make_shared<Scene>() )
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->applicationInstance = applicationInstance;

	setupWindow();

	direct3DFrameRenderer.initialize( windowHandle, screenWidth, screenHeight, fullscreen, verticalSync );
	direct3DDefferedRenderer.initialize( screenWidth, screenHeight, direct3DFrameRenderer.getDevice(), direct3DFrameRenderer.getDeviceContext() );
	direct3DRendererCore.initialize( direct3DFrameRenderer.getDeviceContext( ) );

	initialized = true;
}

void Application::setupWindow() {
	const LPCTSTR className = L"Engine1";
	const LPCTSTR wndCaption = L"Engine1";

	WNDCLASSEX wc;
	int style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	ZeroMemory( &wc, sizeof( WNDCLASSEX ) );
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = applicationInstance;
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

	if ( fullscreen ) {
		DEVMODE screen = { 0 };

		screen.dmSize = sizeof( DEVMODE );
		screen.dmPelsWidth = screenWidth;
		screen.dmPelsHeight = screenHeight;
		screen.dmBitsPerPel = screenColorDepth;
		screen.dmDisplayFrequency = displayFrequency;
		screen.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		ChangeDisplaySettings( &screen, CDS_FULLSCREEN );

		style |= WS_POPUP;
	} else {
		style |= WS_OVERLAPPEDWINDOW;
	}

	DWORD exStyle = WS_EX_ACCEPTFILES; // Allow drag&drop files.

	windowHandle = CreateWindowEx( exStyle, className, wndCaption, style, 0, 0, screenWidth, screenHeight, NULL, NULL, applicationInstance, NULL );

	deviceContext = GetDC( windowHandle );

	int pixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof( PIXELFORMATDESCRIPTOR ),
		1,
		PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		screenColorDepth,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		zBufferDepth,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	pixelFormat = ChoosePixelFormat( deviceContext, &pfd );
	SetPixelFormat( deviceContext, pixelFormat, &pfd );
}

void Application::show() {
	if ( !initialized ) throw std::exception( "Application::show called on uninitialized Application" );

	ShowWindow( windowHandle, SW_SHOW );
}

void Application::run() {
	if ( !initialized ) throw std::exception( "Application::run called on uninitialized Application" );

	bool run = true;
	MSG msg;

    // Add 'axis' actor to the scene.
	std::shared_ptr<BlockMesh> axisMesh = BlockMesh::createFromFile( "../Engine1/Assets/Meshes/dx-coordinate-axises.obj", BlockMeshFileInfo::Format::OBJ, true, true, true ).front();
	axisMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );
    std::shared_ptr<BlockActor> axisActor = std::make_shared<BlockActor>( std::make_shared<BlockModel>() );
    axisActor->getModel()->setMesh( axisMesh );
    scene->addActor( axisActor );

    // Setup the camera.
	camera.setUp( float3( 0.0f, 1.0f, 0.0f ) );
	camera.setPosition( float3( 0.0f, 0.0f, -30.0f ) );

	Font font( uint2(screenWidth, screenHeight) );
	font.loadFromFile( "../Engine1/Assets/Fonts/DoulosSILR.ttf", 35 );

	double frameTime = 0.0;

	while ( run ) {
		Timer frameStartTime;

		inputManager.lockCursor( windowFocused );

		inputManager.updateMouseState();

		while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( WM_QUIT == msg.message ) run = false;
		}

        // Update the camera.
        if ( windowFocused ) { 
			const float cameraRotationSensitivity = 0.00005f;

			if      ( inputManager.isKeyPressed( InputManager::Keys::w ) ) camera.accelerateForward( (float)frameTime );
			else if ( inputManager.isKeyPressed( InputManager::Keys::s ) ) camera.accelerateReverse( (float)frameTime );
			if      ( inputManager.isKeyPressed( InputManager::Keys::d ) ) camera.accelerateRight( (float)frameTime );
			else if ( inputManager.isKeyPressed( InputManager::Keys::a ) ) camera.accelerateLeft( (float)frameTime );
			if      ( inputManager.isKeyPressed( InputManager::Keys::e ) ) camera.accelerateUp( (float)frameTime );
			else if ( inputManager.isKeyPressed( InputManager::Keys::q ) ) camera.accelerateDown( (float)frameTime );

			int2 mouseMove = inputManager.getMouseMove( );
			camera.rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTime * cameraRotationSensitivity );
		}

		camera.updateState( (float)frameTime );

		direct3DDefferedRenderer.clearRenderTargets( float4( 0.2f, 0.2f, 0.2f, 1.0f ), 1.0f );

        float44 viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint( ), camera.getPosition( ), camera.getUp( ) );

        // Render the scene.
        const std::unordered_set< std::shared_ptr<Actor> >& actors = scene->getActors();
        for ( const std::shared_ptr<Actor> actor : actors )
		{ 
            if ( actor->getType() == Actor::Type::BlockActor ) {
                const std::shared_ptr<BlockActor> blockActor = std::static_pointer_cast<BlockActor>( actor );
                const std::shared_ptr<BlockModel> blockModel = blockActor->getModel();

                if ( blockModel->isInGpuMemory( ) )
                    direct3DDefferedRenderer.render( *blockModel, blockActor->getPose(), viewMatrix );
                else if ( blockModel->getMesh( ) && blockModel->getMesh( )->isInGpuMemory( ) )
                    direct3DDefferedRenderer.render( *blockModel->getMesh( ), blockActor->getPose( ), viewMatrix );

            } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
                const std::shared_ptr<SkeletonActor> skeletonActor = std::static_pointer_cast<SkeletonActor>( actor );
                const std::shared_ptr<SkeletonModel> skeletonModel = skeletonActor->getModel( );

                if ( skeletonModel->isInGpuMemory( ) )
                    direct3DDefferedRenderer.render( *skeletonModel, skeletonActor->getPose( ), viewMatrix, skeletonActor->getSkeletonPose() );
                else if ( skeletonModel->getMesh( ) && skeletonModel->getMesh( )->isInGpuMemory( ) )
                    direct3DDefferedRenderer.render( *skeletonModel->getMesh( ), skeletonActor->getPose( ), viewMatrix, skeletonActor->getSkeletonPose( ) );
            }
		}

		{ // Render FPS.
			std::stringstream ss;
			ss << "FPS: " << (int)(1000.0 / frameTime) << " / " << frameTime << "ms";
			direct3DDefferedRenderer.render( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		{ // Render camera state.
			std::stringstream ss;
			ss << "Speed: " << camera.getSpeed( ).x << " / " << camera.getSpeed( ).y << " / " << camera.getSpeed( ).z;
			//direct3DRenderer.renderText( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		{ // Render keyboard state.
			std::stringstream ss;
			ss << "";
			if ( inputManager.isKeyPressed( InputManager::Keys::w ) ) ss << "W";
			if ( inputManager.isKeyPressed( InputManager::Keys::s ) ) ss << "S";
			if ( inputManager.isKeyPressed( InputManager::Keys::a ) ) ss << "A";
			if ( inputManager.isKeyPressed( InputManager::Keys::d ) ) ss << "D";
				
			//direct3DRenderer.renderText( ss.str(), font, float2( -500.0f, 270.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		std::shared_ptr<RenderTargetTexture2D> renderTarget = direct3DDefferedRenderer.getRenderTarget( Direct3DDefferedRenderer::RenderTargetType::ALBEDO );

		std::shared_ptr<Texture2D> renderTargetTexture = std::dynamic_pointer_cast<Texture2D>( renderTarget );

		direct3DFrameRenderer.renderTexture( *renderTargetTexture, 0.0f, 0.0f );

		direct3DFrameRenderer.displayFrame();

		Timer frameEndTime;
		frameTime = Timer::lapse( frameEndTime, frameStartTime );
	}
}

LRESULT CALLBACK Application::windowsMessageHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch ( msg ) {
		case WM_CREATE:
			SetTimer( hWnd, inputTimerId, inputTimerInterval, 0 );

			if ( windowsMessageReceiver ) windowsMessageReceiver->onStart();
			//ShowCursor( true );
			break;

		case WM_DESTROY:
			KillTimer( hWnd, inputTimerId );

			if ( windowsMessageReceiver ) windowsMessageReceiver->onExit();

			PostQuitMessage( 0 );
			break;
		case WM_SIZE:
			if ( windowsMessageReceiver ) {
				int newWidth = LOWORD( lParam );
				int newHeight = HIWORD( lParam );
				windowsMessageReceiver->onResize( newWidth, newHeight );
			}
			break;
		case WM_SETFOCUS:
			if ( windowsMessageReceiver ) windowsMessageReceiver->onFocusChange( true );
			break;
		case WM_KILLFOCUS:
			if ( windowsMessageReceiver ) windowsMessageReceiver->onFocusChange( false );
			break;
		case WM_KEYDOWN:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onKeyboardButton( wParam, true );
			}
			break;
		case WM_KEYUP:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onKeyboardButton( wParam, false );
			}
			break;
		case WM_MOUSEMOVE:
			//if ( windowsMessageReceiver ) {
			//}
			break;
		case WM_LBUTTONDOWN:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 0, true );
			}
			break;
		case WM_LBUTTONUP:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 0, false );
			}
			break;
		case WM_MBUTTONDOWN:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 1, true );
			}
			break;
		case WM_MBUTTONUP:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 1, false );
			}
			break;
		case WM_RBUTTONDOWN:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 2, true );
			}
			break;
		case WM_RBUTTONUP:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onMouseButton( 2, false );
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
			const DWORD charCount = DragQueryFileW( dropInfo, 0, nullptr, 0 ) + 1;
			std::vector<wchar_t> pathBufferW;
			pathBufferW.resize( charCount );

			DragQueryFileW( dropInfo, 0, (LPWSTR)pathBufferW.data( ), charCount );
			std::wstring pathW( pathBufferW.data( ), charCount - 1 );
			std::string path = StringUtil::narrow( pathW );

			windowsMessageReceiver->onDragAndDropFile( path );
			break;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

void Application::onStart( ) {
	//unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
	//if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

	//AssetManager assetManager( cpuThreadCount );

	//BlockMesh mesh;

	//mesh.loadFile( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ );
	//mesh.load( );

	//std::shared_ptr<BlockMesh> mesh = assetManager.loadBlockMesh( "Assets/Meshes/Dragon.obj", AssetFileFormat::OBJ );
	//BlockMesh& mesh = AssetManager::loadBlockMesh( "Assets/Meshes/Pyramid.obj" );

	//Texture2D texture;
	//texture.loadFile( "Assets/Textures/Floor1/floor1 - diffuse.png" );
	//texture.load( TextureFileFormat::PNG );
}

void Application::onExit( ) {

}

void Application::onResize( int newWidth, int newHeight ) {

}

void Application::onFocusChange( bool windowFocused )
{
	this->windowFocused = windowFocused;
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

	const unsigned int dotIndex = filePath.rfind( "." );
	if ( dotIndex == std::string::npos )
		return;

	std::string extension = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

	std::array< const std::string, 2 > blockMeshExtensions     = { "obj", "dae" };
	std::array< const std::string, 1 > skeletonkMeshExtensions = { "dae" };
	std::array< const std::string, 8 > textureExtensions       = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff" };
    std::array< const std::string, 1 > blockModelExtensions    = { "blockmodel" };
    std::array< const std::string, 1 > skeletonModelExtensions = { "skeletonmodel" };

	bool isBlockMesh     = false;
	bool isSkeletonMesh  = false;
	bool isTexture       = false;
    bool isBlockModel    = false;
    bool isSkeletonModel = false;

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

    float43 pose = float43::IDENTITY;
    pose.setTranslation( camera.getPosition() + camera.getDirection() );

	if ( isBlockMesh ) {
		BlockMeshFileInfo::Format format;

		if (      extension.compare( "obj" ) == 0 ) format = BlockMeshFileInfo::Format::OBJ;
		else if ( extension.compare( "dae" ) == 0 ) format = BlockMeshFileInfo::Format::DAE;

		std::shared_ptr<BlockMesh> mesh = BlockMesh::createFromFile( filePath, format, 0, false, false, false );
		mesh->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

        // Add new actor to the scene.
        defaultBlockActor = std::make_shared<BlockActor>( std::make_shared<BlockModel>(), pose );
        defaultBlockActor->getModel( )->setMesh( mesh );
        scene->addActor( defaultBlockActor );
	}

	if ( isSkeletonMesh ) {
		SkeletonMeshFileInfo::Format format;

		if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

		std::shared_ptr<SkeletonMesh> mesh = SkeletonMesh::createFromFile( filePath, format, 0, false, false, false );
		mesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

        // Add new actor to the scene.
        defaultSkeletonActor = std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>( ), pose );
        defaultSkeletonActor->getModel( )->setMesh( mesh );
        defaultSkeletonActor->resetSkeletonPose();
        scene->addActor( defaultSkeletonActor );
	}

	if ( isTexture ) {
		Texture2DFileInfo::Format format;

		if ( extension.compare( "bmp" ) == 0 )       format = Texture2DFileInfo::Format::BMP;
		else if ( extension.compare( "dds" ) == 0 )  format = Texture2DFileInfo::Format::DDS;
		else if ( extension.compare( "jpg" ) == 0 )  format = Texture2DFileInfo::Format::JPEG;
        else if ( extension.compare( "jpeg" ) == 0 ) format = Texture2DFileInfo::Format::JPEG;
		else if ( extension.compare( "png" ) == 0 )  format = Texture2DFileInfo::Format::PNG;
		else if ( extension.compare( "raw" ) == 0 )  format = Texture2DFileInfo::Format::RAW;
		else if ( extension.compare( "tga" ) == 0 )  format = Texture2DFileInfo::Format::TGA;
		else if ( extension.compare( "tiff" ) == 0 ) format = Texture2DFileInfo::Format::TIFF;

		std::shared_ptr<Texture2D> texture = Texture2D::createFromFile( filePath, format );
		texture->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

		if ( filePath.find( "_A" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addAlbedoTexture( ModelTexture2D( texture ) );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addAlbedoTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_N" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addNormalTexture( ModelTexture2D( texture ) );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addNormalTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_R" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addRoughnessTexture( ModelTexture2D( texture ) );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addRoughnessTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_E" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addEmissionTexture( ModelTexture2D( texture ) );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addEmissionTexture( ModelTexture2D( texture ) );
		}
	}

    if ( isBlockModel ) {

        std::shared_ptr<BlockModel> model = BlockModel::createFromFile( filePath, BlockModelFileInfo::Format::BLOCKMODEL, true );
        model->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

        // Add new actor to the scene.
        defaultBlockActor = std::make_shared<BlockActor>( model, pose );
        scene->addActor( defaultBlockActor );
    }

    if ( isSkeletonModel ) {

        std::shared_ptr<SkeletonModel> model = SkeletonModel::createFromFile( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, true );
        model->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

        // Add new actor to the scene.
        defaultSkeletonActor = std::make_shared<SkeletonActor>( model, pose );
        scene->addActor( defaultSkeletonActor );
    }


    if ( (isBlockMesh || isTexture) && defaultBlockActor && defaultBlockActor->getModel( ) && defaultBlockActor->getModel( )->isInCpuMemory( ) )
        defaultBlockActor->getModel()->saveToFile( "Assets/Models/new.blockmodel" );

    if ( (isSkeletonMesh || isTexture) && defaultSkeletonActor && defaultSkeletonActor->getModel( ) && defaultSkeletonActor->getModel( )->isInCpuMemory( ) )
        defaultSkeletonActor->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );
}