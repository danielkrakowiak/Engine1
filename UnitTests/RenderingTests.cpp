#include "stdafx.h"
#include "CppUnitTest.h"

#include <experimental/filesystem>
#include <d3d11_3.h>
#include <wrl.h>

#include "RenderingTester.h"
#include "AssetManager.h"
#include "SceneManager.h"
#include "DX11RendererCore.h"
#include "Profiler.h"
#include "Renderer.h"
#include "RenderTargetManager.h"
#include "StringUtil.h"
#include "Settings.h"
#include "AssetPathManager.h"


using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(RenderingTests)
	{
		private:

		Microsoft::WRL::ComPtr< ID3D11Device3 > testDevice;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > testDeviceContext;

		TEST_METHOD_INITIALIZE( initTest ) 
        {
			testDevice = nullptr;
			testDeviceContext = nullptr;

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

            // Enable debug layer if in debug mode.
	        unsigned int flags = D3D11_CREATE_DEVICE_DEBUG;

            Microsoft::WRL::ComPtr< ID3D11Device > basicDevice;
            Microsoft::WRL::ComPtr< ID3D11DeviceContext > basicDeviceContext;

			HRESULT result = D3D11CreateDevice( 
                nullptr, D3D_DRIVER_TYPE_HARDWARE, 
                nullptr, flags, &featureLevel, 1, 
                D3D11_SDK_VERSION, 
                basicDevice.ReleaseAndGetAddressOf(), 
                nullptr, 
                basicDeviceContext.ReleaseAndGetAddressOf() );

			if ( result < 0 ) {
                throw std::exception( "Device creation failed." );
            }

            result = basicDevice.As( &testDevice );
            basicDeviceContext.As( &testDeviceContext );

            if ( result < 0 ) {
                throw std::exception( "Creation of DirectX 11.3 device failed" );
            }

            // Modify current path to point to root project directory.
            auto currentPath = std::experimental::filesystem::v1::current_path();
            currentPath = currentPath.parent_path();
            currentPath = currentPath.parent_path();
            std::experimental::filesystem::v1::current_path(currentPath);
		}

		TEST_METHOD_CLEANUP( cleanupTest ) 
        {
			testDeviceContext.Reset();
			testDevice.Reset();
		}

		public:

		TEST_METHOD( RenderingTest ) 
        {
            try
            {
                const auto parallelThreadCount = std::thread::hardware_concurrency( ) > 0 ? std::thread::hardware_concurrency( ) : 1;

                AssetManager assetManager;
                assetManager.initialize( parallelThreadCount, parallelThreadCount, testDevice );

                SceneManager sceneManager( assetManager );
                sceneManager.initialize( testDevice, testDeviceContext );

	            DX11RendererCore rendererCore;
                rendererCore.initialize( *testDeviceContext.Get() );

                Profiler profiler;
                profiler.initialize( testDevice, testDeviceContext );

                RenderTargetManager renderTargetManager;
                renderTargetManager.initialize( testDevice );

                Renderer renderer(rendererCore, profiler, renderTargetManager );
                renderer.initialize( 
                    settings().main.screenDimensions, 
                    testDevice, 
                    testDeviceContext, 
                    nullptr 
                );

                // We need to switch AssetPathManager to test-assets directory. 
                // Otherwise it would look for assets in the normal assets directory.
                AssetPathManager::get().scanDirectory( "TestAssets" );

                RenderingTester renderingTester( sceneManager, assetManager, rendererCore, renderer );
                renderingTester.initialize();

                const auto result = renderingTester.runTests();

                Assert::IsTrue(result.empty(), StringUtil::widen( result ).c_str() );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}
    };
}