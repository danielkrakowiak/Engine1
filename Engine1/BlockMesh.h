#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <wrl.h>

#include "float2.h"
#include "float3.h"
#include "uint3.h"

struct ID3D11Device;
struct ID3D11Buffer;

class BlockMesh {
	friend class MyOBJFileParser;
	friend class MyDAEFileParser;

public:

	enum class FileFormat : char
	{
		OBJ,
		DAE
	};

	static std::vector< std::shared_ptr<BlockMesh> > createFromFile( const std::string& path, const FileFormat format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
	static std::vector< std::shared_ptr<BlockMesh> > createFromMemory( std::vector<char>& fileData, const FileFormat format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );

	BlockMesh();
	~BlockMesh();

	void loadCpuToGpu( ID3D11Device& device );
	void loadGpuToCpu( );
	void unloadFromCpu( );
	void unloadFromGpu( );
	bool isInCpuMemory( ) const;
	bool isInGpuMemory( ) const;

	const std::vector<float3>& getVertices( ) const;
	std::vector<float3>& getVertices( );
	const std::vector<float3>& getNormals( ) const;
	std::vector<float3>& getNormals( );
	int getTexcoordsCount( ) const;
	const std::vector<float2>& getTexcoords( int setIndex = 0 ) const;
	std::vector<float2>& getTexcoords( int setIndex = 0 );
	const std::vector<uint3>& getTriangles( ) const;
	std::vector<uint3>& getTriangles( );

	ID3D11Buffer* getVertexBuffer() const;
	ID3D11Buffer* getNormalBuffer( ) const;
	std::list< ID3D11Buffer* > getTexcoordBuffers( ) const;
	ID3D11Buffer* getTriangleBuffer( ) const;

private:

	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::list< std::vector<float2> > texcoords;
	std::vector<uint3> triangles;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> normalBuffer;
	std::list< Microsoft::WRL::ComPtr<ID3D11Buffer> > texcoordBuffers;
	Microsoft::WRL::ComPtr<ID3D11Buffer> triangleBuffer;

	// Copying meshes in not allowed.
	BlockMesh( const BlockMesh& ) = delete;
	BlockMesh& operator=( const BlockMesh& ) = delete;
};