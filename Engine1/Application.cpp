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

#include "PointLight.h"

#include "CScene.h"

#include "RenderTargetTexture2D.h"

#include "Timer.h"

Application* Application::windowsMessageReceiver = nullptr;

// Initialize external libraries.
ImageLibrary Application::imageLibrary;
FontLibrary  Application::fontLibrary;

using namespace Engine1;

Application::Application() :
	rendererCore(),
	frameRenderer( rendererCore ),
	defferedRenderer( rendererCore ),
    raytraceRenderer( rendererCore ),
    renderer( defferedRenderer, raytraceRenderer ),
	initialized( false ),
	applicationInstance( nullptr ),
	windowHandle( nullptr ),
	deviceContext( nullptr ),
	fullscreen( false ),
	screenWidth( 1024 ),
	screenHeight( 768 ),
	verticalSync( true ),
	displayFrequency( 60 ),
	screenColorDepth( 32 ),
	zBufferDepth( 32 ),
	windowFocused( false ),
    scene( std::make_shared<CScene>() ),
    assetManager( std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) : 1 )
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->applicationInstance = applicationInstance;

	setupWindow();

	frameRenderer.initialize( windowHandle, screenWidth, screenHeight, fullscreen, verticalSync );
	defferedRenderer.initialize( screenWidth, screenHeight, frameRenderer.getDevice(), frameRenderer.getDeviceContext() );
    raytraceRenderer.initialize( screenWidth, screenHeight, frameRenderer.getDevice(), frameRenderer.getDeviceContext() );
	rendererCore.initialize( frameRenderer.getDeviceContext( ) );

    // Load 'axises' model.
    BlockMeshFileInfo axisMeshFileInfo( "Assets/Meshes/dx-coordinate-axises.obj", BlockMeshFileInfo::Format::OBJ, 0, true, true, true );
    std::shared_ptr<BlockMesh> axisMesh = std::static_pointer_cast<BlockMesh>(assetManager.getOrLoad( axisMeshFileInfo ));
    axisMesh->loadCpuToGpu( frameRenderer.getDevice() );

    // Load 'light source' model.
    BlockModelFileInfo lightModelFileInfo( "Assets/Models/bulb.blockmodel", BlockModelFileInfo::Format::BLOCKMODEL, 0 );
    std::shared_ptr<BlockModel> lightModel = std::static_pointer_cast<BlockModel>(assetManager.getOrLoad( lightModelFileInfo ));
    lightModel->loadCpuToGpu( frameRenderer.getDevice() );

    renderer.initialize( axisMesh, lightModel );

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
		(BYTE)screenColorDepth,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)zBufferDepth,
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

        // Translate / rotate the default actor.
        bool movingActors = false;
        if ( windowFocused && defaultBlockActor ) {
            const float   translationSensitivity = 0.002f;
            const float   rotationSensitivity    = 0.0002f;
            const float43 currentPose            = defaultBlockActor->getPose();
            const int2    mouseMove              = inputManager.getMouseMove();

            const float3 sensitivity(
                inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

            if ( inputManager.isKeyPressed( InputManager::Keys::r ) ) {
                defaultBlockActor->getPose().rotate( (float)(mouseMove.x - mouseMove.y) * (float)frameTime * sensitivity * rotationSensitivity );
                movingActors = true;
            } else if ( inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                defaultBlockActor->getPose().translate( (float)(mouseMove.x - mouseMove.y) * (float)frameTime * sensitivity * translationSensitivity );
                movingActors = true;
            }
        }

        // Update the camera.
        if ( windowFocused && !movingActors ) { 
            const float cameraRotationSensitivity = 0.00005f;

            if ( inputManager.isKeyPressed( InputManager::Keys::w ) ) camera.accelerateForward( (float)frameTime );
            else if ( inputManager.isKeyPressed( InputManager::Keys::s ) ) camera.accelerateReverse( (float)frameTime );
            if ( inputManager.isKeyPressed( InputManager::Keys::d ) ) camera.accelerateRight( (float)frameTime );
            else if ( inputManager.isKeyPressed( InputManager::Keys::a ) ) camera.accelerateLeft( (float)frameTime );
            if ( inputManager.isKeyPressed( InputManager::Keys::e ) ) camera.accelerateUp( (float)frameTime );
            else if ( inputManager.isKeyPressed( InputManager::Keys::q ) ) camera.accelerateDown( (float)frameTime );

			int2 mouseMove = inputManager.getMouseMove( );
			camera.rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTime * cameraRotationSensitivity );
		}

		camera.updateState( (float)frameTime );

        std::shared_ptr<Texture2D> frame;

        if ( scene )
            frame = renderer.renderScene( *scene, camera );

		{ // Render FPS.
			//std::stringstream ss;
			//ss << "FPS: " << (int)(1000.0 / frameTime) << " / " << frameTime << "ms";
			//direct3DDefferedRenderer.render( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		{ // Render camera state.
			//std::stringstream ss;
			//ss << "Speed: " << camera.getSpeed( ).x << " / " << camera.getSpeed( ).y << " / " << camera.getSpeed( ).z;
			//direct3DRenderer.renderText( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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

        if ( frame )
		    frameRenderer.renderTexture( *frame, 0.0f, 0.0f );

		frameRenderer.displayFrame();

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
				windowsMessageReceiver->inputManager.onKeyboardButton( (int)wParam, true );
                windowsMessageReceiver->onKeyPress( (int)wParam );
			}
			break;
		case WM_KEYUP:
			if ( windowsMessageReceiver ) {
				windowsMessageReceiver->inputManager.onKeyboardButton( (int)wParam, false );
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
}

