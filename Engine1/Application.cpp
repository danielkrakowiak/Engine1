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
	rendererCore(),
	frameRenderer( rendererCore ),
    renderer( rendererCore ),
	initialized( false ),
	applicationInstance( nullptr ),
	windowHandle( nullptr ),
	deviceContext( nullptr ),
    windowPosition( 0, 0 ),
	fullscreen( false ),
	screenWidth( 1024 ),
	screenHeight( 768 ),
	verticalSync( false ),
	displayFrequency( 60 ),
	screenColorDepth( 32 ),
	zBufferDepth( 32 ),
	windowFocused( false ),
    debugRenderAlpha( false ),
    scenePath( "Assets/Scenes/new.scene" ),
    scene( std::make_shared<CScene>() ),
    assetManager()
{
	windowsMessageReceiver = this;
}

Application::~Application() {}

void Application::initialize( HINSTANCE applicationInstance ) {
	this->applicationInstance = applicationInstance;

	setupWindow();

	frameRenderer.initialize( windowHandle, screenWidth, screenHeight, fullscreen, verticalSync );
	rendererCore.initialize( *frameRenderer.getDeviceContext( ).Get() );
    assetManager.initialize( std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) * 2 : 1, frameRenderer.getDevice() );

    createUcharDisplayFrame( screenWidth, screenHeight, frameRenderer.getDevice() );

    // Load 'axises' model.
    //BlockMeshFileInfo axisMeshFileInfo( "Assets/Meshes/dx-coordinate-axises.obj", BlockMeshFileInfo::Format::OBJ, 0, false, false, false );
    //std::shared_ptr<BlockMesh> axisMesh = std::static_pointer_cast<BlockMesh>(assetManager.getOrLoad( axisMeshFileInfo ));
    //axisMesh->buildBvhTree();
    //axisMesh->loadCpuToGpu( *frameRenderer.getDevice().Get() );
    //axisMesh->loadBvhTreeToGpu( *frameRenderer.getDevice().Get() );

    //// Load 'light source' model.
    BlockModelFileInfo lightModelFileInfo( "Assets/Models/light_bulb.blockmodel", BlockModelFileInfo::Format::BLOCKMODEL, 0 );
    std::shared_ptr<BlockModel> lightModel = std::static_pointer_cast<BlockModel>(assetManager.getOrLoad( lightModelFileInfo ));
    lightModel->loadCpuToGpu( *frameRenderer.getDevice().Get(), *frameRenderer.getDeviceContext().Get() );

    renderer.initialize( screenWidth, screenHeight, frameRenderer.getDevice(), frameRenderer.getDeviceContext(), nullptr /*axisMesh*/, lightModel );

	initialized = true;
}

