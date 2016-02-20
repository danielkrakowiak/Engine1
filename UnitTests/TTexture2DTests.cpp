#include "stdafx.h"
#include "CppUnitTest.h"

#include <d3d11.h>

#include "TTexture2D.h"
#include "uchar4.h"

// Tests to add:
// When uploaded a texture to GPU, removed from CPU - get width and other getters should not throw exception for valid mipmap levels.

using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(TTexture2DTests)
	{
		private:

		ID3D11Device* testDevice;
		ID3D11DeviceContext* testDeviceContext;

		TEST_METHOD_INITIALIZE( initTest ) 
        {
			testDevice = nullptr;
			testDeviceContext = nullptr;

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

			HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &testDevice, nullptr, &testDeviceContext );
			if ( result < 0 ) throw std::exception( "Device creation failed." );

            BOOL success = SetCurrentDirectoryW( L"F:/Projekty/Engine1/Engine1/" );
            if ( !success ) throw std::exception( "Failed to set current path for tests." );
		}

		TEST_METHOD_CLEANUP( cleanupTest ) 
        {
			if ( testDeviceContext ) {
				testDeviceContext->Release( );
				testDeviceContext = nullptr;
			}

			if ( testDevice ) {
				testDevice->Release( );
				testDevice = nullptr;
			}
		}

        template< TTexture2DUsage usage, TTexture2DBinding binding, typename PixelType, DXGI_FORMAT format >
        void Texture_Initial_State_1_Test()
        {
            const int  width      = 256;
            const int  height     = 256;
            const bool storeOnCpu = true;
            const bool storeOnGpu = true;

            const int expectedLineSize = width * sizeof( PixelType );
            const int excpectedSize    = width * height * sizeof( PixelType );
            
            try {
			    TTexture2D< usage, binding, PixelType, format > texture( *testDevice, width, height, storeOnCpu, storeOnGpu );
            
			    Assert::IsTrue( texture.isInCpuMemory() == storeOnCpu );
			    Assert::IsTrue( texture.isInGpuMemory( ) == storeOnGpu );
                Assert::IsTrue( texture.getWidth() == width );
                Assert::IsTrue( texture.getHeight() == height );
                Assert::IsTrue( texture.getSize() == excpectedSize );
                Assert::IsTrue( texture.getLineSize() == expectedLineSize );
			    Assert::IsTrue( texture.getBytesPerPixel() == sizeof( PixelType ) );
			    Assert::IsTrue( texture.getMipMapCountOnCpu( ) == 1 );
			    Assert::IsTrue( texture.getMipMapCountOnGpu( ) == 1 );
            } catch ( ... ) {
                Assert::Fail();
            }
        }

		public:

		TEST_METHOD( Texture_Initial_State_1 ) 
        {
            typedef uchar4 PixelType;
            typedef uchar4 PixelTypeDepth;

            const DXGI_FORMAT format      = DXGI_FORMAT_B8G8R8A8_UNORM;
            const DXGI_FORMAT formatDepth = DXGI_FORMAT_D24_UNORM_S8_UINT;

            Texture_Initial_State_1_Test< TTexture2DUsage::Default, TTexture2DBinding::DepthStencil, PixelTypeDepth, DXGI_FORMAT_D24_UNORM_S8_UINT >();
            Texture_Initial_State_1_Test< TTexture2DUsage::Default, TTexture2DBinding::DepthStencil_ShaderResource, PixelTypeDepth, DXGI_FORMAT_R24G8_TYPELESS >();
            //Texture_Initial_State_1_Test< TTexture2DUsage::Default, TTexture2DBinding::DepthStencil_UnorderedAccess, PixelTypeDepth, formatDepth >();
            //Texture_Initial_State_1_Test< TTexture2DUsage::Default, TTexture2DBinding::DepthStencil_UnorderedAccess_ShaderResource, PixelTypeDepth, formatDepth >();

            std::vector< TTexture2DUsage > usages = { 
                TTexture2DUsage::Default, 
                TTexture2DUsage::Dynamic, 
                TTexture2DUsage::Immutable, 
                TTexture2DUsage::StagingRead, 
                TTexture2DUsage::StagingReadWrite, 
                TTexture2DUsage::StagingWrite
            };

            std::vector< TTexture2DBinding > bindings = {
                TTexture2DBinding::DepthStencil,
                TTexture2DBinding::DepthStencil_ShaderResource,
                TTexture2DBinding::DepthStencil_UnorderedAccess,
                TTexture2DBinding::DepthStencil_UnorderedAccess_ShaderResource,
                TTexture2DBinding::RenderTarget,
                TTexture2DBinding::RenderTarget_ShaderResource,
                TTexture2DBinding::RenderTarget_UnorderedAccess,
                TTexture2DBinding::RenderTarget_UnorderedAccess_ShaderResource,
                TTexture2DBinding::ShaderResource,
                TTexture2DBinding::UnorderedAccess,
                TTexture2DBinding::UnorderedAccess_ShaderResource
            };

		}
    };
}