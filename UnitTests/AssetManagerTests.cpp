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

	TEST_METHOD( AssetManager_Construct_Destruct_1 )
	{
		int cpuThreadCount = (int)std::thread::hardware_concurrency();
		if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

		try {
			{
				AssetManager assetManager( cpuThreadCount );
			}
		} catch ( ... ) {
			Assert::Fail( L"AssetManager::AssetManager or AssetManager::~AssetManager threw an exception" );
		}

	}

		TEST_METHOD( AssetManager_Construct_Destruct2 ) {
			try {
				{
					AssetManager assetManager( -10 );
					Assert::Fail( L"AssetManager::AssetManager didn't throw the exception for negative number of worker threads" );
				}
			} catch ( ... ) {
				//correct exception
			}

			try {
				{
					AssetManager assetManager( 0 );
					Assert::Fail( L"AssetManager::AssetManager didn't throw the exception for zero worker threads" );
				}
			} catch ( ... ) {
				//correct exception
			}

		}

		TEST_METHOD(AssetManager_Initial_State_1)
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency( );
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			Assert::IsFalse( assetManager.isAvailable( "xxx" ), L"AssetManager::isAvailable returned true for non-exisiting mesh" );

			try {
				assetManager.get( "xxx" );
				Assert::Fail( L"AssetManager::get didn't throw the exception for non-exisiting mesh" );
			} catch ( ... ) {
				//correct exception
			}
		}

		TEST_METHOD( AssetManager_Mesh_Loading_1 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.load( fileInfo );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::load threw an exception" );
			}

			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAvailable returned false" );

			try {
				std::shared_ptr<Asset> asset = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Mesh_Loading_2 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.load( fileInfo1 );
				assetManager.load( fileInfo2 );
				assetManager.load( fileInfo3 );
				assetManager.load( fileInfo4 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::load threw an exception" );
			}

			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isAvailable returned false" );

			try {
				std::shared_ptr<Asset> asset1 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset2 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset3 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset4 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Mesh_Loading_3 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.load( fileInfo1 );
				assetManager.load( fileInfo2 );
				assetManager.load( fileInfo3 );
				assetManager.load( fileInfo4 );
				assetManager.load( fileInfo5 );

				Assert::Fail( L"AssetManager::load didn't throw the exception" );
			} catch ( ... ) {

			}
		}

		TEST_METHOD( AssetManager_Mesh_Loading_4 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.load( fileInfo1 );
				assetManager.load( fileInfo2 );
				assetManager.load( fileInfo3 );
				assetManager.load( fileInfo4 );
				assetManager.load( fileInfo5 );

				Assert::Fail( L"AssetManager::load didn't throw the exception" );
			} catch ( ... ) {

			}
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_1 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 2000;
			#else
			int meshMaxLoadingTimeMilisec = 500;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );

			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAvailable returned false" );

			try {
				std::shared_ptr<Asset> asset = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_2 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 60000;
			#else
			int meshMaxLoadingTimeMilisec = 5000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );

			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isAvailable returned false" );

			try {
				std::shared_ptr<Asset> asset1 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset2 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset3 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset4 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_3 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 100000;
			#else
			int meshMaxLoadingTimeMilisec = 10000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );


			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo6( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo7( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo8( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );
				assetManager.loadAsync( fileInfo6 );
				assetManager.loadAsync( fileInfo7 );
				assetManager.loadAsync( fileInfo8 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );

			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj" ), L"AssetManager::isAvailable returned false" );
			Assert::IsTrue( assetManager.isAvailable( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj" ), L"AssetManager::isAvailable returned false" );

			try {
				std::shared_ptr<Asset> asset1 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset2 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset3 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset4 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset5 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset5 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset6 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset6 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset7 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset7 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
				std::shared_ptr<Asset> asset8 = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj" );
				Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset8 ).get(), L"AssetManager::get returned an asset which is not a BlockMesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_4 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 10000;
			#else
			int meshMaxLoadingTimeMilisec = 5000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo6( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo7( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo8( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );
				assetManager.loadAsync( fileInfo6 );
				assetManager.loadAsync( fileInfo7 );
				assetManager.loadAsync( fileInfo8 );

				Assert::Fail( L"AssetManager::loadAssetAsync didn't throw the exception" );
			} catch ( ... ) {

			}

			//std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_5 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 10000;
			#else
			int meshMaxLoadingTimeMilisec = 5000;
			#endif
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );

				Assert::Fail( L"AssetManager::loadAsync didn't throw the exception" );
			} catch ( ... ) {

			}

			//std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
		}

		TEST_METHOD( AssetManager_Mesh_Async_Loading_6 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 10000;
			#else
			int meshMaxLoadingTimeMilisec = 5000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			BlockMeshFileInfo fileInfo1( "../Engine1/Assets/TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo2( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( "../Engine1/Assets/TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj", BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );

				Assert::Fail( L"AssetManager::loadAssetAsync didn't throw the exception" );
			} catch ( ... ) {

			}

			//std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );
		}
	};
}