void Application::onExit( ) {
}

void Application::onResize( int newWidth, int newHeight ) {
    // Unused.    
    newWidth;
    newHeight;
}

void Application::onFocusChange( bool windowFocused )
{
	this->windowFocused = windowFocused;
}

void Application::onKeyPress( int key )
{
    if ( key == InputManager::Keys::l ) {
        if ( scene ) {
            float3 lightPosition = camera.getPosition() + camera.getDirection();

            scene->addLight( std::make_shared<PointLight>( lightPosition ) );
        }
    } else if ( key == InputManager::Keys::ctrl || key == InputManager::Keys::s ) {
        if ( scene && !scenePath.empty() && inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::s ) ) {
            saveScene( scenePath );
        }
    }

    if ( key == InputManager::Keys::one )
        renderer.setActiveView( Renderer::View::Albedo );
    else if ( key == InputManager::Keys::two )
        renderer.setActiveView( Renderer::View::Normal );
    else if ( key == InputManager::Keys::three )
        renderer.setActiveView( Renderer::View::RayDirections1 );
    else if ( key == InputManager::Keys::four )
        renderer.setActiveView( Renderer::View::Reflection1 );
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

	std::string extension = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

	std::array< const std::string, 2 > blockMeshExtensions     = { "obj", "dae" };
	std::array< const std::string, 1 > skeletonkMeshExtensions = { "dae" };
	std::array< const std::string, 8 > textureExtensions       = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff" };
    std::array< const std::string, 1 > blockModelExtensions    = { "blockmodel" };
    std::array< const std::string, 1 > skeletonModelExtensions = { "skeletonmodel" };
    std::array< const std::string, 1 > sceneExtensions         = { "scene" };

	bool isBlockMesh     = false;
	bool isSkeletonMesh  = false;
	bool isTexture       = false;
    bool isBlockModel    = false;
    bool isSkeletonModel = false;
    bool isScene         = false;

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

    for ( const std::string& sceneExtension : sceneExtensions ) {
        if ( extension.compare( sceneExtension ) == 0 )
            isScene = true;
    }

    float43 pose = float43::IDENTITY;
    pose.setTranslation( camera.getPosition() + camera.getDirection() );

	if ( isBlockMesh ) {
		BlockMeshFileInfo::Format format = BlockMeshFileInfo::Format::OBJ;

		if (      extension.compare( "obj" ) == 0 ) format = BlockMeshFileInfo::Format::OBJ;
		else if ( extension.compare( "dae" ) == 0 ) format = BlockMeshFileInfo::Format::DAE;

        BlockMeshFileInfo fileInfo( filePath, format, 0, false, false, false );
        std::shared_ptr<BlockMesh> mesh = std::static_pointer_cast<BlockMesh>( assetManager.getOrLoad( fileInfo ) );
        if ( !mesh->isInGpuMemory( ) )
            mesh->loadCpuToGpu( frameRenderer.getDevice() );

        // Add new actor to the scene.
        defaultBlockActor = std::make_shared<BlockActor>( std::make_shared<BlockModel>(), pose );
        defaultBlockActor->getModel( )->setMesh( mesh );
        scene->addActor( defaultBlockActor );
	}

	if ( isSkeletonMesh ) {
		SkeletonMeshFileInfo::Format format = SkeletonMeshFileInfo::Format::DAE;

		if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

        SkeletonMeshFileInfo fileInfo( filePath, format, 0, false, false, false );
        std::shared_ptr<SkeletonMesh> mesh = std::static_pointer_cast<SkeletonMesh>(assetManager.getOrLoad( fileInfo ));  
        if ( !mesh->isInGpuMemory( ) )
            mesh->loadCpuToGpu( frameRenderer.getDevice( ) );

        // Add new actor to the scene.
        defaultSkeletonActor = std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>( ), pose );
        defaultSkeletonActor->getModel( )->setMesh( mesh );
        defaultSkeletonActor->resetSkeletonPose();
        scene->addActor( defaultSkeletonActor );
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

        Texture2DFileInfo fileInfo( filePath, format );
        std::shared_ptr<Texture2D> texture = std::static_pointer_cast<Texture2D>( assetManager.getOrLoad( fileInfo ) );
        if ( !texture->isInGpuMemory( ) )
            texture->loadCpuToGpu( frameRenderer.getDevice() );

        ModelTexture2D modelTexture( texture );

		if ( filePath.find( "_A" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addAlbedoTexture( modelTexture );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addAlbedoTexture( modelTexture );
		} else if ( filePath.find( "_N" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addNormalTexture( modelTexture );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addNormalTexture( modelTexture );
		} else if ( filePath.find( "_R" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addRoughnessTexture( modelTexture );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addRoughnessTexture( modelTexture );
		} else if ( filePath.find( "_E" ) ) {
            if ( defaultBlockActor )    defaultBlockActor->getModel( )->addEmissionTexture( modelTexture );
            if ( defaultSkeletonActor ) defaultSkeletonActor->getModel( )->addEmissionTexture( modelTexture );
		}
	}

    if ( isBlockModel ) {

        BlockModelFileInfo fileInfo( filePath, BlockModelFileInfo::Format::BLOCKMODEL, 0 );
        std::shared_ptr<BlockModel> model = std::static_pointer_cast<BlockModel>( assetManager.getOrLoad( fileInfo ) );
        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( frameRenderer.getDevice( ) );

        // Add new actor to the scene.
        defaultBlockActor = std::make_shared<BlockActor>( model, pose );
        scene->addActor( defaultBlockActor );
    }

    if ( isSkeletonModel ) {

        SkeletonModelFileInfo fileInfo( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, 0 );
        std::shared_ptr<SkeletonModel> model = std::static_pointer_cast<SkeletonModel>(assetManager.getOrLoad( fileInfo ));
        if ( !model->isInGpuMemory( ) )
            model->loadCpuToGpu( frameRenderer.getDevice() );

        // Add new actor to the scene.
        defaultSkeletonActor = std::make_shared<SkeletonActor>( model, pose );
        scene->addActor( defaultSkeletonActor );
    }


    if ( (isBlockMesh || isTexture) && defaultBlockActor && defaultBlockActor->getModel( ) && defaultBlockActor->getModel( )->isInCpuMemory( ) )
        defaultBlockActor->getModel()->saveToFile( "Assets/Models/new.blockmodel" );

    if ( (isSkeletonMesh || isTexture) && defaultSkeletonActor && defaultSkeletonActor->getModel( ) && defaultSkeletonActor->getModel( )->isInCpuMemory( ) )
        defaultSkeletonActor->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );

    if ( isScene ) {
        loadScene( filePath );
        scenePath = filePath;
    }
}

