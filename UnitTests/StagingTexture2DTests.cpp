#include "stdafx.h"
//#include "CppUnitTest.h"
//
//#include <d3d11.h>
//
//#include "TTexture2D.h"
//#include "uchar4.h"
//
//using namespace Engine1;
//using namespace Microsoft::VisualStudio::CppUnitTestFramework;
//
//namespace UnitTests
//{
//	TEST_CLASS(StagingTexture2DTests)
//	{
//		private:
//
//		ID3D11Device* testDevice;
//		ID3D11DeviceContext* testDeviceContext;
//
//		TEST_METHOD_INITIALIZE( initTest ) 
//        {
//			testDevice = nullptr;
//			testDeviceContext = nullptr;
//
//			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
//
//            // Enable debug layer if in debug mode.
//	        unsigned int flags = D3D11_CREATE_DEVICE_DEBUG;
//
//			HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &featureLevel, 1, D3D11_SDK_VERSION, &testDevice, nullptr, &testDeviceContext );
//			if ( result < 0 ) throw std::exception( "Device creation failed." );
//
//            BOOL success = SetCurrentDirectoryW( L"F:/Projekty/Engine1/Engine1/" );
//            if ( !success ) throw std::exception( "Failed to set current path for tests." );
//		}
//
//		TEST_METHOD_CLEANUP( cleanupTest ) 
//        {
//			if ( testDeviceContext ) {
//				testDeviceContext->Release( );
//				testDeviceContext = nullptr;
//			}
//
//			if ( testDevice ) {
//				testDevice->Release( );
//				testDevice = nullptr;
//			}
//		}
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< StagingTexture2D< TexUsage::StagingRead, PixelType > > > createTexturesStagingReadUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingRead, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingRead, TexBind::None, PixelType > >
//                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > createTexturesStagingReadWriteUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
//                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > createTexturesStagingReadWriteUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
//                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > createTexturesStagingReadWriteUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
//                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > createTexturesStagingReadWriteUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingReadWrite, TexBind::None, PixelType > >
//                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > createTexturesStagingWriteUsageWithoutInitialData( const int width, const int height, const bool storeOnCpu, const bool storeOnGpu )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
//                ( *testDevice, width, height, storeOnCpu, storeOnGpu, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > createTexturesStagingWriteUsageFromFile( const Texture2DFileInfo& fileInfo, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
//                ( *testDevice, fileInfo, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > createTexturesStagingWriteUsageFromFileInMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
//                ( *testDevice, dataIt, dataEndIt, format, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        template< typename PixelType >
//        std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > createTexturesStagingWriteUsageFromRawData( const std::vector< PixelType >& data, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps )
//        {
//            std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > textures;
//
//            textures.push_back( std::make_shared< TTexture2D< TexUsage::StagingWrite, TexBind::None, PixelType > >
//                ( *testDevice, data, width, height, storeOnCpu, storeOnGpu, generateMipmaps, DXGI_FORMAT_R8G8B8A8_UINT ) );
//
//            return textures;
//        }
//
//        // #TODO: should move all these getters to TTexture2DBase.
//        template< typename PixelType >
//        void verifyTexture( const Texture2DGeneric< PixelType >& texture, const int width, const int height, const bool storeOnCpu, const bool storeOnGpu,
//                            const int expectedBytesPerPixel, const int expectedLineSize, const int excpectedSize )
//        {
//            try {
//			    Assert::IsTrue( texture.isInCpuMemory()  == storeOnCpu );
//			    Assert::IsTrue( texture.isInGpuMemory( ) == storeOnGpu );
//
//                if ( storeOnCpu || storeOnGpu )
//                {
//                    Assert::IsTrue( texture.getWidth()         == width );
//                    Assert::IsTrue( texture.getHeight()        == height );
//                    Assert::IsTrue( texture.getSize()          == excpectedSize );
//                    Assert::IsTrue( texture.getLineSize()      == expectedLineSize );
//                }
//                else
//                {
//                    try { texture.getWidth();         Assert::Fail(); } catch( ... ){}
//                    try { texture.getHeight();        Assert::Fail(); } catch( ... ){}
//                    try { texture.getSize();          Assert::Fail(); } catch( ... ){}
//                    try { texture.getLineSize();      Assert::Fail(); } catch( ... ){}
//                }
//
//                Assert::IsTrue( texture.getBytesPerPixel() == expectedBytesPerPixel );
//			    Assert::IsTrue( texture.getMipMapCountOnCpu( ) == (storeOnCpu ? 1 : 0) );
//			    Assert::IsTrue( texture.getMipMapCountOnGpu( ) == (storeOnGpu ? 1 : 0) );
//                
//            } catch ( ... ) {
//                Assert::Fail();
//            }
//        }
//
//		public:
//
//		TEST_METHOD( Texture_Create_Without_Initial_Data_1 ) 
//        {
//            typedef uchar4 PixelType;
//            const int  width                 = 256;
//            const int  height                = 256;
//            const bool storeOnCpu            = true;
//            const bool storeOnGpu            = true;
//            const int  expectedBytesPerPixel = sizeof( PixelType );
//            const int  excpectedLineSize     = width * sizeof( PixelType );
//            const int  excpectedSize         = width * height * sizeof( PixelType );
//            const bool generateMipmaps       = false;
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingRead, PixelType > > > texturesStagingReadUsage      
//                    = createTexturesStagingReadUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageWithoutInitialData< PixelType >( width, height, storeOnCpu, storeOnGpu );
//                
//                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
//                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
//                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
//                textures.insert( textures.begin(), texturesStagingReadUsage.begin(),      texturesStagingReadUsage.end() );
//                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
//                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );
//
//                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
//                    verifyTexture( *texture, width, height, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Create_From_File_1 ) 
//        {
//            Texture2DFileInfo fileInfo( "Assets/TestAssets/Textures/stone.png", Texture2DFileInfo::Format::PNG );
//
//            typedef uchar4 PixelType;
//            const int   expectedWidth         = 256;
//            const int   expectedHeight        = 256;
//            const bool  storeOnCpu            = true;
//            const bool  storeOnGpu            = true;
//            const int   expectedBytesPerPixel = sizeof( PixelType );
//            const int   excpectedLineSize     = expectedWidth * sizeof( PixelType );
//            const int   excpectedSize         = expectedWidth * expectedHeight * sizeof( PixelType );
//            const bool  generateMipmaps       = false;
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageFromFile< PixelType >( fileInfo, storeOnCpu, storeOnGpu, generateMipmaps );
//                
//                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
//                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
//                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
//                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
//                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
//                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );
//
//                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
//                    verifyTexture( *texture, expectedWidth, expectedHeight, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Create_From_File_In_Memory_1 ) 
//        {
//            std::shared_ptr< std::vector<char> > file   = BinaryFile::load( "Assets/TestAssets/Textures/stone.png" );
//            Texture2DFileInfo::Format            format = Texture2DFileInfo::Format::PNG;
//
//            typedef uchar4 PixelType;
//            const int   expectedWidth         = 256;
//            const int   expectedHeight        = 256;
//            const bool  storeOnCpu            = true;
//            const bool  storeOnGpu            = true;
//            const int   expectedBytesPerPixel = sizeof( PixelType );
//            const int   excpectedLineSize     = expectedWidth * sizeof( PixelType );
//            const int   excpectedSize         = expectedWidth * expectedHeight * sizeof( PixelType );
//            const bool  generateMipmaps       = false;
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageFromFileInMemory< PixelType >( file->cbegin(), file->cend(), format, storeOnCpu, storeOnGpu, generateMipmaps );
//                
//                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
//                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
//                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
//                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
//                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
//                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );
//
//                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
//                    verifyTexture( *texture, expectedWidth, expectedHeight, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Create_From_Raw_Data_1 ) 
//        {
//            typedef uchar4 PixelType;
//            const int  width                 = 256;
//            const int  height                = 256;
//            const bool storeOnCpu            = true;
//            const bool storeOnGpu            = true;
//            const int  expectedBytesPerPixel = sizeof( PixelType );
//            const int  excpectedLineSize     = width * sizeof( PixelType );
//            const int  excpectedSize         = width * height * sizeof( PixelType );
//            const bool generateMipmaps       = false;
//
//            std::vector< PixelType > data;
//            data.resize( width * height );
//
//            for ( int i = 0; i < width * height; ++i )
//                data[ i ] = uchar4( 0, 0, 0, 255 );
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//                
//                std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures;
//                textures.insert( textures.begin(), texturesImmutableUsage.begin(),        texturesImmutableUsage.end() );
//                textures.insert( textures.begin(), texturesDynamicUsage.begin(),          texturesDynamicUsage.end() );
//                textures.insert( textures.begin(), texturesDefaultUsage.begin(),          texturesDefaultUsage.end() );
//                textures.insert( textures.begin(), texturesStagingReadWriteUsage.begin(), texturesStagingReadWriteUsage.end() );
//                textures.insert( textures.begin(), texturesStagingWriteUsage.begin(),     texturesStagingWriteUsage.end() );
//
//                for ( const std::shared_ptr< Texture2DGeneric< PixelType > >& texture : textures )
//                    verifyTexture( *texture, width, height, storeOnCpu, storeOnGpu, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Load_Cpu_To_Gpu ) 
//        {
//            typedef uchar4 PixelType;
//            const int  width                 = 256;
//            const int  height                = 256;
//            const bool storeOnCpu            = true;
//            const bool storeOnGpu            = false;
//            const int  expectedBytesPerPixel = sizeof( PixelType );
//            const int  excpectedLineSize     = width * sizeof( PixelType );
//            const int  excpectedSize         = width * height * sizeof( PixelType );
//            const bool generateMipmaps       = false;
//
//            std::vector< PixelType > data;
//            data.resize( width * height );
//
//            for ( int i = 0; i < width * height; ++i )
//                data[ i ] = uchar4( 0, 0, 0, 255 );
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//                
//                for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > >& texture : texturesStagingReadWriteUsage ) {
//                    texture->loadCpuToGpu( *testDevice, *testDeviceContext );
//                    verifyTexture( *texture, width, height, storeOnCpu, true, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//                }
//
//                for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > >& texture : texturesStagingWriteUsage ) {
//                    texture->loadCpuToGpu( *testDevice, *testDeviceContext );
//                    verifyTexture( *texture, width, height, storeOnCpu, true, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//                }
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Unload_From_Gpu ) 
//        {
//            typedef uchar4 PixelType;
//            const int  width                 = 256;
//            const int  height                = 256;
//            const bool storeOnCpu            = true;
//            const bool storeOnGpu            = true;
//            const int  expectedBytesPerPixel = sizeof( PixelType );
//            const int  excpectedLineSize     = width * sizeof( PixelType );
//            const int  excpectedSize         = width * height * sizeof( PixelType );
//            const bool generateMipmaps       = false;
//
//            std::vector< PixelType > data;
//            data.resize( width * height );
//
//            for ( int i = 0; i < width * height; ++i )
//                data[ i ] = uchar4( 0, 0, 0, 255 );
//
//            try
//            {
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                    = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//
//                std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                    = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width, height, storeOnCpu, storeOnGpu, generateMipmaps );
//                
//                for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > >& texture : texturesStagingReadWriteUsage ) {
//                    texture->unloadFromGpu();
//                    verifyTexture( *texture, width, height, storeOnCpu, false, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//                }
//
//                for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > >& texture : texturesStagingWriteUsage ) {
//                    texture->unloadFromGpu();
//                    verifyTexture( *texture, width, height, storeOnCpu, false, expectedBytesPerPixel, excpectedLineSize, excpectedSize );
//                }
//            }
//            catch ( ... )
//            {
//                Assert::Fail();
//            }
//		}
//
//        TEST_METHOD( Texture_Unload_From_Cpu ) 
//        {
//            typedef uchar4 PixelType;
//            const int  paramSetCount         = 2;
//            const int  width[]                 = { 256, 256 };
//            const int  height[]                = { 256, 256 };
//            const bool storeOnCpu[]            = { true, true };
//            const bool storeOnGpu[]            = { false, true };
//            const int  expectedBytesPerPixel[] = { sizeof( PixelType ), sizeof( PixelType ) };
//            const int  excpectedLineSize[]     = { width[0] * sizeof( PixelType ), width[1] * sizeof( PixelType ) };
//            const int  excpectedSize[]         = { width[0] * height[0] * sizeof( PixelType ), width[1] * height[1] * sizeof( PixelType ) };
//            const bool generateMipmaps[]       = { false, false };
//
//            for ( int t = 0; t < paramSetCount; ++t )
//            {
//                std::vector< PixelType > data;
//                data.resize( width[t] * height[t] );
//
//                for ( int i = 0; i < width[t] * height[t]; ++i )
//                    data[ i ] = uchar4( 0, 0, 0, 255 );
//
//                try
//                {
//                    std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > > > texturesStagingReadWriteUsage 
//                        = createTexturesStagingReadWriteUsageFromRawData< PixelType >( data, width[t], height[t], storeOnCpu[t], storeOnGpu[t], generateMipmaps[t] );
//
//                    std::vector< std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > > > texturesStagingWriteUsage     
//                        = createTexturesStagingWriteUsageFromRawData< PixelType >( data, width[t], height[t], storeOnCpu[t], storeOnGpu[t], generateMipmaps[t] );
//                
//                    for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingReadWrite, PixelType > >& texture : texturesStagingReadWriteUsage ) {
//                        texture->unloadFromCpu();
//                        verifyTexture( *texture, width[t], height[t], false, storeOnGpu[t], expectedBytesPerPixel[t], excpectedLineSize[t], excpectedSize[t] );
//                    }
//
//                    for ( std::shared_ptr< Texture2DSpecializedUsage< TexUsage::StagingWrite, PixelType > >& texture : texturesStagingWriteUsage ) {
//                        texture->unloadFromCpu();
//                        verifyTexture( *texture, width[t], height[t], false, storeOnGpu[t], expectedBytesPerPixel[t], excpectedLineSize[t], excpectedSize[t] );
//                    }
//                }
//                catch ( ... )
//                {
//                    Assert::Fail();
//                }
//            }
//		}
//    };
//}