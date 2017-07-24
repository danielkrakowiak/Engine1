#include "stdafx.h"
#include "CppUnitTest.h"

#include "BlockMesh.h"
//#include "OBJMeshFileParser.h"
#include "MathUtil.h"

#include <d3d11_3.h>

using namespace Engine1;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS( MeshTestSet ) {

	private:

		ID3D11Device* testDevice;
		ID3D11DeviceContext* testDeviceContext;

		TEST_METHOD_INITIALIZE( initTest ) {

			testDevice = nullptr;
			testDeviceContext = nullptr;

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

			HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &testDevice, nullptr, &testDeviceContext );
			if ( result < 0 )
				throw std::exception( "Device creation failed." );

            BOOL success = SetCurrentDirectoryW( L"F:/Projekty/Engine1/" );
            if ( !success )
                throw std::exception( "Failed to set current path for tests." );
		}

		TEST_METHOD_CLEANUP( cleanupTest ) {

			if ( testDeviceContext ) {
				testDeviceContext->Release();
				testDeviceContext = nullptr;
			}

			if ( testDevice ) {
				testDevice->Release();
				testDevice = nullptr;
			}
		}

	public:

		TEST_METHOD( Mesh_Initial_State_1 ) {
			BlockMesh mesh;

			try {
				Assert::IsFalse( mesh.isInCpuMemory(), L"BlockMesh::isInCpuMemory() returned true" );
				Assert::IsFalse( mesh.isInGpuMemory(), L"BlockMesh::isInGpuMemory() returned true" );

			} catch ( ... ) {
				Assert::Fail( L"Exception thrown" );
			}

			try {
				mesh.getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getNormals();
				Assert::Fail( L"BlockMesh::getNormals() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTexcoords();
				Assert::Fail( L"BlockMesh::getTexcoords() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTriangles();
				Assert::Fail( L"BlockMesh::getTriangles() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getVertices();
				Assert::Fail( L"BlockMesh::getVertices() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}

			try {
				mesh.loadCpuToGpu( *testDevice );
				Assert::Fail( L"BlockMesh::loadCpuToGpu() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh.getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
		}

		TEST_METHOD( Mesh_Loading_From_File_1 ) {
			std::shared_ptr<BlockMesh> mesh = nullptr;

			try {
				mesh = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				
                Assert::IsTrue( mesh->isInCpuMemory(), L"BlockMesh::isInCpuMemory() returned false" );
				Assert::IsFalse( mesh->isInGpuMemory(), L"BlockMesh::isInGpuMemory() returned true" );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw an exception" );
			}

			try {
				mesh->getNormals( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getNormals() threw an exception" );
			}
			try {
				mesh->getTexcoords( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getTexcoords() threw an exception" );
			}
			try {
				mesh->getTriangles( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getTriangles() threw an exception" );
			}
			try {
				mesh->getVertices( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getVertices() threw an exception" );
			}

			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
		}

		TEST_METHOD( Mesh_Loading_From_File_2 ) {

			std::shared_ptr<BlockMesh> mesh = nullptr;

			try {
				mesh = BlockMesh::createFromFile( "TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw an exception" );
			}

			Assert::IsTrue( mesh->isInCpuMemory(), L"BlockMesh::isInCpuMemory() returned false" );

			std::vector<float3>& meshVertices = mesh->getVertices();
			std::vector<float3>& meshNormals = mesh->getNormals();

			std::vector<uint3>& meshTriangles = mesh->getTriangles();

			Assert::AreEqual( (int)meshTriangles.size(), 1, L"Incorrect number of triangles in mesh" );
			Assert::AreEqual( (int)meshVertices.size(), 3, L"Incorrect number of vertices in mesh" );
			Assert::AreEqual( (int)meshNormals.size(), 3, L"Incorrect number of normals in mesh" );
			Assert::AreEqual( mesh->getTexcoordsCount(), 1, L"Incorrect number of texcoord sets in mesh" );

			std::vector<float2>& meshTexcoords0 = mesh->getTexcoords( 0 );

			Assert::AreEqual( (int)meshTexcoords0.size(), 3, L"Incorrect number of texcoords in mesh" );

			Assert::IsTrue(
				MathUtil::areEqual( meshVertices.at( 0 ), float3( 0.0f, 5.0f, 0.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshVertices.at( 1 ), float3( 5.0f, 0.0f, 0.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshVertices.at( 2 ), float3( -5.0f, 0.0f, 0.0f ), 0.000001f, MathUtil::epsilonFive ),
				L"Mesh vertices are incorrect"
				);

			Assert::IsTrue(
				MathUtil::areEqual( meshNormals.at( 0 ), float3( 0.0f, 0.0f, 1.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshNormals.at( 1 ), float3( 0.0f, 0.0f, 1.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshNormals.at( 2 ), float3( 0.0f, 0.0f, 1.0f ), 0.000001f, MathUtil::epsilonFive ),
				L"Mesh normals are incorrect"
				);

			Assert::IsTrue(
				MathUtil::areEqual( meshTexcoords0.at( 0 ), float2( 0.5f, 1.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshTexcoords0.at( 1 ), float2( 1.0f, 0.0f ), 0.000001f, MathUtil::epsilonFive ) &&
				MathUtil::areEqual( meshTexcoords0.at( 2 ), float2( 0.0f, 0.0f ), 0.000001f, MathUtil::epsilonFive ),
				L"Mesh normals are incorrect"
				);

			Assert::IsTrue(
				meshTriangles.at( 0 ) == uint3( 0, 1, 2 ),
				L"Mesh triangles are incorrect"
				);
		}

		TEST_METHOD( Mesh_Loading_From_File_3 ) {

			std::shared_ptr<BlockMesh> mesh = nullptr;

			try {
				mesh = BlockMesh::createFromFile( "TestAssets/Meshes/bunny.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw an exception" );
			}

			Assert::IsTrue( mesh->isInCpuMemory(), L"BlockMesh::isInCpuMemory() returned false" );
		}

		TEST_METHOD( Mesh_Loading_From_File_4 ) {

			std::shared_ptr<BlockMesh> mesh = nullptr;

			try {
				mesh = BlockMesh::createFromFile( "TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw an exception" );
			}

			Assert::IsTrue( mesh->isInCpuMemory(), L"BlockMesh::isInCpuMemory() returned false" );
		}

		TEST_METHOD( Mesh_Loading_From_Wrong_Path_1 ) {

			std::shared_ptr<BlockMesh> mesh = nullptr;

			try {
				mesh = BlockMesh::createFromFile( "TestAssets/Meshes/XXX.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			} catch ( std::exception ex ) {
				//correct exception
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw incorrect exception" );
			}

			Assert::IsNull( mesh.get(), L"mesh is not null" );
		}

		TEST_METHOD( Mesh_Loading_Cpu_To_Gpu_1 )
		{
            std::string currentPath;
            {
                const DWORD charCount = GetCurrentDirectoryW( 0, nullptr );
                std::vector<wchar_t> currentPathBufferW;
                currentPathBufferW.resize( charCount );
                GetCurrentDirectoryW( charCount, (LPWSTR)currentPathBufferW.data() );
                std::wstring currentPathW( currentPathBufferW.data(), charCount - 1 );
            }

			std::shared_ptr<BlockMesh> mesh = nullptr;

			mesh = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );

			try {
				mesh->loadCpuToGpu( *testDevice );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::loadCpuToGpu() threw an exception" );
			}
			try {
				mesh->getNormalBuffer();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getNormalBuffer() threw an exception" );
			}
			try {
				mesh->getTexcoordBuffers();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() threw an exception" );
			}
			try {
				mesh->getTriangleBuffer();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getTriangleBuffer() threw an exception" );
			}
			try {
				mesh->getVertexBuffer();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::getVertexBuffer() threw an exception" );
			}
		}

		TEST_METHOD( Mesh_Loading_Multiple_Meshes_From_Files_1 ) {

			std::shared_ptr<BlockMesh> mesh1, mesh2, mesh3, mesh4, mesh5, mesh6, mesh7, mesh8;

			try {
				mesh1 = BlockMesh::createFromFile( "TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh2 = BlockMesh::createFromFile( "TestAssets/Meshes/bunny.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh3 = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh4 = BlockMesh::createFromFile( "TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh5 = BlockMesh::createFromFile( "TestAssets/Meshes/bunny.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh6 = BlockMesh::createFromFile( "TestAssets/Meshes/dragon.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh7 = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );
				mesh8 = BlockMesh::createFromFile( "TestAssets/Meshes/triangle.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::createFromFile() threw incorrect exception" );
			}

			Assert::IsTrue( mesh1->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh2->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh3->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh4->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh5->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh6->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh7->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
			Assert::IsTrue( mesh8->isInCpuMemory(), L"mesh.isInCpuMemory() returned false" );
		}


		TEST_METHOD( Mesh_Unloading_From_Cpu_1 ) {
			std::shared_ptr<BlockMesh> mesh = nullptr;

			mesh = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );

			try {
				mesh->unloadFromCpu();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::unloadFromCpu() threw an exception" );
			}

			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormals();
				Assert::Fail( L"BlockMesh::getNormals() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoords();
				Assert::Fail( L"BlockMesh::getTexcoords() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangles();
				Assert::Fail( L"BlockMesh::getTriangles() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertices();
				Assert::Fail( L"BlockMesh::getVertices() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}

			try {
				mesh->loadCpuToGpu( *testDevice );
				Assert::Fail( L"BlockMesh::loadCpuToGpu() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
		}

		TEST_METHOD( Mesh_Unloading_From_Cpu_Gpu_1 ) {
			std::shared_ptr<BlockMesh> mesh = nullptr;

			mesh = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			mesh->loadCpuToGpu( *testDevice );

			try {
				mesh->unloadFromCpu();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::unloadFromCpu() threw an exception" );
			}

			try {
				mesh->unloadFromGpu();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::unloadFromGpu() threw an exception" );
			}

			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormals();
				Assert::Fail( L"BlockMesh::getNormals() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoords();
				Assert::Fail( L"BlockMesh::getTexcoords() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangles();
				Assert::Fail( L"BlockMesh::getTriangles() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertices();
				Assert::Fail( L"BlockMesh::getVertices() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}

			try {
				mesh->loadCpuToGpu( *testDevice );
				Assert::Fail( L"BlockMesh::loadCpuToGpu() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
		}

		TEST_METHOD( Mesh_Unloading_From_Gpu_Cpu_1 ) {
			std::shared_ptr<BlockMesh> mesh = nullptr;

			mesh = BlockMesh::createFromFile( "TestAssets/Meshes/Pyramid.obj", BlockMeshFileInfo::Format::OBJ ).front( );
			mesh->loadCpuToGpu( *testDevice );

			try {
				mesh->unloadFromGpu();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::unloadFromGpu() threw an exception" );
			}

			try {
				mesh->unloadFromCpu();
			} catch ( ... ) {
				Assert::Fail( L"BlockMesh::unloadFromCpu() threw an exception" );
			}

			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormals();
				Assert::Fail( L"BlockMesh::getNormals() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoords();
				Assert::Fail( L"BlockMesh::getTexcoords() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangles();
				Assert::Fail( L"BlockMesh::getTriangles() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertices();
				Assert::Fail( L"BlockMesh::getVertices() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}

			try {
				mesh->loadCpuToGpu( *testDevice );
				Assert::Fail( L"BlockMesh::loadCpuToGpu() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getVertexBuffer();
				Assert::Fail( L"BlockMesh::getVertexBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getNormalBuffer();
				Assert::Fail( L"BlockMesh::getNormalBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTexcoordBuffers();
				Assert::Fail( L"BlockMesh::getTexcoordBuffers() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
			try {
				mesh->getTriangleBuffer();
				Assert::Fail( L"BlockMesh::getTriangleBuffer() - Exception not thrown on illegal operation" );
			} catch ( ... ) {}
		}

	};
}
