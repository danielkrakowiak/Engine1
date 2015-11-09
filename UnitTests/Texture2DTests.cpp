#include "stdafx.h"
#include "CppUnitTest.h"

#include "Texture2D.h"

#include "uint3.h"

#include <d3d11.h>

// Tests to add:
// When uploaded a texture to GPU, removed from CPU - get width and other getters should not throw exception for valid mipmap levels.

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(Texture2DTests)
	{
		private:

		ID3D11Device* testDevice;
		ID3D11DeviceContext* testDeviceContext;

		TEST_METHOD_INITIALIZE( initTest ) {
			testDevice = nullptr;
			testDeviceContext = nullptr;

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

			HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &testDevice, nullptr, &testDeviceContext );
			if ( result < 0 )
				throw std::exception("Device creation failed.");
		}

		TEST_METHOD_CLEANUP( cleanupTest ) {
			if ( testDeviceContext ) {
				testDeviceContext->Release( );
				testDeviceContext = nullptr;
			}

			if ( testDevice ) {
				testDevice->Release( );
				testDevice = nullptr;
			}
		}

		public:

		TEST_METHOD( Texture_Initial_State_1 ) {
			Texture2D texture;

			ID3D11Device* device = testDevice;
			ID3D11DeviceContext* deviceContext = testDeviceContext;

			Assert::IsFalse( texture.isInCpuMemory(), L"Texture2D::isInCpuMemory() returned true" );
			Assert::IsFalse( texture.isInGpuMemory( ), L"Texture2D::isInGpuMemory() returned true" );
			Assert::IsTrue( texture.getBytesPerPixel() == 0, L"Texture2D::getBytesPerPixel() returned incorrect value" );
			Assert::IsFalse( texture.hasMipMapsOnCpu( ), L"Texture2D::hasMipMapsOnCpu() returned incorrect value" );
			Assert::IsFalse( texture.hasMipMapsOnGpu( ), L"Texture2D::hasMipMapsOnGpu() returned incorrect value" );
			Assert::IsTrue( texture.getMipMapCountOnCpu( ) == 0, L"Texture2D::getMipMapCountOnCpu() returned incorrect value" );
			Assert::IsTrue( texture.getMipMapCountOnGpu( ) == 0, L"Texture2D::getMipMapCountOnGpu() returned incorrect value" );

			auto getHeight = [&texture] { texture.getHeight( ); };
			Assert::ExpectException<std::exception>( getHeight, L"Texture2D::getHeight() - Exception not thrown (or incorrect exception thrown)" );
			auto getWidth = [&texture] { texture.getWidth( ); };
			Assert::ExpectException<std::exception>( getWidth, L"Texture2D::getWidth() - Exception not thrown (or incorrect exception thrown)" );
			auto getSize = [&texture] { texture.getSize( ); };
			Assert::ExpectException<std::exception>( getSize, L"Texture2D::getSize() - Exception not thrown (or incorrect exception thrown)" );
			auto getLineSize = [&texture] { texture.getLineSize( ); };
			Assert::ExpectException<std::exception>( getLineSize, L"Texture2D::getLineSize() - Exception not thrown (or incorrect exception thrown)" );
			auto getData = [&texture] { texture.getData( ); };
			Assert::ExpectException<std::exception>( getData, L"Texture2D::getData() - Exception not thrown (or incorrect exception thrown)" );
			auto getTexture = [&texture] { texture.getTexture( ); };
			Assert::ExpectException<std::exception>( getTexture, L"Texture2D::getTexture() - Exception not thrown (or incorrect exception thrown)" );
			auto getShaderResource = [&texture] { texture.getShaderResource( ); };
			Assert::ExpectException<std::exception>( getShaderResource, L"Texture2D::getSize() - Exception not thrown (or incorrect exception thrown)" );
			auto generateMipMapsOnCpu = [&texture] { texture.generateMipMapsOnCpu( ); };
			Assert::ExpectException<std::exception>( generateMipMapsOnCpu, L"Texture2D::generateMipMapsOnCpu() - Exception not thrown (or incorrect exception thrown)" );
			auto removeMipMapsOnCpu = [&texture] { texture.removeMipMapsOnCpu( ); };
			Assert::ExpectException<std::exception>( removeMipMapsOnCpu, L"Texture2D::removeMipMapsOnCpu() - Exception not thrown (or incorrect exception thrown)" );
			auto generateMipMapsOnGpu = [&texture, deviceContext] { texture.generateMipMapsOnGpu( *deviceContext ); };
			Assert::ExpectException<std::exception>( generateMipMapsOnGpu, L"Texture2D::generateMipMapsOnGpu() - Exception not thrown (or incorrect exception thrown)" );
		}
		
		TEST_METHOD(Texture_Loading_File_1)
		{
			std::shared_ptr<Texture2D> texture = nullptr;

			ID3D11Device* device = testDevice;
			ID3D11DeviceContext* deviceContext = testDeviceContext;

			try {
				texture = Texture2D::createFromFile( "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - normal.jpg", Texture2DFileInfo::Format::JPEG );

				Assert::IsTrue( texture->isInCpuMemory(), L"Texture2D::isInCpuMemory() returned false" );
				Assert::IsFalse( texture->isInGpuMemory( ), L"Texture2D::isInGpuMemory() returned true" );
				Assert::IsTrue( texture->getBytesPerPixel( ) == 4, L"Texture2D::getBytesPerPixel() returned incorrect value" );
				Assert::IsTrue( texture->getHeight( ) == 1024, L"Texture2D::getHeight() returned incorrect value" );
				Assert::IsTrue( texture->getWidth( ) == 1024, L"Texture2D::getWidth() returned incorrect value" );
				Assert::IsTrue( texture->getLineSize( ) == 1024 * 4, L"Texture2D::getLineSize() returned incorrect value" );
				Assert::IsTrue( texture->getSize( ) == 1024 * 1024 * 4, L"Texture2D::getSize() returned incorrect value" );
				Assert::IsTrue( !texture->getData( ).empty(), L"Texture2D::getData() returned empty vector" );
				Assert::IsFalse( texture->hasMipMapsOnCpu( ), L"Texture2D::hasMipMapsOnCpu() returned incorrect value" );
				Assert::IsFalse( texture->hasMipMapsOnGpu( ), L"Texture2D::hasMipMapsOnGpu() returned incorrect value" );
				Assert::IsTrue( texture->getMipMapCountOnCpu( ) == 1, L"Texture2D::getMipMapCountOnCpu() returned incorrect value" );
				Assert::IsTrue( texture->getMipMapCountOnGpu( ) == 0, L"Texture2D::getMipMapCountOnGpu() returned incorrect value" );
				texture->removeMipMapsOnCpu( );
			} catch ( ... ) {
				Assert::Fail( L"Exception thrown" );
			}

			auto getTexture = [&texture] { texture->getTexture(); };
			Assert::ExpectException<std::exception>( getTexture, L"Texture2D::getTexture() - Exception not thrown (or incorrect exception thrown)" );
			auto getShaderResource = [&texture] { texture->getShaderResource(); };
			Assert::ExpectException<std::exception>( getShaderResource, L"Texture2D::getSize() - Exception not thrown (or incorrect exception thrown)" );
			auto generateMipMapsOnGpu = [&texture, deviceContext] { texture->generateMipMapsOnGpu( *deviceContext ); };
			Assert::ExpectException<std::exception>( generateMipMapsOnGpu, L"Texture2D::generateMipMapsOnGpu() - Exception not thrown (or incorrect exception thrown)" );
		}

		TEST_METHOD( Texture_Generating_MipMaps_1 ) {
			std::shared_ptr<Texture2D> texture = nullptr;

			try {
				texture = Texture2D::createFromFile( "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - normal.jpg", Texture2DFileInfo::Format::JPEG );

				texture->generateMipMapsOnCpu();

				Assert::IsTrue( texture->isInCpuMemory(), L"Texture2D::isInCpuMemory() returned false" );
				Assert::IsFalse( texture->isInGpuMemory(), L"Texture2D::isInGpuMemory() returned true" );
				Assert::IsTrue( texture->getBytesPerPixel() == 4, L"Texture2D::getBytesPerPixel() returned incorrect value" );
				Assert::IsTrue( texture->getHeight() == 1024, L"Texture2D::getHeight() returned incorrect value" );
				Assert::IsTrue( texture->getWidth() == 1024, L"Texture2D::getWidth() returned incorrect value" );
				Assert::IsTrue( texture->getLineSize() == 1024 * 4, L"Texture2D::getLineSize() returned incorrect value" );
				Assert::IsTrue( texture->getSize() == 1024 * 1024 * 4, L"Texture2D::getSize() returned incorrect value" );
				Assert::IsTrue( !texture->getData().empty(), L"Texture2D::getData() returned empty vector" );
				Assert::IsTrue( texture->hasMipMapsOnCpu(), L"Texture2D::hasMipMapsOnCpu() returned incorrect value" );
				Assert::IsFalse( texture->hasMipMapsOnGpu(), L"Texture2D::hasMipMapsOnGpu() returned incorrect value" );
				Assert::IsTrue( texture->getMipMapCountOnCpu() == 11, L"Texture2D::getMipMapCountOnCpu() returned incorrect value" );
				Assert::IsTrue( texture->getMipMapCountOnGpu() == 0, L"Texture2D::getMipMapCountOnGpu() returned incorrect value" );
			} catch ( ... ) {
				Assert::Fail( L"Exception thrown" );
			}
		}

		//TEST_METHOD( Texture_Loading_File_2 ) {
		//	Texture2D texture( "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png", AssetFileFormat::PNG );
		//	try {
		//		texture.loadFile( );
		//		texture.load( );
		//		Assert::IsTrue( texture.isLoaded( ), L"Texture2D::isLoaded() returned false" );
		//		Assert::IsFalse( texture.isInGpuMemory( ), L"Texture2D::isInGpuMemory() returned true" );

		//		Assert::AreEqual( texture.getDimensions( ).x, 1024, L"Texture2D::getDimensions() returned incorrect width value" );
		//		Assert::AreEqual( texture.getDimensions( ).y, 1024, L"Texture2D::getDimensions() returned incorrect height value" );
		//		Assert::AreEqual( texture.getBitsPerPixel( ), 32, L"Texture2D::getBitsPerPixel() returned incorrect value" );
		//		Assert::AreEqual( texture.getMipMapCount( ), 1, L"Texture2D::getMipMapCount() returned incorrect value" );
		//		unsigned char* data = texture.getData( );

		//		//TODO: image seems to be flipped vertically - is it desired and normal?
		//		Assert::IsTrue( data[ 0 ] == 198 && data[ 1 ] == 198 && data[ 2 ] == 198 && data[ 3 ] == 255, L"Texture2D::getData() - data is incorrect" );
		//		//int offset = 44 * 4;
		//		//Assert::IsTrue( data[ offset + 0 ] == 194 && data[ offset + 1 ] == 194 && data[ offset + 2 ] == 194 && data[ offset + 3 ] == 255, L"Texture2D::getData() - data is incorrect" );
		//		//offset = ( 165 + 130 * 1024 ) * 4;
		//		//Assert::IsTrue( data[ offset + 0 ] == 196 && data[ offset + 1 ] == 196 && data[ offset + 2 ] == 196 && data[ offset + 3 ] == 255, L"Texture2D::getData() - data is incorrect" );
		//		
		//	} catch ( ... ) {
		//		Assert::Fail( L"Exception thrown" );
		//	}
		//}

		//TEST_METHOD( Texture_Loading_File_3 ) {
		//	Texture2D texture( "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png", AssetFileFormat::PNG );
		//	try {
		//		texture.getDimensions();
		//		Assert::Fail( L"Exception not thrown" );
		//		texture.getBitsPerPixel();
		//	} catch ( ... ) {
		//	}

		//	try {
		//		texture.getBitsPerPixel( );
		//		Assert::Fail( L"Exception not thrown" );
		//	} catch ( ... ) {
		//	}

		//	try {
		//		texture.getData( );
		//		Assert::Fail( L"Exception not thrown" );
		//	} catch ( ... ) {
		//	}
		//}

		////TEST_METHOD( Texture_Loading_File_4 ) {
		////	Texture2D texture;
		////	try {
		////		texture.loadFile( "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png" );
		////		texture.load( TextureFileFormat::BMP ); //incorrect file format
		////		Assert::Fail( L"Exception not thrown" );
		////	} catch ( ... ) {
		////	}
		////}

		//TEST_METHOD( Texture_Unloading_1 ) {
		//	std::string path = "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png";

		//	Texture2D texture( path, AssetFileFormat::PNG );

		//	try {
		//		texture.unloadFile( );
		//		texture.unload( );
		//		texture.unloadFromGpu( );
		//	} catch ( ... ) {
		//		Assert::Fail( L"Texture2D::unloadFile() or Texture2D::unload() or Texture2D::unloadFromGpu() - Exception thrown" );
		//	}
		//}

		//TEST_METHOD( Texture_Unloading_2 ) {
		//	std::string path = "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png";

		//	Texture2D texture( path, AssetFileFormat::PNG );
		//	texture.loadFile();

		//	try {
		//		texture.unloadFile( );
		//		Assert::IsFalse( texture.isInCpuMemory( ), L"Texture2D::isInCpuMemory() returned true" );
		//	} catch ( ... ) {
		//		Assert::Fail( L"Exception thrown" );
		//	}
		//}

		//TEST_METHOD( Texture_Unloading_3 ) {
		//	std::string path = "../Engine1/Assets/TestAssets/Textures/Floor1/floor1 - diffuse.png";

		//	Texture2D texture( path, AssetFileFormat::PNG );
		//	texture.loadFile( );
		//	texture.load();

		//	try {
		//		texture.unloadFile( );
		//		texture.unload( );

		//		Assert::IsFalse( texture.isInCpuMemory( ), L"Texture2D::isInCpuMemory() returned true" );
		//		Assert::IsFalse( texture.isLoaded( ), L"Texture2D::isLoaded() returned true" );
		//		
		//	} catch ( ... ) {
		//		Assert::Fail( L"Exception thrown" );
		//	}

		//	try {
		//		texture.generateMipMaps( );
		//		Assert::Fail( L"Texture2D::generateMipMaps() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getBitsPerPixel( );
		//		Assert::Fail( L"Texture2D::getBitsPerPixel() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getData( );
		//		Assert::Fail( L"Texture2D::getData() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getDimensions( );
		//		Assert::Fail( L"Texture2D::getDimensions() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getMipMapCount( );
		//		Assert::Fail( L"Texture2D::getMipMapCount() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getTexture( );
		//		Assert::Fail( L"Texture2D::getTexture() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.getTextureResource( );
		//		Assert::Fail( L"Texture2D::getTextureResource() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.hasMipMaps( );
		//		Assert::Fail( L"Texture2D::hasMipMaps() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		texture.load( );
		//		Assert::Fail( L"Texture2D::load() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//	try {
		//		//TODO: get device, deviceContext
		//		//texture.loadToGpu( );
		//		//Assert::Fail( L"Texture2D::loadToGpu() - Exception not thrown on illegal operation" );
		//	} catch ( ... ) {}
		//}

	};
}