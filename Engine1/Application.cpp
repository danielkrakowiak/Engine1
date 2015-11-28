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
	windowFocused( false )
{
	windowsMessageReceiver = this;

	createdBlockModel    = std::make_shared<BlockModel>();
	createdSkeletonModel = std::make_shared<SkeletonModel>();
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

	//std::shared_ptr<BlockMesh> meshDae = std::make_shared<BlockMesh>( "../Engine1/Assets/Meshes/character.dae", AssetFileFormat::DAE, true, true, true );
	//meshDae->loadFile( );
	//meshDae->load( );
	//meshDae->loadToGpu( direct3DRenderer.getDevice( ) );

	std::shared_ptr<BlockMesh> axisMesh = BlockMesh::createFromFile( "../Engine1/Assets/Meshes/dx-coordinate-axises.obj", BlockMeshFileInfo::Format::OBJ, true, true, true ).front();
	axisMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	/*std::shared_ptr<BlockMesh> mesh2 = std::make_shared<BlockMesh>( "../Engine1/Assets/Meshes/spaceship.obj", AssetFileFormat::OBJ, true, true, true );
	mesh2->loadFile();
	mesh2->load( );
	mesh2->loadToGpu( direct3DRenderer.getDevice( ) );*/

	//////
	//std::shared_ptr<BlockMesh> mesh3 = BlockMesh::createFromFile( "../Engine1/Assets/TestAssets/Meshes/quadbot2.obj", BlockMeshFileInfo::Format::OBJ, false, false ).front( );
	//mesh3->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	//std::shared_ptr<Texture2D> albedoTexture = Texture2D::createFromFile( "../Engine1/Assets/TestAssets/Textures/Quadbot/quadbot_dirt.png", Texture2DFileInfo::Format::BMP );
	//albedoTexture->loadCpuToGpu( direct3DFrameRenderer.getDevice(  ) );

	//std::shared_ptr<BlockModel> model1 = std::make_shared<BlockModel>( );
	//model1->setMesh( mesh3 );
	//model1->addAlbedoTexture( ModelTexture2D( albedoTexture, 0 ) );

	//model1->saveToFile( "../Engine1/Assets/TestAssets/Models/quadbot.blockmodel" );
	/////

	//std::shared_ptr<BlockModel> model1 = BlockModel::createFromFile( "../Engine1/Assets/Models/quadbot.blockmodel", BlockModel::FileFormat::BLOCKMODEL, true );
	//model1->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

	/////
	/*std::shared_ptr<BlockMesh> mesh4 = std::make_shared<BlockMesh>( "../Engine1/Assets/Meshes/tree/tree-trunk.obj", AssetFileFormat::OBJ, true, true, true );
	mesh4->loadFile( );
	mesh4->load( );
	mesh4->loadToGpu( direct3DRenderer.getDevice( ) );*/

	/*std::shared_ptr<BlockModel> model2 = std::make_shared<BlockModel>( );
	model2->setMesh( mesh4 );
	model2->addAlbedoTexture( albedoTexture2, 0 );*/
	/////

	/////
	/*std::shared_ptr<BlockMesh> mesh5 = std::make_shared<BlockMesh>( "../Engine1/Assets/Meshes/tree/tree-leaves-4.obj", AssetFileFormat::OBJ, true, true );
	mesh5->loadFile( );
	mesh5->load( );
	mesh5->loadToGpu( direct3DRenderer.getDevice( ) );

	std::shared_ptr<Texture2D> albedoTexture3 = std::make_shared<Texture2D>( "../Engine1/Assets/Textures/tree/leaf.jpg", AssetFileFormat::BMP );
	albedoTexture3->loadFile( );
	albedoTexture3->load( );
	albedoTexture3->loadToGpu( direct3DRenderer.getDevice( ), direct3DRenderer.getDeviceContext( ) );

	std::shared_ptr<BlockModel> model3 = std::make_shared<BlockModel>( );
	model3->setMesh( mesh5 );
	model3->addAlbedoTexture( albedoTexture3, 0 );*/
	/////

	std::shared_ptr<BlockMesh> pilotBlockMesh = BlockMesh::createFromFile( "../Engine1/Assets/Meshes/Pilot/Pilot.dae", BlockMeshFileInfo::Format::DAE, false, false, false ).at( 1 );
	pilotBlockMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<BlockMesh> ellisBlockMesh = BlockMesh::createFromFile( "../Engine1/Assets/Meshes/Ellis/Ellis.dae", BlockMeshFileInfo::Format::DAE, false, false, false ).at( 1 );
	ellisBlockMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<SkeletonMesh> pilotSkeletonMesh = SkeletonMesh::createFromFile( "../Engine1/Assets/Meshes/Pilot/Pilot.dae", SkeletonMeshFileInfo::Format::DAE, false, false, false ).at( 1 );
	pilotSkeletonMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<SkeletonMesh> skeletonMesh2 = SkeletonMesh::createFromFile( "../Engine1/Assets/Meshes/character2/character2.dae", SkeletonMeshFileInfo::Format::DAE, false, false, false ).front( );
	skeletonMesh2->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<SkeletonAnimation> idleAnimationInSkeletonSpace     = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/character2/idle_pose.xaf", SkeletonAnimationFileInfo::Format::XAF, *skeletonMesh2, false );
	std::shared_ptr<SkeletonAnimation> crouchAnimationInSkeletonSpace   = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/character2/crouch_pose_selected_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *skeletonMesh2, false );
	std::shared_ptr<SkeletonAnimation> bendHandAnimationInSkeletonSpace = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/character2/bend_hand_pose_selected_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *skeletonMesh2, false );
	std::shared_ptr<SkeletonAnimation> waveAnimationInSkeletonSpace     = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/character2/wave_anim_selected_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *skeletonMesh2, false );
	std::shared_ptr<SkeletonAnimation> waveAnimationInSkeletonSpace2    = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/character2/wave_pose_selected_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *skeletonMesh2, false );


	std::shared_ptr<SkeletonAnimation> idleAnimationInParentSpace       = SkeletonAnimation::calculateAnimationInParentSpace( *idleAnimationInSkeletonSpace, *skeletonMesh2 );
	std::shared_ptr<SkeletonAnimation> crouchAnimationInParentSpace     = SkeletonAnimation::calculateAnimationInParentSpace( *crouchAnimationInSkeletonSpace, *skeletonMesh2 );
	std::shared_ptr<SkeletonAnimation> bendHandAnimationInParentSpace   = SkeletonAnimation::calculateAnimationInParentSpace( *bendHandAnimationInSkeletonSpace, *skeletonMesh2 );
	std::shared_ptr<SkeletonAnimation> waveAnimationInParentSpace       = SkeletonAnimation::calculateAnimationInParentSpace( *waveAnimationInSkeletonSpace, *skeletonMesh2 );
	std::shared_ptr<SkeletonAnimation> waveAnimationInParentSpace2      = SkeletonAnimation::calculateAnimationInParentSpace( *waveAnimationInSkeletonSpace2, *skeletonMesh2 );

	SkeletonPose& skeletonPoseIdle     = idleAnimationInParentSpace->getPose( 0u );
	SkeletonPose& skeletonPoseCrouch   = crouchAnimationInParentSpace->getPose( 0u );
	SkeletonPose& skeletonPoseBendHand = bendHandAnimationInParentSpace->getPose( 0u );
	SkeletonPose& skeletonPoseWave2    = bendHandAnimationInParentSpace->getPose( 0u );


	


	std::shared_ptr<SkeletonMesh> girlMesh = SkeletonMesh::createFromFile( "../Engine1/Assets/TestAssets/Meshes/bikini_girl.DAE", SkeletonMeshFileInfo::Format::DAE, 0, false, false, false );
	girlMesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<Texture2D> girlAlbedoTexture = Texture2D::createFromFile( "../Engine1/Assets/TestAssets/Textures/Bikini Girl/BikiniGirl_Body_D.tga", Texture2DFileInfo::Format::TGA );
	girlAlbedoTexture->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

	std::shared_ptr<SkeletonModel> girlModel = std::make_shared<SkeletonModel>( );
	girlModel->setMesh( girlMesh );
	girlModel->addAlbedoTexture( ModelTexture2D( girlAlbedoTexture, 0 ) );

	girlModel->saveToFile( "../Engine1/Assets/TestAssets/Models/bikini_girl.skeletonmodel" );



	std::vector< std::shared_ptr<BlockMesh> > girlBlockMeshes = BlockMesh::createFromFile( "../Engine1/Assets/Meshes/Bikini_Girl2/Bikini Girl.dae", BlockMeshFileInfo::Format::DAE, false, false, false );
	for ( std::shared_ptr<BlockMesh>& mesh : girlBlockMeshes )
		mesh->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

	std::shared_ptr<BlockMesh> girlBlockMesh = girlBlockMeshes.at( 0 );

	std::shared_ptr<BlockModel> girlBlockModel = std::make_shared<BlockModel>();
	girlBlockModel->setMesh( girlBlockMesh );
	girlBlockModel->addAlbedoTexture( ModelTexture2D( girlAlbedoTexture, 0 ) );






	std::shared_ptr<SkeletonAnimation> idleAnimationInSkeletonSpace3 = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/Bikini_Girl2/kick_all_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *girlMesh, false );
	std::shared_ptr<SkeletonAnimation> idleAnimationInParentSpace3 = SkeletonAnimation::calculateAnimationInParentSpace( *idleAnimationInSkeletonSpace3, *girlMesh );

	std::shared_ptr<SkeletonAnimation> pilotRunAnimationInSkeletonSpace = SkeletonAnimation::createFromFile( "../Engine1/Assets/Meshes/Pilot/run_anim_all_bones.xaf", SkeletonAnimationFileInfo::Format::XAF, *pilotSkeletonMesh, false );
	SkeletonPose& pilotPose = pilotRunAnimationInSkeletonSpace->getPose( 0u );

	float3 up, position, lookAt;
	float43 worldMatrix, worldMatrix2, worldMatrix3;
	float44 viewMatrix;

	worldMatrix.identity();
	worldMatrix2.identity();
	worldMatrix3.identity();
	worldMatrix2.setTranslation( float3( 0.0f, 0.0f, -25.0f ) );
	//worldMatrix2.setOrientation( MathUtil::anglesToRotationMatrix( float3( MathUtil::pi, 0.0f, 0.0f ) ) );
	worldMatrix3.setTranslation( float3( 0.0f, 10.0f, 0.0f ) );

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

		{ //update camera
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

		viewMatrix = MathUtil::lookAtTransformation( camera.getLookAtPoint( ), camera.getPosition( ), camera.getUp( ) );


		direct3DDefferedRenderer.clearRenderTargets( float4( 0.2f, 0.2f, 0.2f, 1.0f ), 1.0f );

		direct3DDefferedRenderer.render( *axisMesh, worldMatrix, viewMatrix );

		//direct3DDefferedRenderer.render( *model1, worldMatrix, viewMatrix );

		{ // Ellis rendering.
			direct3DDefferedRenderer.render( *ellisBlockMesh, worldMatrix3, viewMatrix );

			//SkeletonPose poseInSkeletonSpace = pilotPose;// SkeletonPose::calculatePoseInSkeletonSpace( skeletonPoseIdle3, *girlMesh );
			//direct3DDefferedRenderer.render( *pilotSkeletonMesh, worldMatrix2, viewMatrix, poseInSkeletonSpace );
		}

		{ // Pilot rendering.
			//direct3DDefferedRenderer.render( *pilotBlockMesh, worldMatrix, viewMatrix );

			SkeletonPose poseInSkeletonSpace = pilotPose;// SkeletonPose::calculatePoseInSkeletonSpace( skeletonPoseIdle3, *girlMesh );
			direct3DDefferedRenderer.render( *pilotSkeletonMesh, worldMatrix2, viewMatrix, poseInSkeletonSpace );
		}

		
		{ // Render the newly created models.
			if ( createdBlockModel->isInGpuMemory() ) 
					direct3DDefferedRenderer.render( *createdBlockModel, worldMatrix, viewMatrix );
			else if ( createdBlockModel->getMesh( ) && createdBlockModel->getMesh( )->isInGpuMemory() )
					direct3DDefferedRenderer.render( *createdBlockModel->getMesh(), worldMatrix, viewMatrix );

			if ( createdSkeletonModel->getMesh( ) && createdSkeletonModel->getMesh( )->isInGpuMemory() ) {
				// Create 'identity' pose for the mesh.
				SkeletonPose poseInParentSpace;
				for ( unsigned char boneIndex = 0; boneIndex < createdSkeletonModel->getMesh()->getBoneCount(); ++boneIndex )
					poseInParentSpace.setBonePose( boneIndex, float43::IDENTITY );

				SkeletonPose poseInSkeletonSpace = SkeletonPose::calculatePoseInSkeletonSpace( poseInParentSpace, *createdSkeletonModel->getMesh( ) );

				if ( createdSkeletonModel->isInGpuMemory() )
					direct3DDefferedRenderer.render( *createdSkeletonModel, worldMatrix, viewMatrix, poseInSkeletonSpace );
				else
					direct3DDefferedRenderer.render( *createdSkeletonModel->getMesh(), worldMatrix, viewMatrix, poseInSkeletonSpace );
			}
		}

		//direct3DDefferedRenderer.render( *mesh3, worldMatrix, viewMatrix );

		{ // Skeleton mesh rendering.
			static float factor = 0.0f;
			static float factorDelta = 0.001f;
			factor += factorDelta;
			if ( factor < 0.0f ) {
				factor = 0.0f;
				factorDelta = -factorDelta;
			} else if ( factor > 1.0f ) {
				factor = 1.0f;
				factorDelta = -factorDelta;
			}

			static float factor2 = 0.0f;
			static float factorDelta2 = 0.0005f;
			factor2 += factorDelta2;
			if ( factor2 < 0.0f ) {
				factor2 = 0.0f;
				factorDelta2 = -factorDelta2;
			} else if ( factor2 > 1.0f ) {
				factor2 = 1.0f;
				factorDelta2 = -factorDelta2;
			}

			SkeletonPose skeletonPoseWave = waveAnimationInParentSpace->getInterpolatedPose( factor2 );

			SkeletonPose skeletonPose1 = SkeletonPose::blendPoses( skeletonPoseIdle, skeletonPoseCrouch, factor );
			SkeletonPose skeletonPose2 = SkeletonPose::blendPoses( skeletonPose1, skeletonPoseWave, 1.0f );

			SkeletonPose poseInSkeletonSpace = SkeletonPose::calculatePoseInSkeletonSpace( skeletonPose2, *skeletonMesh2 );
			direct3DDefferedRenderer.render( *skeletonMesh2, worldMatrix2, viewMatrix, poseInSkeletonSpace );
		}

		{ // Bikini girl rendering.
			static float factor3 = 0.0f;
			static float factorDelta3 = 0.001f;
			factor3 += factorDelta3;
			if ( factor3 < 0.0f ) {
				factor3 = 0.0f;
				factorDelta3 = -factorDelta3;
			} else if ( factor3 > 1.0f ) {
				factor3 = 1.0f;
				factorDelta3 = -factorDelta3;
			}

			//SkeletonPose poseInSkeletonSpace = idleAnimationInSkeletonSpace3->getPose( 3 );
			
			SkeletonPose poseInSkeletonSpace = idleAnimationInSkeletonSpace3->getInterpolatedPose( factor3 );//skeletonPoseIdle3;// SkeletonPose::calculatePoseInSkeletonSpace( skeletonPoseIdle3, *girlMesh );
			
			//SkeletonPose poseInParentSpace = idleAnimationInParentSpace3->getInterpolatedPose( factor3 );
			//SkeletonPose poseInSkeletonSpace = SkeletonPose::calculatePoseInSkeletonSpace( poseInParentSpace, *girlMesh );

			direct3DDefferedRenderer.render( *girlModel, worldMatrix2, viewMatrix, poseInSkeletonSpace );

			direct3DDefferedRenderer.render( *girlBlockModel, worldMatrix2, viewMatrix );
		}

		{ //render FPS
			std::stringstream ss;
			ss << "FPS: " << (int)(1000.0 / frameTime) << " / " << frameTime << "ms";
			direct3DDefferedRenderer.render( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		{ //render camera state
			std::stringstream ss;
			ss << "Speed: " << camera.getSpeed( ).x << " / " << camera.getSpeed( ).y << " / " << camera.getSpeed( ).z;
			//direct3DRenderer.renderText( ss.str( ), font, float2( -500.0f, 300.0f ), float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		{ //render keyboard state
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
	std::array< const std::string, 3 > textureExtensions       = { "tga", "png", "bmp" };

	bool isBlockMesh    = false;
	bool isSkeletonMesh = false;
	bool isTexture      = false;

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

	if ( isBlockMesh ) {
		BlockMeshFileInfo::Format format;

		if (      extension.compare( "obj" ) == 0 ) format = BlockMeshFileInfo::Format::OBJ;
		else if ( extension.compare( "dae" ) == 0 ) format = BlockMeshFileInfo::Format::DAE;

		std::shared_ptr<BlockMesh> mesh = BlockMesh::createFromFile( filePath, format, 0, false, false, false );
		mesh->loadCpuToGpu( direct3DFrameRenderer.getDevice() );
		createdBlockModel->setMesh( mesh );
		createdBlockModel->loadCpuToGpu( direct3DFrameRenderer.getDevice() );
	}

	if ( isSkeletonMesh ) {
		SkeletonMeshFileInfo::Format format;

		if ( extension.compare( "dae" ) == 0 ) format = SkeletonMeshFileInfo::Format::DAE;

		std::shared_ptr<SkeletonMesh> mesh = SkeletonMesh::createFromFile( filePath, format, 0, false, false, false );
		mesh->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );

		createdSkeletonModel->setMesh( mesh );
		createdSkeletonModel->loadCpuToGpu( direct3DFrameRenderer.getDevice( ) );
	}

	if ( isTexture ) {
		Texture2DFileInfo::Format format;

		if ( extension.compare( "bmp" ) == 0 )       format = Texture2DFileInfo::Format::BMP;
		else if ( extension.compare( "dds" ) == 0 )  format = Texture2DFileInfo::Format::DDS;
		else if ( extension.compare( "jpeg" ) == 0 ) format = Texture2DFileInfo::Format::JPEG;
		else if ( extension.compare( "png" ) == 0 )  format = Texture2DFileInfo::Format::PNG;
		else if ( extension.compare( "raw" ) == 0 )  format = Texture2DFileInfo::Format::RAW;
		else if ( extension.compare( "tga" ) == 0 )  format = Texture2DFileInfo::Format::TGA;
		else if ( extension.compare( "tiff" ) == 0 ) format = Texture2DFileInfo::Format::TIFF;

		std::shared_ptr<Texture2D> texture = Texture2D::createFromFile( filePath, format );
		texture->loadCpuToGpu( direct3DFrameRenderer.getDevice() );

		if ( filePath.find( "_A" ) ) {
			createdBlockModel->addAlbedoTexture( ModelTexture2D( texture ) );
			createdSkeletonModel->addAlbedoTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_N" ) ) {
			createdBlockModel->addNormalTexture( ModelTexture2D( texture ) );
			createdSkeletonModel->addNormalTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_R" ) ) {
			createdBlockModel->addRoughnessTexture( ModelTexture2D( texture ) );
			createdSkeletonModel->addRoughnessTexture( ModelTexture2D( texture ) );
		} else if ( filePath.find( "_E" ) ) {
			createdBlockModel->addEmissionTexture( ModelTexture2D( texture ) );
			createdSkeletonModel->addEmissionTexture( ModelTexture2D( texture ) );
		}
	}


	if ( ( isBlockMesh || isTexture ) && createdBlockModel->isInCpuMemory( ) )
		createdBlockModel->saveToFile( "Assets/Models/new.blockmodel" );

	if ( ( isSkeletonMesh || isTexture ) && createdSkeletonModel->isInCpuMemory( ) )
		createdSkeletonModel->saveToFile( "Assets/Models/new.skeletonmodel" );
}