void Application::createUcharDisplayFrame( int imageWidth, int imageHeight, ComPtr< ID3D11Device > device )
{
    ucharDisplayFrame = std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >
        ( *device.Get(), imageWidth, imageHeight, false, true, DXGI_FORMAT_R8_TYPELESS, DXGI_FORMAT_R8_UNORM );
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
	camera.setPosition( float3( 0.0f, 4.0f, -53.0f ) );
    camera.rotate( float3( 0.0f, MathUtil::piHalf, 0.0f ) );
     
	Font font( uint2(screenWidth, screenHeight) );
	font.loadFromFile( "Assets/Fonts/DoulosSILR.ttf", 35 );

    Font font2( uint2(screenWidth, screenHeight) );
    font2.loadFromFile( "Assets/Fonts/DoulosSILR.ttf", 15 );

	double frameTimeMs = 0.0;

	while ( run ) {
		Timer frameStartTime;

        // Disable locking when connecting through Team Viewer.
        bool lockCursor = false;

		while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if ( WM_QUIT == msg.message ) run = false;
		}

        // Translate / rotate the selected actor.
        bool movingObjects = false;
        if ( windowFocused && ( selectedBlockActor || selectedSkeletonActor ) ) 
        {
            std::shared_ptr< Actor > actor;
            if ( selectedBlockActor )
                actor = selectedBlockActor;
            else
                actor = selectedSkeletonActor;

            const float   translationSensitivity = 0.002f;//0.002f;
            const float   rotationSensitivity    = 0.0002f;
            const float43 currentPose            = actor->getPose();
            const int2    mouseMove              = inputManager.getMouseMove();

            const float3 sensitivity(
                inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

            if ( inputManager.isKeyPressed( InputManager::Keys::r ) ) {
                actor->getPose().rotate( (float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * rotationSensitivity );
                movingObjects = true;
            } else if ( inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                actor->getPose().translate( (float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * translationSensitivity );
                movingObjects = true;
            }
        }

        // Translate the selected light.
        if ( windowFocused && selectedLight ) 
        {
            const float   translationSensitivity = 0.002f;//0.002f;
            const int2    mouseMove              = inputManager.getMouseMove();

            const float3 sensitivity(
                inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
                inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
                );

            if ( inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                selectedLight->setPosition( selectedLight->getPosition() + ((float)(mouseMove.x - mouseMove.y) * (float)frameTimeMs * sensitivity * translationSensitivity ) );
                movingObjects = true;
            }
        }

        // Update the camera.
        if ( windowFocused && !movingObjects && inputManager.isMouseButtonPressed( InputManager::MouseButtons::right ) ) { 
            const float cameraRotationSensitivity = 0.0001f;

            const float acceleration = 2.0f;

            if ( inputManager.isKeyPressed( InputManager::Keys::w ) ) camera.accelerateForward( (float)frameTimeMs * acceleration );
            else if ( inputManager.isKeyPressed( InputManager::Keys::s ) ) camera.accelerateReverse( (float)frameTimeMs * acceleration );
            if ( inputManager.isKeyPressed( InputManager::Keys::d ) ) camera.accelerateRight( (float)frameTimeMs * acceleration );
            else if ( inputManager.isKeyPressed( InputManager::Keys::a ) ) camera.accelerateLeft( (float)frameTimeMs * acceleration );
            if ( inputManager.isKeyPressed( InputManager::Keys::e ) ) camera.accelerateUp( (float)frameTimeMs * acceleration );
            else if ( inputManager.isKeyPressed( InputManager::Keys::q ) ) camera.accelerateDown( (float)frameTimeMs * acceleration );

			int2 mouseMove = inputManager.getMouseMove( );
			camera.rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTimeMs * cameraRotationSensitivity );
		}

        inputManager.lockCursor( lockCursor );
        inputManager.updateMouseState();

		camera.updateState( (float)frameTimeMs );

        { // Update animations.
            const std::unordered_set< std::shared_ptr<Actor> >& sceneActors = scene->getActors();

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

        renderer.prepare();

        if ( scene )
            std::tie( frameUchar, frameUchar4, frameFloat4, frameFloat2, frameFloat ) = renderer.renderScene( *scene, camera );

		{ // Render FPS.
			//std::stringstream ss;
			//ss << "FPS: " << (int)(1000.0 / frameTimeMs) << " / " << frameTimeMs << "ms";
			//deferredRenderer.render( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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

        if ( frameUchar ) {
            rendererCore.copyTexture( ucharDisplayFrame, frameUchar );
		    frameRenderer.renderTexture( *ucharDisplayFrame, 0.0f, 0.0f );
        } else if ( frameUchar4 ) {
            if ( debugRenderAlpha )
		        frameRenderer.renderTextureAlpha( *frameUchar4, 0.0f, 0.0f );
            else
                frameRenderer.renderTexture( *frameUchar4, 0.0f, 0.0f );
        } else if ( frameFloat4 )
            frameRenderer.renderTexture( *frameFloat4, 0.0f, 0.0f );
        else if ( frameFloat2 )
            frameRenderer.renderTexture( *frameFloat2, 0.0f, 0.0f );
        else if ( frameFloat )
            frameRenderer.renderTexture( *frameFloat, 0.0f, 0.0f );

		frameRenderer.displayFrame();

		Timer frameEndTime;
		frameTimeMs = Timer::lapse( frameEndTime, frameStartTime );
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
        case WM_MOVE:
			if ( windowsMessageReceiver ) {
				int posX = LOWORD( lParam );
				int posY = HIWORD( lParam );
				windowsMessageReceiver->onMove( posX, posY );
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
                windowsMessageReceiver->onMouseButtonPress( 0 );
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
                windowsMessageReceiver->onMouseButtonPress( 1 );
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
                windowsMessageReceiver->onMouseButtonPress( 2 );
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

void Application::onMove( int newPosX, int newPosY )
{
    windowPosition.x = newPosX;
    windowPosition.y = newPosY;
}

void Application::onFocusChange( bool windowFocused )
{
	this->windowFocused = windowFocused;
}

void Application::onKeyPress( int key )
{
    if ( key == InputManager::Keys::l ) 
    {
        if ( scene )
        {
            float3 lightPosition = camera.getPosition() + camera.getDirection();

            std::shared_ptr< Light > light = std::make_shared<PointLight>( lightPosition );
            light->setColor( float3( 1.0f, 1.0f, 1.0f ) );
            scene->addLight( light );

            selectedLight = light;
        }
    } 
    else if ( key == InputManager::Keys::ctrl || key == InputManager::Keys::s ) 
    {
        if ( scene && !scenePath.empty() && inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::s ) ) {
            saveScene( scenePath );
        }
    }

    if ( key == InputManager::Keys::delete_ ) 
    {
        if ( scene ) 
        {
            if ( selectedBlockActor ) {
                scene->removeActor( selectedBlockActor );
                selectedBlockActor.reset();
            } else if ( selectedSkeletonActor ) {
                scene->removeActor( selectedSkeletonActor );
                selectedSkeletonActor.reset();
            } else if ( selectedLight ) {
                scene->removeLight( selectedLight );
                selectedLight.reset();
            }
        }
    }

    // Clone the actor, but share the model with the original actor.
    if ( key == InputManager::Keys::c && inputManager.isKeyPressed( InputManager::Keys::shift ) ) 
    {
        if ( scene ) 
        {
            if ( selectedBlockActor ) {
                selectedBlockActor = std::make_shared< BlockActor >( *selectedBlockActor ); // Clone the actor.
                scene->addActor( selectedBlockActor );
            } else if ( selectedSkeletonActor ) {
                selectedSkeletonActor = std::make_shared< SkeletonActor >( *selectedSkeletonActor ); // Clone the actor.
                scene->addActor( selectedSkeletonActor );
            }
        }
    }

    // Clone the actor and clone the model.
    if ( key == InputManager::Keys::c && inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) 
    {
        if ( scene ) 
        {
            if ( selectedBlockActor ) {
                selectedBlockActor = std::make_shared< BlockActor >( *selectedBlockActor ); // Clone the actor.
                selectedBlockActor->setModel( std::make_shared< BlockModel >( *selectedBlockActor->getModel() ) ); // Clone it's model.
                scene->addActor( selectedBlockActor );
            } else if ( selectedSkeletonActor ) {
                selectedSkeletonActor = std::make_shared< SkeletonActor >( *selectedSkeletonActor ); // Clone the actor.
                selectedSkeletonActor->setModel( std::make_shared< SkeletonModel >( *selectedSkeletonActor->getModel() ) ); // Clone it's model.
                scene->addActor( selectedSkeletonActor );
            }
        }
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

    if ( key == InputManager::Keys::tilde )
        renderer.setActiveViewType( Renderer::View::Final );
    else if ( key == InputManager::Keys::one )
        renderer.setActiveViewType( Renderer::View::Shaded );
    else if ( key == InputManager::Keys::two )
        renderer.setActiveViewType( Renderer::View::Depth );
    else if ( key == InputManager::Keys::three )
        renderer.setActiveViewType( Renderer::View::Position );
    else if ( key == InputManager::Keys::four )
        renderer.setActiveViewType( Renderer::View::Emissive );
    else if ( key == InputManager::Keys::five )
        renderer.setActiveViewType( Renderer::View::Albedo );
    else if ( key == InputManager::Keys::six )
        renderer.setActiveViewType( Renderer::View::Normal );
    else if ( key == InputManager::Keys::seven )
        renderer.setActiveViewType( Renderer::View::Metalness );
    else if ( key == InputManager::Keys::eight )
        renderer.setActiveViewType( Renderer::View::Roughness );
    else if ( key == InputManager::Keys::nine )
        renderer.setActiveViewType( Renderer::View::IndexOfRefraction );
    else if ( key == InputManager::Keys::zero )
        renderer.setActiveViewType( Renderer::View::RayDirections );
    else if ( key == InputManager::Keys::back )
        renderer.setActiveViewType( Renderer::View::Test );

    if ( key == InputManager::Keys::plus && inputManager.isKeyPressed( InputManager::Keys::shift ) )
        renderer.setMaxLevelCount( std::min( 10, renderer.getMaxLevelCount() + 1 ) );
    else if ( key == InputManager::Keys::minus && inputManager.isKeyPressed( InputManager::Keys::shift ) )
        renderer.setMaxLevelCount( std::max( 0, renderer.getMaxLevelCount() - 1 ) );
    else if ( key == InputManager::Keys::plus && inputManager.isKeyPressed( InputManager::Keys::r ) )
        renderer.activateNextViewLevel( true );
    else if ( key == InputManager::Keys::plus && inputManager.isKeyPressed( InputManager::Keys::t ) )
        renderer.activateNextViewLevel( false );
    else if ( key == InputManager::Keys::minus )
        renderer.activatePrevViewLevel();

    if ( key == InputManager::Keys::enter ) 
        debugRenderAlpha = !debugRenderAlpha;
}

void Application::onMouseButtonPress( int button )
{
    if ( button == 0 ) // On left button press.
    {
        // Calculate mouse pos relative to app window top-left corner.
        int2 mousePos = inputManager.getMousePos();
        mousePos -= windowPosition; 

        // TODO: FOV shouldn't be hardcoded.
        const float fieldOfView = (float)MathUtil::pi / 4.0f;

        std::shared_ptr< Actor > pickedActor;
        std::shared_ptr< Light > pickedLight;
        std::tie( pickedActor, pickedLight ) = pickActorOrLight( *scene, camera, float2( (float)mousePos.x, (float)mousePos.y ), (float)screenWidth, (float)screenHeight, fieldOfView );

        if ( pickedActor && pickedActor->getType() == Actor::Type::BlockActor ) {
            selectedBlockActor    = std::static_pointer_cast< BlockActor >( pickedActor );
            selectedSkeletonActor = nullptr;
            selectedLight         = nullptr;
        } else if ( pickedActor && pickedActor->getType() == Actor::Type::SkeletonActor ) {
            selectedBlockActor    = nullptr;
            selectedSkeletonActor = std::static_pointer_cast< SkeletonActor >( pickedActor );
            selectedLight         = nullptr;
        } else if ( pickedLight ) {
            selectedBlockActor    = nullptr;
            selectedSkeletonActor = nullptr;
            selectedLight         = pickedLight;
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

	std::string extension = StringUtil::toLowercase( filePath.substr( dotIndex + 1 ) );

	std::array< const std::string, 3 > blockMeshExtensions     = { "obj", "dae", "fbx" };
	std::array< const std::string, 1 > skeletonkMeshExtensions = { "dae" };
	std::array< const std::string, 9 > textureExtensions       = { "bmp", "dds", "jpg", "jpeg", "png", "raw", "tga", "tiff", "tif" };
    std::array< const std::string, 1 > blockModelExtensions    = { "blockmodel" };
    std::array< const std::string, 1 > skeletonModelExtensions = { "skeletonmodel" };
    std::array< const std::string, 1 > animationExtensions     = { "xaf" };
    std::array< const std::string, 1 > sceneExtensions         = { "scene" };

	bool isBlockMesh     = false;
	bool isSkeletonMesh  = false;
	bool isTexture       = false;
    bool isBlockModel    = false;
    bool isSkeletonModel = false;
    bool isAnimation     = false;
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

    for ( const std::string& animationExtension : animationExtensions ) {
        if ( extension.compare( animationExtension ) == 0 )
            isAnimation = true;
    }

    for ( const std::string& sceneExtension : sceneExtensions ) {
        if ( extension.compare( sceneExtension ) == 0 )
            isScene = true;
    }

    const bool replaceAsset = inputManager.isKeyPressed( InputManager::Keys::ctrl );

    float43 pose = float43::IDENTITY;
    pose.setTranslation( camera.getPosition() + camera.getDirection() );

	if ( isBlockMesh ) {
		BlockMeshFileInfo::Format format = BlockMeshFileInfo::Format::OBJ;

		if (      extension.compare( "obj" ) == 0 ) format = BlockMeshFileInfo::Format::OBJ;
		else if ( extension.compare( "dae" ) == 0 ) format = BlockMeshFileInfo::Format::DAE;
        else if ( extension.compare( "fbx" ) == 0 ) format = BlockMeshFileInfo::Format::FBX;

        BlockMeshFileInfo fileInfo( filePath, format, 0, false, false, false );
        std::shared_ptr<BlockMesh> mesh = std::static_pointer_cast<BlockMesh>( assetManager.getOrLoad( fileInfo ) );

        if ( !mesh->getBvhTree() )
            mesh->buildBvhTree();

        if ( !mesh->isInGpuMemory( ) ) {
            mesh->loadCpuToGpu( *frameRenderer.getDevice().Get() );
            mesh->loadBvhTreeToGpu( *frameRenderer.getDevice().Get() );
        }

        if ( replaceAsset ) {
            // Replace a mesh of an existing model.
            if ( selectedBlockActor && selectedBlockActor->getModel() ) {
                selectedBlockActor->getModel( )->setMesh( mesh );
            }
        } else {
            // Add new actor to the scene.
            selectedBlockActor = std::make_shared<BlockActor>( std::make_shared<BlockModel>(), pose );
            selectedBlockActor->getModel( )->setMesh( mesh );
            scene->addActor( selectedBlockActor );
        }
	}

	if ( isSkeletonMesh ) {
		SkeletonMeshFileInfo::Format format = SkeletonMeshFileInfo::Format::DAE;

		if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

        SkeletonMeshFileInfo fileInfo( filePath, format, 0, false, false, false );
        std::shared_ptr<SkeletonMesh> mesh = std::static_pointer_cast<SkeletonMesh>(assetManager.getOrLoad( fileInfo ));  
        if ( !mesh->isInGpuMemory( ) )
            mesh->loadCpuToGpu( *frameRenderer.getDevice( ).Get() );

        // #TODO: add replacing mesh? How to deal with non-matching animation?
        // Add new actor to the scene.
        selectedSkeletonActor = std::make_shared<SkeletonActor>( std::make_shared<SkeletonModel>( ), pose );
        selectedSkeletonActor->getModel( )->setMesh( mesh );
        selectedSkeletonActor->resetSkeletonPose();
        scene->addActor( selectedSkeletonActor );
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
        std::shared_ptr< Asset > textureAsset = assetManager.getOrLoad( fileInfo );

		if ( filePath.find( "_A." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            if ( selectedBlockActor ) {  
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllAlbedoTextures();

                selectedBlockActor->getModel( )->addAlbedoTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllAlbedoTextures();

                selectedSkeletonActor->getModel( )->addAlbedoTexture( modelTexture );
            }
        } 
        else if ( filePath.find( "_AL." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            if ( selectedBlockActor ) {    
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllAlphaTextures();

                selectedBlockActor->getModel( )->addAlphaTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllAlphaTextures();

                selectedSkeletonActor->getModel( )->addAlphaTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_M." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            if ( selectedBlockActor ) {    
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllMetalnessTextures();

                selectedBlockActor->getModel( )->addMetalnessTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllMetalnessTextures();

                selectedSkeletonActor->getModel( )->addMetalnessTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_N." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            if ( selectedBlockActor ) {  
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllNormalTextures();

                selectedBlockActor->getModel( )->addNormalTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllNormalTextures();

                selectedSkeletonActor->getModel( )->addNormalTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_R." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            if ( selectedBlockActor ) {   
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllRoughnessTextures();

                selectedBlockActor->getModel( )->addRoughnessTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllRoughnessTextures();

                selectedSkeletonActor->getModel( )->addRoughnessTexture( modelTexture );
            }
		} 
        else if ( filePath.find( "_E." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >( textureAsset );
            ModelTexture2D< uchar4 > modelTexture( texture );

            if ( selectedBlockActor ) {    
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllEmissionTextures();

                selectedBlockActor->getModel( )->addEmissionTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllEmissionTextures();

                selectedSkeletonActor->getModel( )->addEmissionTexture( modelTexture );
            }
        } 
        else if ( filePath.find( "_I." ) != std::string::npos ) 
        {
            auto texture = std::dynamic_pointer_cast< TTexture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > >( textureAsset );
            ModelTexture2D< unsigned char > modelTexture( texture );

            if ( selectedBlockActor ) {   
                if ( replaceAsset )
                    selectedBlockActor->getModel( )->removeAllIndexOfRefractionTextures();

                selectedBlockActor->getModel( )->addIndexOfRefractionTexture( modelTexture );
            }

            if ( selectedSkeletonActor ) {
                if ( replaceAsset )
                    selectedSkeletonActor->getModel( )->removeAllIndexOfRefractionTextures();

                selectedSkeletonActor->getModel( )->addIndexOfRefractionTexture( modelTexture );
            }
		}
	}

    if ( isBlockModel ) {

        BlockModelFileInfo fileInfo( filePath, BlockModelFileInfo::Format::BLOCKMODEL, 0 );
        std::shared_ptr<BlockModel> model = std::static_pointer_cast<BlockModel>( assetManager.getOrLoad( fileInfo ) );

        if ( model->getMesh() && !model->getMesh()->getBvhTree() )
        {
            model->getMesh()->buildBvhTree();
            model->getMesh()->loadBvhTreeToGpu( *frameRenderer.getDevice( ).Get() );
        }

        if ( !model->isInGpuMemory() )
            model->loadCpuToGpu( *frameRenderer.getDevice( ).Get(), *frameRenderer.getDeviceContext( ).Get() );

        // Add new actor to the scene.
        selectedBlockActor = std::make_shared<BlockActor>( model, pose );
        scene->addActor( selectedBlockActor );
    }

    if ( isSkeletonModel ) {

        SkeletonModelFileInfo fileInfo( filePath, SkeletonModelFileInfo::Format::SKELETONMODEL, 0 );
        std::shared_ptr<SkeletonModel> model = std::static_pointer_cast<SkeletonModel>(assetManager.getOrLoad( fileInfo ));
        if ( !model->isInGpuMemory( ) )
            model->loadCpuToGpu( *frameRenderer.getDevice().Get(), *frameRenderer.getDeviceContext( ).Get() );

        // Add new actor to the scene.
        selectedSkeletonActor = std::make_shared<SkeletonActor>( model, pose );
        scene->addActor( selectedSkeletonActor );
    }

    if ( isAnimation && selectedSkeletonActor ) {
        if ( selectedSkeletonActor->getModel() && selectedSkeletonActor->getModel()->getMesh() )
        {
            SkeletonMeshFileInfo&     referenceMeshFileInfo = selectedSkeletonActor->getModel()->getMesh()->getFileInfo();
            SkeletonAnimationFileInfo fileInfo( filePath, SkeletonAnimationFileInfo::Format::XAF, referenceMeshFileInfo, false );

            std::shared_ptr< SkeletonAnimation > animation = std::static_pointer_cast< SkeletonAnimation >( assetManager.getOrLoad( fileInfo ) );

            if ( animation )
                selectedSkeletonActor->startAnimation( animation );
        }
    }


    if ( (isBlockMesh || isTexture) && selectedBlockActor && selectedBlockActor->getModel( ) && selectedBlockActor->getModel( )->isInCpuMemory( ) )
        selectedBlockActor->getModel()->saveToFile( "Assets/Models/new.blockmodel" );

    if ( (isSkeletonMesh || isTexture) && selectedSkeletonActor && selectedSkeletonActor->getModel( ) && selectedSkeletonActor->getModel( )->isInCpuMemory( ) )
        selectedSkeletonActor->getModel()->saveToFile( "Assets/Models/new.skeletonmodel" );

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

    // Swap actors' empty models with the loaded models. Create BVH trees. Load models to GPU.
    const std::unordered_set< std::shared_ptr< Actor > > sceneActors = scene->getActors();
    for ( const std::shared_ptr< Actor >& actor : sceneActors ) 
    {
        if ( actor->getType() == Actor::Type::BlockActor ) 
        {

            const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>(actor);
            if ( blockActor->getModel() ) 
            {
                const BlockModelFileInfo& fileInfo = blockActor->getModel()->getFileInfo();
                std::shared_ptr<BlockModel> blockModel = std::static_pointer_cast<BlockModel>(assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( blockModel ) 
                {
                    // Build BVH tree and load it to GPU.
                    if ( blockModel->getMesh() ) {
                        blockModel->getMesh()->buildBvhTree();
                        blockModel->getMesh()->loadBvhTreeToGpu( *frameRenderer.getDevice().Get() );
                    }

                    blockModel->loadCpuToGpu( *frameRenderer.getDevice().Get(), *frameRenderer.getDeviceContext( ).Get() );
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
                std::shared_ptr<SkeletonModel> skeletonModel = std::static_pointer_cast<SkeletonModel>(assetManager.get( fileInfo.getAssetType(), fileInfo.getPath(), fileInfo.getIndexInFile() ));
                if ( skeletonModel ) 
                {
                    skeletonModel->loadCpuToGpu( *frameRenderer.getDevice().Get(), *frameRenderer.getDeviceContext( ).Get() );
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
    scene->saveToFile( path );
}
