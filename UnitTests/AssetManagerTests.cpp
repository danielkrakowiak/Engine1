#include "stdafx.h"
#include "CppUnitTest.h"

#include "AssetManager.h"

#include "BlockMesh.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(AssetManagerTests)
	{
	public:
//
//		TEST_METHOD( AssetManager_Construct_Destruct_1 ) {
//			int cpuThreadCount = (int)std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			try {
//				{
//					AssetManager assetManager( cpuThreadCount );
//				}
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::AssetManager or AssetManager::~AssetManager threw an exception" );
//			}
//
//		}
//
//		TEST_METHOD( AssetManager_Construct_Destruct2 ) {
//			try {
//				{
//					AssetManager assetManager( -10 );
//					Assert::Fail( L"AssetManager::AssetManager didn't throw the exception for negative number of worker threads" );
//				}
//			} catch ( ... ) {
//				//correct exception
//			}
//
//			try {
//				{
//					AssetManager assetManager( 0 );
//					Assert::Fail( L"AssetManager::AssetManager didn't throw the exception for zero worker threads" );
//				}
//			} catch ( ... ) {
//				//correct exception
//			}
//
//		}
//
//		TEST_METHOD(AssetManager_Initial_State_1)
//		{
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			Assert::IsFalse( assetManager.isAssetLoaded( "xxx" ), L"AssetManager::isAssetLoaded returned true for non-exisiting mesh" );
//
//			try {
//				assetManager.getLoadedAsset( "xxx" );
//				Assert::Fail( L"AssetManager::getLoadedAsset didn't throw the exception for non-exisiting mesh" );
//			} catch ( ... ) {
//				//correct exception
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Loading_1 ) {
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BlockMesh> mesh = std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ );
//			std::shared_ptr<BasicAsset> asset = std::static_pointer_cast<BasicAsset>(mesh);
//
//			try {
//				assetManager.loadAsset( asset );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::loadBlockMesh threw an exception" );
//			}
//
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			
//			try {
//				std::shared_ptr<BasicAsset> asset = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset ).get(), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::getLoadedAsset threw an exception" );
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Loading_2 ) {
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAsset( mesh1 );
//				assetManager.loadAsset( mesh2 );
//				assetManager.loadAsset( mesh3 );
//				assetManager.loadAsset( mesh4 );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::loadAsset threw an exception" );
//			}
//
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isAssetLoaded returned false" );
//
//			try {
//				std::shared_ptr<BasicAsset> asset1 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get(), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset2 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get(), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset3 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get(), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset4 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get(), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::getLoadedAsset threw an exception" );
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Loading_3 ) {
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAsset( mesh1 );
//				assetManager.loadAsset( mesh2 );
//				assetManager.loadAsset( mesh3 );
//				assetManager.loadAsset( mesh4 );
//				assetManager.loadAsset( mesh5 );
//
//				Assert::Fail( L"AssetManager::loadAsset didn't throw the exception" );
//			} catch ( ... ) {
//				
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Loading_4 ) {
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/pyramid.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAsset( mesh1 );
//				assetManager.loadAsset( mesh2 );
//				assetManager.loadAsset( mesh3 );
//				assetManager.loadAsset( mesh4 );
//				assetManager.loadAsset( mesh5 );
//
//				Assert::Fail( L"AssetManager::loadAsset didn't throw the exception" );
//			} catch ( ... ) {
//
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_1 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 2000;
//#else
//			int meshMaxLoadingTimeMilisec = 500;
//#endif
//
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::loadAssetAsync threw an exception" );
//			}
//
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAssetLoaded returned false" );
//
//			try {
//				std::shared_ptr<BasicAsset> asset = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::getLoadedAsset threw an exception" );
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_2 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 60000;
//#else
//			int meshMaxLoadingTimeMilisec = 5000;
//#endif
//
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//				assetManager.loadAssetAsync( mesh2 );
//				assetManager.loadAssetAsync( mesh3 );
//				assetManager.loadAssetAsync( mesh4 );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::loadAssetAsync threw an exception" );
//			}
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isAssetLoaded returned false" );
//
//			try {
//				std::shared_ptr<BasicAsset> asset1 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset2 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset3 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset4 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::getLoadedAsset threw an exception" );
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_3 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 100000;
//#else
//			int meshMaxLoadingTimeMilisec = 10000;
//#endif
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh6 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh7 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh8 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//				assetManager.loadAssetAsync( mesh2 );
//				assetManager.loadAssetAsync( mesh3 );
//				assetManager.loadAssetAsync( mesh4 );
//				assetManager.loadAssetAsync( mesh5 );
//				assetManager.loadAssetAsync( mesh6 );
//				assetManager.loadAssetAsync( mesh7 );
//				assetManager.loadAssetAsync( mesh8 );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::loadAssetAsync threw an exception" );
//			}
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj" ), L"AssetManager::isAssetLoaded returned false" );
//			Assert::IsTrue( assetManager.isAssetLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj" ), L"AssetManager::isAssetLoaded returned false" );
//
//			try {
//				std::shared_ptr<BasicAsset> asset1 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset2 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset3 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset4 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset5 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset5 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset6 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset6 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset7 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset7 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//				std::shared_ptr<BasicAsset> asset8 = assetManager.getLoadedAsset( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj" );
//				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset8 ).get( ), L"AssetManager::getLoadedAsset returned BasicAsset which is not a BlockMesh" );
//			} catch ( ... ) {
//				Assert::Fail( L"AssetManager::getLoadedAsset threw an exception" );
//			}
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_4 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 10000;
//#else
//			int meshMaxLoadingTimeMilisec = 5000;
//#endif
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh6 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh7 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh8 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//				assetManager.loadAssetAsync( mesh2 );
//				assetManager.loadAssetAsync( mesh3 );
//				assetManager.loadAssetAsync( mesh4 );
//				assetManager.loadAssetAsync( mesh5 );
//				assetManager.loadAssetAsync( mesh6 );
//				assetManager.loadAssetAsync( mesh7 );
//				assetManager.loadAssetAsync( mesh8 );
//				Assert::Fail( L"AssetManager::loadAssetAsync didn't throw the exception" );
//			} catch ( ... ) {
//				
//			}
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_5 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 10000;
//#else
//			int meshMaxLoadingTimeMilisec = 5000;
//#endif
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//				assetManager.loadAssetAsync( mesh2 );
//				assetManager.loadAssetAsync( mesh3 );
//				assetManager.loadAssetAsync( mesh4 );
//				assetManager.loadAssetAsync( mesh5 );
//
//				Assert::Fail( L"AssetManager::loadAssetAsync didn't throw the exception" );
//			} catch ( ... ) {
//
//			}
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//		}
//
//		TEST_METHOD( AssetManager_Mesh_Async_Loading_6 ) {
//#if defined _DEBUG
//			int meshMaxLoadingTimeMilisec = 10000;
//#else
//			int meshMaxLoadingTimeMilisec = 5000;
//#endif
//			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
//			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;
//
//			AssetManager assetManager( cpuThreadCount );
//
//			std::shared_ptr<BasicAsset> mesh1 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh2 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh3 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh4 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", AssetFileFormat::OBJ ) );
//			std::shared_ptr<BasicAsset> mesh5 = std::static_pointer_cast<BasicAsset>( std::make_shared<BlockMesh>( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", AssetFileFormat::OBJ ) );
//
//			try {
//				assetManager.loadAssetAsync( mesh1 );
//				assetManager.loadAssetAsync( mesh2 );
//				assetManager.loadAssetAsync( mesh3 );
//				assetManager.loadAssetAsync( mesh4 );
//				assetManager.loadAssetAsync( mesh5 );
//
//				Assert::Fail( L"AssetManager::loadAssetAsync didn't throw the exception" );
//			} catch ( ... ) {
//
//			}
//
//			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
//
//		}
//
	};
}