void Application::loadScene( std::string path )
{
    std::shared_ptr< std::vector < std::shared_ptr< FileInfo > > > fileInfos;

    std::tie( scene, fileInfos ) = CScene::createFromFile( path );

    // Load all assets.
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        assetManager.loadAsync( *fileInfo );

    // Wait for all assets to be loaded.
    const float timeout = 60.0f;
    for ( const std::shared_ptr<FileInfo>& fileInfo : *fileInfos )
        assetManager.getWhenLoaded( fileInfo->getAssetType(), fileInfo->getPath(), fileInfo->getIndexInFile(), timeout );

    // Swap actors' empty models with the loaded models. Load models to GPU.
    const std::unordered_set< std::shared_ptr< Actor > > sceneActors = scene->getActors();
    for ( const std::shared_ptr< Actor >& actor : sceneActors ) {
        if ( actor->getType() == Actor::Type::BlockActor ) {

            const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>(actor);
            if ( blockActor->getModel() ) {
                const BlockModelFileInfo& fileInfo = blockActor->getModel()->getFileInfo();
                std::shared_ptr<BlockModel> blockModel = std::static_pointer_cast<BlockModel>(assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( blockModel ) {
                    blockModel->loadCpuToGpu( frameRenderer.getDevice() );
                    blockActor->setModel( blockModel ); // Swap an empty model with a loaded model.
                } else {
                    throw std::exception( "Application::onStart - failed to load one of the scene's models." );
                }
            }
        } else if ( actor->getType() == Actor::Type::SkeletonActor ) {

            const std::shared_ptr<SkeletonActor>& skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
            if ( skeletonActor->getModel() ) {
                const SkeletonModelFileInfo& fileInfo = skeletonActor->getModel()->getFileInfo();
                std::shared_ptr<SkeletonModel> skeletonModel = std::static_pointer_cast<SkeletonModel>(assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( skeletonModel ) {
                    skeletonModel->loadCpuToGpu( frameRenderer.getDevice() );
                    skeletonActor->setModel( skeletonModel ); // Swap an empty model with a loaded model.
                } else {
                    throw std::exception( "Application::onStart - failed to load one of the scene's models." );
                }
            }
        }
    }
}

void Application::saveScene( std::string path )
{
    scene->saveToFile( path );
}