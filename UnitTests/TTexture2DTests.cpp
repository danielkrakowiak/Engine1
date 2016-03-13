#include "stdafx.h"
#include "CppUnitTest.h"

#include <d3d11.h>

#include "TTexture2D.h"
#include "uchar4.h"

// Tests to add:
// When uploaded a texture to GPU, removed from CPU - get width and other getters should not throw exception for valid mipmap levels.
// Create texture where PixelSize is different than the one in textureFormat passed in the constructor. Should throw exception.

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

            // Enable debug layer if in debug mode.
	        unsigned int flags = D3D11_CREATE_DEVICE_DEBUG;

			HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &featureLevel, 1, D3D11_SDK_VERSION, &testDevice, nullptr, &testDeviceContext );
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

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesImmutableUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesImmutableUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesImmutableUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDynamicUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDynamicUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDynamicUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > > > createTexturesDynamicUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > > > textures;

            textures.push_back( 
                std::dynamic_pointer_cast< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > >(
                    std::make_shared< TTexture2D< TexUsage::Dynamic, TexBind::ShaderResource, PixelType > >
                        ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) 
                )
            );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDefaultUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDefaultUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDefaultUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesDefaultUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            textures.push_back( std::make_shared< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingReadUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingRead, TexBind::None, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingReadWriteUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingReadWriteUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingReadWriteUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingReadWriteUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingWriteUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingWriteUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingWriteUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        template< typename PixelType >
        std::vector< std::shared_ptr< TTexture2DBase > > createTexturesStagingWriteUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
        {
            std::vector< std::shared_ptr< TTexture2DBase > > textures;

            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );

            return textures;
        }

        // #TODO: should move all these getters to TTexture2DBase.
        void verifyTexture( const TTexture2DBase& texture, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
                            const int expectedBytesPerPixel, const int expectedLineSize, const int excpectedSize )
        {
            try {
			    Assert::IsTrue( texture.isInCpuMemory()        == storeOnCpu );
			    Assert::IsTrue( texture.isInGpuMemory( )       == storeOnGpu );
                Assert::IsTrue( texture.getWidth()             == width );
                Assert::IsTrue( texture.getHeight()            == height );
                Assert::IsTrue( texture.getSize()              == excpectedSize );
                Assert::IsTrue( texture.getLineSize()          == expectedLineSize );
			    Assert::IsTrue( texture.getBytesPerPixel()     == expectedBytesPerPixel );
			    Assert::IsTrue( texture.getMipMapCountOnCpu( ) == 1 );
			    Assert::IsTrue( texture.getMipMapCountOnGpu( ) == 1 );
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
            const bool generateMipmaps       = false;

            try
            {
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDynamicUsage          = createTexturesDynamicUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDefaultUsage          = createTexturesDefaultUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadUsage      = createTexturesStagingReadUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadWriteUsage = createTexturesStagingReadWriteUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingWriteUsage     = createTexturesStagingWriteUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
                
                std::vector< std::shared_ptr< TTexture2DBase > > textures;
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
                textures.insert( textures.begin(), texturesStagingReadUsage.begin(),      texturesStagingReadUsage.end() );
                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );

                for ( const std::shared_ptr< TTexture2DBase >& texture : textures )
                    verifyTexture( *texture, width, height, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Create_From_File_1 ) 
        {
            Texture2DFileInfo fileInfo( "Assets/TestAssets/Textures/stone.png", Texture2DFileInfo::Format::PNG );

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
                std::vector< std::shared_ptr< TTexture2DBase > > texturesImmutableUsage        = createTexturesImmutableUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDynamicUsage          = createTexturesDynamicUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDefaultUsage          = createTexturesDefaultUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadWriteUsage = createTexturesStagingReadWriteUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingWriteUsage     = createTexturesStagingWriteUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< TTexture2DBase > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );

                for ( const std::shared_ptr< TTexture2DBase >& texture : textures )
                    verifyTexture( *texture, expectedWidth, expectedHeight, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}

        TEST_METHOD( Texture_Create_From_File_In_Memory_1 ) 
        {
            std::shared_ptr< std::vector<char> > file   = BinaryFile::load( "Assets/TestAssets/Textures/stone.png" );
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
                std::vector< std::shared_ptr< TTexture2DBase > > texturesImmutableUsage        = createTexturesImmutableUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDynamicUsage          = createTexturesDynamicUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDefaultUsage          = createTexturesDefaultUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadWriteUsage = createTexturesStagingReadWriteUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingWriteUsage     = createTexturesStagingWriteUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< TTexture2DBase > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );

                for ( const std::shared_ptr< TTexture2DBase >& texture : textures )
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
                std::vector< std::shared_ptr< TTexture2DBase > > texturesImmutableUsage        = createTexturesImmutableUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > > > texturesDynamicUsage          = createTexturesDynamicUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDefaultUsage          = createTexturesDefaultUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadWriteUsage = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingWriteUsage     = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                
                std::vector< std::shared_ptr< TTexture2DBase > > textures;
                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );

                for ( const std::shared_ptr< TTexture2DBase >& texture : textures )
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
            const bool generateMipmaps       = false;

            std::vector< PixelType > data;
            data.resize( width * height );

            for ( int i = 0; i < width * height; ++i )
                data[ i ] = uchar4( 0, 0, 0, 255 );

            try
            {
                std::vector< std::shared_ptr< TTexture2DBase > > texturesImmutableUsage        = createTexturesImmutableUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > > > texturesDynamicUsage          = createTexturesDynamicUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesDefaultUsage          = createTexturesDefaultUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingReadWriteUsage = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                std::vector< std::shared_ptr< TTexture2DBase > > texturesStagingWriteUsage     = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
                
                for ( std::shared_ptr< TTexture2D< TexUsage::Dynamic, TexBind::None, PixelType > >& texture : texturesDynamicUsage )
                {
                    texture->loadCpuToGpu( *testDevice, *testDeviceContext );
                }
            }
            catch ( ... )
            {
                Assert::Fail();
            }
		}
    };
}