#include "stdafx.h"
#include "CppUnitTest.h"

#include <experimental/filesystem>
#include <d3d11_3.h>

#include "Texture2D.h"
#include "uchar4.h"

// Tests to add:
// When uploaded a texture to GPU, removed from CPU - get width and other getters should not throw exception for valid mipmap levels.
// Create texture where PixelSize is different than the one in textureFormat passed in the constructor. Should throw exception.
//
// Test multiple configurations in each test using arrays of params.
// Test getting views for each binding.

using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(Texture2DTests)
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

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > createTexturesImmutableUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > createTexturesImmutableUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > createTexturesImmutableUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > createTexturesDynamicUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool hasMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > textures;

            textures.push_back(
                std::make_shared< Texture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                    ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT )
                );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > createTexturesDynamicUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > textures;

            textures.push_back( 
                std::make_shared< Texture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                    ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) 
            );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > createTexturesDynamicUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > textures;

            textures.push_back( 
                std::make_shared< Texture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                    ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) 
            );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > createTexturesDynamicUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > textures;

            textures.push_back( 
                std::make_shared< Texture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                    ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) 
            );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > createTexturesDefaultUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool hasMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                                ( *testDevice.Get(), width, height, storeOnCpu, storeOnGpu, hasMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > createTexturesDefaultUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > createTexturesDefaultUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > createTexturesDefaultUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > textures;

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice.Get(), data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        void verifyTexture( const Texture2DGeneric< PixelType >& texture, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                            const int expectedBytesPerPixel, const int expectedLineSize, const int excpectedSize )
        {
            try {
			    Assert::IsTrue( texture.isInCpuMemory()  == storeOnCpu );
			    Assert::IsTrue( texture.isInGpuMemory( ) == storeOnGpu );

                if ( storeOnCpu || storeOnGpu )
                {
                    Assert::IsTrue( texture.getWidth()    == width );
                    Assert::IsTrue( texture.getHeight()   == height );
                    Assert::IsTrue( texture.getSize()     == excpectedSize );
                    Assert::IsTrue( texture.getLineSize() == expectedLineSize );
                }
                else
                {
                    try { texture.getWidth();         Assert::Fail(); } catch( ... ){}
                    try { texture.getHeight();        Assert::Fail(); } catch( ... ){}
                    try { texture.getSize();          Assert::Fail(); } catch( ... ){}
                    try { texture.getLineSize();      Assert::Fail(); } catch( ... ){}
                }

                Assert::IsTrue( texture.getBytesPerPixel()     == expectedBytesPerPixel );
			    Assert::IsTrue( texture.getMipMapCountOnCpu( ) == (storeOnCpu ? 1 : 0) );
			    Assert::IsTrue( texture.getMipMapCountOnGpu( ) == (storeOnGpu ? 1 : 0) );
                
            } catch ( ... ) {
                Assert::Fail();
            }
        }

		public:

		TEST_METHOD( Texture_Create_Without_Initial_Data_1 ) 
        {
            typedef uchar4 PixelType;
            const int  width                 = 256;
            const int  height                = 256;
            const bool storeOnCpu            = true;
            const bool storeOnGpu            = true;
            const int  expectedBytesPerPixel = sizeof( PixelType );
            const int  excpectedLineSize     = width * sizeof( PixelType );
            const int  excpectedSize         = width * height * sizeof( PixelType );
            const bool hasMipmaps            = false;

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu, hasMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                    = createTexturesDefaultUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu, hasMipmaps );

                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
                textures.insert( textures.begin(), texturesDynamicUsage.begin(), texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(), texturesDefaultUsage.end() );

                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
                    verifyTexture( *texture, width, height, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Create_From_File_1 ) 
        {
            Texture2DFileInfo fileInfo( "TestAssets/Textures/stone.png", Texture2DFileInfo::Format::PNG, Texture2DFileInfo::PixelType::UCHAR4 );

            typedef uchar4 PixelType;
            const int   expectedWidth         = 256;
            const int   expectedHeight        = 256;
            const bool  storeOnCpu            = true;
            const bool  storeOnGpu            = true;
            const int   expectedBytesPerPixel = sizeof( PixelType );
            const int   excpectedLineSize     = expectedWidth * sizeof( PixelType );
            const int   excpectedSize         = expectedWidth * expectedHeight * sizeof( PixelType );
            const bool  generateMipmaps       = false;

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                    = createTexturesImmutableUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                    = createTexturesDefaultUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );

                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
                    verifyTexture( *texture, expectedWidth, expectedHeight, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Create_From_File_In_Memory_1 ) 
        {
            std::shared_ptr< std::vector<char> > file   = BinaryFile::load( "TestAssets/Textures/stone.png" );
            Texture2DFileInfo::Format            format = Texture2DFileInfo::Format::PNG;

            typedef uchar4 PixelType;
            const int   expectedWidth         = 256;
            const int   expectedHeight        = 256;
            const bool  storeOnCpu            = true;
            const bool  storeOnGpu            = true;
            const int   expectedBytesPerPixel = sizeof( PixelType );
            const int   excpectedLineSize     = expectedWidth * sizeof( PixelType );
            const int   excpectedSize         = expectedWidth * expectedHeight * sizeof( PixelType );
            const bool  generateMipmaps       = false;

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                    = createTexturesImmutableUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage         
                    = createTexturesDefaultUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );

                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
                    verifyTexture( *texture, expectedWidth, expectedHeight, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Create_From_Raw_Data_1 ) 
        {
            typedef uchar4 PixelType;
            const int  width                 = 256;
            const int  height                = 256;
            const bool storeOnCpu            = true;
            const bool storeOnGpu            = true;
            const int  expectedBytesPerPixel = sizeof( PixelType );
            const int  excpectedLineSize     = width * sizeof( PixelType );
            const int  excpectedSize         = width * height * sizeof( PixelType );
            const bool generateMipmaps       = false;

            std::vector< PixelType > data;
            data.resize( width * height );

            for ( int i = 0; i < width * height; ++i )
                data[ i ] = uchar4( 0, 0, 0, 255 );

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                    = createTexturesImmutableUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                    = createTexturesDefaultUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );

                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
                    verifyTexture( *texture, width, height, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Load_Cpu_To_Gpu ) 
        {
            typedef uchar4 PixelType;
            const int  width                 = 256;
            const int  height                = 256;
            const bool storeOnCpu            = true;
            const bool storeOnGpu            = false;
            const int  expectedBytesPerPixel = sizeof( PixelType );
            const int  excpectedLineSize     = width * sizeof( PixelType );
            const int  excpectedSize         = width * height * sizeof( PixelType );
            const bool generateMipmaps       = false;

            std::vector< PixelType > data;
            data.resize( width * height );

            for ( int i = 0; i < width * height; ++i )
                data[ i ] = uchar4( 0, 0, 0, 255 );

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                    = createTexturesImmutableUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                    = createTexturesDefaultUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > >& texture : texturesImmutableUsage ) {
                    texture->loadCpuToGpu( *testDevice.Get(), *testDeviceContext.Get() );
                    verifyTexture( *texture, width, height, storeOnCpu, true, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > >& texture : texturesDynamicUsage ) {
                    texture->loadCpuToGpu( *testDevice.Get(), *testDeviceContext.Get() );
                    verifyTexture( *texture, width, height, storeOnCpu, true, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > >& texture : texturesDefaultUsage ) {
                    texture->loadCpuToGpu( *testDevice.Get(), *testDeviceContext.Get() );
                    verifyTexture( *texture, width, height, storeOnCpu, true, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Unload_From_Gpu ) 
        {
            typedef uchar4 PixelType;
            const int  width                 = 256;
            const int  height                = 256;
            const bool storeOnCpu            = true;
            const bool storeOnGpu            = true;
            const int  expectedBytesPerPixel = sizeof( PixelType );
            const int  excpectedLineSize     = width * sizeof( PixelType );
            const int  excpectedSize         = width * height * sizeof( PixelType );
            const bool generateMipmaps       = false;

            std::vector< PixelType > data;
            data.resize( width * height );

            for ( int i = 0; i < width * height; ++i )
                data[ i ] = uchar4( 0, 0, 0, 255 );

            try
            {
                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                    = createTexturesImmutableUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                    = createTexturesDynamicUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                    = createTexturesDefaultUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > >& texture : texturesImmutableUsage ) {
                    texture->unloadFromGpu();
                    verifyTexture( *texture, width, height, storeOnCpu, false, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > >& texture : texturesDynamicUsage ) {
                    texture->unloadFromGpu();
                    verifyTexture( *texture, width, height, storeOnCpu, false, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }

                for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > >& texture : texturesDefaultUsage ) {
                    texture->unloadFromGpu();
                    verifyTexture( *texture, width, height, storeOnCpu, false, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
                }
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Unload_From_Cpu ) 
        {
            typedef uchar4 PixelType;
            const int  paramSetCount         = 2;
            const int  width[]                 = { 256, 256 };
            const int  height[]                = { 256, 256 };
            const bool storeOnCpu[]            = { true, true };
            const bool storeOnGpu[]            = { false, true };
            const int  expectedBytesPerPixel[] = { (int)sizeof( PixelType ), (int)sizeof( PixelType ) };
            const int  excpectedLineSize[]     = { width[0] * (int)sizeof( PixelType ), width[1] * (int)sizeof( PixelType ) };
            const int  excpectedSize[]         = { width[0] * height[0] * (int)sizeof( PixelType ), width[1] * height[1] * (int)sizeof( PixelType ) };
            const bool generateMipmaps[]       = { false, false };

            for ( int t = 0; t < paramSetCount; ++t )
            {
                std::vector< PixelType > data;
                data.resize( width[t] * height[t] );

                for ( int i = 0; i < width[t] * height[t]; ++i )
                    data[ i ] = uchar4( 0, 0, 0, 255 );

                try
                {
                    std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > > > texturesImmutableUsage        
                        = createTexturesImmutableUsageFromRawData< PixelType >( data, width[t], height[t], storeOnCpu[t], storeOnGpu[t], generateMipmaps[t] );

                    std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > > > texturesDynamicUsage          
                        = createTexturesDynamicUsageFromRawData< PixelType >( data, width[t], height[t], storeOnCpu[t], storeOnGpu[t], generateMipmaps[t] );

                    std::vector< std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > > > texturesDefaultUsage          
                        = createTexturesDefaultUsageFromRawData< PixelType >( data, width[t], height[t], storeOnCpu[t], storeOnGpu[t], generateMipmaps[t] );

                    for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Immutable, PixelType > >& texture : texturesImmutableUsage ) {
                        texture->unloadFromCpu();
                        verifyTexture( *texture, width[t], height[t], false, storeOnGpu[t], expectedBytesPerPixel[t], excpectedLineSize[t], excpectedSize[t] );
                    }

                    for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Dynamic, PixelType > >& texture : texturesDynamicUsage ) {
                        texture->unloadFromCpu();
                        verifyTexture( *texture, width[t], height[t], false, storeOnGpu[t], expectedBytesPerPixel[t], excpectedLineSize[t], excpectedSize[t] );
                    }

                    for ( std::shared_ptr< Texture2DSpecUsage< TexUsage::Default, PixelType > >& texture : texturesDefaultUsage ) {
                        texture->unloadFromCpu();
                        verifyTexture( *texture, width[t], height[t], false, storeOnGpu[t], expectedBytesPerPixel[t], excpectedLineSize[t], excpectedSize[t] );
                    }
                }
                catch ( ... )
                {
                    Assert::Fail();
                }
            }
		}
    };
}