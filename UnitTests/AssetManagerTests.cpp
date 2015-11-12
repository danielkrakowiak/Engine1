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

			Assert::IsFalse( assetManager.isLoaded( "xxx" ), L"AssetManager::isLoaded returned true for non-exisiting mesh" );

			try {
				std::shared_ptr<Asset> asset = assetManager.get( "xxx" );
				Assert::IsNull( asset.get(), L"AssetManager::get didn't return nullptr for a non-exisiting mesh" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::get threw an exception" );
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

			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isLoaded returned false" );

			try {
				std::shared_ptr<Asset> asset = assetManager.get( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" );
				Assert::IsNotNull( asset.get( ), L"AssetManager::get returned nullptr" );
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

			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isLoaded returned false" );

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

		TEST_METHOD( AssetManager_Mesh_Loading_5 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 4;

			AssetManager assetManager( cpuThreadCount );

			const std::string path = "../Engine1/Assets/TestAssets/Meshes/Bikini_Girl.dae";

			BlockMeshFileInfo fileInfo1( path, BlockMeshFileInfo::Format::DAE, 0 );
			BlockMeshFileInfo fileInfo2( path, BlockMeshFileInfo::Format::DAE, 1 );
			BlockMeshFileInfo fileInfo3( path, BlockMeshFileInfo::Format::DAE, 2 );

			try {
				assetManager.load( fileInfo1 );

				Assert::IsTrue(  assetManager.isLoaded( path, 0 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsFalse( assetManager.isLoaded( path, 1 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsFalse( assetManager.isLoaded( path, 1 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsNotNull( assetManager.get( path, 0 ).get(), L"AssetManager::get returned nullptr" );
				Assert::IsNull(    assetManager.get( path, 1 ).get( ), L"AssetManager::get returned not-nullptr" );
				Assert::IsNull(    assetManager.get( path, 2 ).get( ), L"AssetManager::get returned not-nullptr" );

				assetManager.load( fileInfo2 );

				Assert::IsTrue(  assetManager.isLoaded( path, 0 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsTrue(  assetManager.isLoaded( path, 1 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsFalse( assetManager.isLoaded( path, 2 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsNotNull( assetManager.get( path, 0 ).get(), L"AssetManager::get returned nullptr" );
				Assert::IsNotNull( assetManager.get( path, 1 ).get(), L"AssetManager::get returned nullptr" );
				Assert::IsNull(    assetManager.get( path, 2 ).get(), L"AssetManager::get returned not-nullptr" );

				assetManager.load( fileInfo3 );

				Assert::IsTrue( assetManager.isLoaded( path, 0 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsTrue( assetManager.isLoaded( path, 1 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsTrue( assetManager.isLoaded( path, 2 ), L"AssetManager::isLoaded returned wrong value" );
				Assert::IsNotNull( assetManager.get( path, 0 ).get(), L"AssetManager::get returned nullptr" );
				Assert::IsNotNull( assetManager.get( path, 1 ).get(), L"AssetManager::get returned nullptr" );
				Assert::IsNotNull( assetManager.get( path, 2 ).get(), L"AssetManager::get returned nullptr" );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::load throw an exception" );
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

			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isLoaded returned false" );

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

			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Pyramid.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/triangle.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/Bunny.obj" ), L"AssetManager::isLoaded returned false" );

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

			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon2.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon3.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon4.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon5.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon6.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon7.obj" ), L"AssetManager::isLoaded returned false" );
			Assert::IsTrue( assetManager.isLoaded( "../Engine1/Assets/TestAssets/Meshes/dragon8.obj" ), L"AssetManager::isLoaded returned false" );

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

		TEST_METHOD( AssetManager_Mesh_Async_Loading_7 )
		{
			#if defined _DEBUG
			int meshMaxLoadingTimeMilisec = 10000;
			#else
			int meshMaxLoadingTimeMilisec = 5000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			const std::string path = "../Engine1/Assets/TestAssets/Meshes/Bikini_Girl.dae";

			BlockMeshFileInfo fileInfo1( path, BlockMeshFileInfo::Format::DAE, 0 );
			BlockMeshFileInfo fileInfo2( path, BlockMeshFileInfo::Format::DAE, 2 );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( meshMaxLoadingTimeMilisec ) );

			Assert::IsTrue(  assetManager.isLoaded( path, 0 ), L"AssetManager::isLoaded returned false" );
			Assert::IsFalse( assetManager.isLoaded( path, 1 ), L"AssetManager::isLoaded returned true" );
			Assert::IsTrue(  assetManager.isLoaded( path, 2 ), L"AssetManager::isLoaded returned false" );
		}

		TEST_METHOD( AssetManager_Mesh_GetWhenLoaded_1 )
		{
			#if defined _DEBUG
			float meshMaxLoadingTime = 20000;
			#else
			float meshMaxLoadingTime = 15000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			const std::string path = "../Engine1/Assets/TestAssets/Meshes/Bikini_Girl.dae";
			const std::string path2 = "../Engine1/Assets/TestAssets/Meshes/dragon.obj";
			const std::string path3 = "../Engine1/Assets/TestAssets/Meshes/Bunny.obj";
			const std::string path4 = "../Engine1/Assets/TestAssets/Meshes/dragon2.obj";


			BlockMeshFileInfo fileInfo1( path,  BlockMeshFileInfo::Format::DAE, 0 );
			BlockMeshFileInfo fileInfo2( path,  BlockMeshFileInfo::Format::DAE, 2 );
			BlockMeshFileInfo fileInfo3( path2, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo4( path3, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( path4, BlockMeshFileInfo::Format::OBJ );


			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::shared_ptr<Asset> asset1 = assetManager.get( path4 );
			Assert::IsNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get() );

			asset1 = assetManager.getWhenLoaded( path4, 0, meshMaxLoadingTime );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get() );
		}

		TEST_METHOD( AssetManager_Performance_Sync_Loading_1 )
		{
			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			const std::string path1 = "../Engine1/Assets/TestAssets/Meshes/Bikini_Girl.dae";
			const std::string path2 = "../Engine1/Assets/TestAssets/Meshes/quadbot.obj";
			const std::string path3 = "../Engine1/Assets/TestAssets/Meshes/Bunny.obj";
			const std::string path4 = "../Engine1/Assets/TestAssets/Meshes/dragon2.obj";
			const std::string path5 = "../Engine1/Assets/TestAssets/Meshes/quadbot2.obj";
			const std::string path6 = "../Engine1/Assets/TestAssets/Meshes/dragon4.obj";
			const std::string path7 = "../Engine1/Assets/TestAssets/Meshes/dragon5.obj";
			const std::string path8 = "../Engine1/Assets/TestAssets/Meshes/dragon6.obj";
			const std::string path9 = "../Engine1/Assets/TestAssets/Meshes/dragon7.obj";
			const std::string path10 = "../Engine1/Assets/TestAssets/Meshes/dragon8.obj";

			BlockMeshFileInfo fileInfo1( path1, BlockMeshFileInfo::Format::DAE, 0 );
			BlockMeshFileInfo fileInfo2( path2, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( path1, BlockMeshFileInfo::Format::DAE, 1 );
			BlockMeshFileInfo fileInfo4( path3, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( path4, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo6( path1, BlockMeshFileInfo::Format::DAE, 2 );
			BlockMeshFileInfo fileInfo7( path5, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo8( path6, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo9( path7, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo10( path8, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo11( path9, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo12( path10, BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.load( fileInfo1 );
				assetManager.load( fileInfo2 );
				assetManager.load( fileInfo3 );
				assetManager.load( fileInfo4 );
				assetManager.load( fileInfo5 );
				assetManager.load( fileInfo6 );
				assetManager.load( fileInfo7 );
				assetManager.load( fileInfo8 );
				assetManager.load( fileInfo9 );
				assetManager.load( fileInfo10 );
				assetManager.load( fileInfo11 );
				assetManager.load( fileInfo12 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::load threw an exception" );
			}
		}

		TEST_METHOD( AssetManager_Performance_Async_Loading_1 )
		{
			#if defined _DEBUG
			float meshMaxLoadingTime = 20000;
			#else
			float meshMaxLoadingTime = 5000;
			#endif

			unsigned int cpuThreadCount = std::thread::hardware_concurrency();
			if ( cpuThreadCount <= 0 ) cpuThreadCount = 1;

			AssetManager assetManager( cpuThreadCount );

			const std::string path1 = "../Engine1/Assets/TestAssets/Meshes/Bikini_Girl.dae";
			const std::string path2 = "../Engine1/Assets/TestAssets/Meshes/quadbot.obj";
			const std::string path3 = "../Engine1/Assets/TestAssets/Meshes/Bunny.obj";
			const std::string path4 = "../Engine1/Assets/TestAssets/Meshes/dragon2.obj";
			const std::string path5 = "../Engine1/Assets/TestAssets/Meshes/quadbot2.obj";
			const std::string path6 = "../Engine1/Assets/TestAssets/Meshes/dragon4.obj";
			const std::string path7 = "../Engine1/Assets/TestAssets/Meshes/dragon5.obj";
			const std::string path8 = "../Engine1/Assets/TestAssets/Meshes/dragon6.obj";
			const std::string path9 = "../Engine1/Assets/TestAssets/Meshes/dragon7.obj";
			const std::string path10 = "../Engine1/Assets/TestAssets/Meshes/dragon8.obj";

			BlockMeshFileInfo fileInfo1( path1, BlockMeshFileInfo::Format::DAE, 0 );
			BlockMeshFileInfo fileInfo2( path2, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo3( path1, BlockMeshFileInfo::Format::DAE, 1 );
			BlockMeshFileInfo fileInfo4( path3, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo5( path4, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo6( path1, BlockMeshFileInfo::Format::DAE, 2 );
			BlockMeshFileInfo fileInfo7( path5, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo8( path6, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo9( path7, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo10( path8, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo11( path9, BlockMeshFileInfo::Format::OBJ );
			BlockMeshFileInfo fileInfo12( path10, BlockMeshFileInfo::Format::OBJ );

			try {
				assetManager.loadAsync( fileInfo1 );
				assetManager.loadAsync( fileInfo2 );
				assetManager.loadAsync( fileInfo3 );
				assetManager.loadAsync( fileInfo4 );
				assetManager.loadAsync( fileInfo5 );
				assetManager.loadAsync( fileInfo6 );
				assetManager.loadAsync( fileInfo7 );
				assetManager.loadAsync( fileInfo8 );
				assetManager.loadAsync( fileInfo9 );
				assetManager.loadAsync( fileInfo10 );
				assetManager.loadAsync( fileInfo11 );
				assetManager.loadAsync( fileInfo12 );
			} catch ( ... ) {
				Assert::Fail( L"AssetManager::loadAsync threw an exception" );
			}

			std::shared_ptr<Asset> asset1 = assetManager.getWhenLoaded( path1, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset2 = assetManager.getWhenLoaded( path2, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset3 = assetManager.getWhenLoaded( path1, 1, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset4 = assetManager.getWhenLoaded( path3, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset5 = assetManager.getWhenLoaded( path4, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset6 = assetManager.getWhenLoaded( path1, 2, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset7 = assetManager.getWhenLoaded( path5, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset8 = assetManager.getWhenLoaded( path6, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset9 = assetManager.getWhenLoaded( path7, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset10 = assetManager.getWhenLoaded( path8, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset11 = assetManager.getWhenLoaded( path9, 0, meshMaxLoadingTime );
			std::shared_ptr<Asset> asset12 = assetManager.getWhenLoaded( path10, 0, meshMaxLoadingTime );

			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset1 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset2 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset3 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset4 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset5 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset6 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset7 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset8 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset9 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset10 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset11 ).get() );
			Assert::IsNotNull( std::dynamic_pointer_cast<BlockMesh>( asset12 ).get() );
		}
	};
}