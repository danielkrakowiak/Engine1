#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <wrl.h>

#include "float2.h"
#include "float3.h"
#include "uint3.h"

#include "Asset.h"
#include "BlockMeshFileInfo.h"

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace Engine1
{
    class BVHTree;
    class BVHTreeBuffer;

    class BlockMesh : public Asset
    {
        friend class MyOBJFileParser;
        friend class MyDAEFileParser;

        public:

        static std::shared_ptr<BlockMesh>                createFromFile( const BlockMeshFileInfo& fileInfo );
        static std::shared_ptr<BlockMesh>                createFromFile( const std::string& path, const BlockMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::vector< std::shared_ptr<BlockMesh> > createFromFile( const std::string& path, const BlockMeshFileInfo::Format format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::shared_ptr<BlockMesh>                createFromMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const BlockMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::vector< std::shared_ptr<BlockMesh> > createFromMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const BlockMeshFileInfo::Format format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );

        BlockMesh();
        ~BlockMesh();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                     setFileInfo( const BlockMeshFileInfo& fileInfo );
        const BlockMeshFileInfo& getFileInfo() const;
        BlockMeshFileInfo&       getFileInfo();

        void loadCpuToGpu( ID3D11Device& device, bool reload = false );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        const std::vector<float3>& getVertices() const;
        std::vector<float3>& getVertices();
        const std::vector<float3>& getNormals() const;
        std::vector<float3>& getNormals();
        const std::vector<float3>& getTangents() const;
        std::vector<float3>& getTangents();

        int getTexcoordsCount() const;
        const std::vector<float2>& getTexcoords( int setIndex = 0 ) const;
        std::vector<float2>& getTexcoords( int setIndex = 0 );
        const std::vector<uint3>& getTriangles() const;
        std::vector<uint3>& getTriangles();

        ID3D11Buffer* getVertexBuffer() const;
        ID3D11Buffer* getNormalBuffer() const;
        ID3D11Buffer* getTangentBuffer() const;
        std::list< ID3D11Buffer* > getTexcoordBuffers() const;
        ID3D11Buffer* getTriangleBuffer() const;

        ID3D11ShaderResourceView* getVertexBufferResource() const;
        ID3D11ShaderResourceView* getNormalBufferResource() const;
        ID3D11ShaderResourceView* getTangentBufferResource() const;
        std::list< ID3D11ShaderResourceView* > getTexcoordBufferResources() const;
        ID3D11ShaderResourceView* getTriangleBufferResource() const;

        void recalculateBoundingBox();
        // Returns <min, max> of the bounding box.
        std::tuple<float3, float3> getBoundingBox() const;

        void                             buildBvhTree();
        void                             loadBvhTreeToGpu( ID3D11Device& device );
        std::shared_ptr< const BVHTree > getBvhTree() const;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getBvhTreeBufferNodesShaderResourceView()        const;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getBvhTreeBufferNodesExtentsShaderResourceView() const;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getBvhTreeBufferTrianglesShaderResourceView()    const;

        private:

        BlockMeshFileInfo fileInfo;

        std::vector<float3> vertices;
        std::vector<float3> normals;
        std::vector<float3> tangents;
        std::list< std::vector<float2> > texcoords;
        std::vector<uint3> triangles;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> normalBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> tangentBuffer;
        std::list< Microsoft::WRL::ComPtr<ID3D11Buffer> > texcoordBuffers;
        Microsoft::WRL::ComPtr<ID3D11Buffer> triangleBuffer;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vertexBufferResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalBufferResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tangentBufferResource;
        std::list< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > texcoordBufferResources;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> triangleBufferResource;

        float3 boundingBoxMin;
        float3 boundingBoxMax;

        std::shared_ptr< BVHTree >                       bvhTree;
        std::shared_ptr< BVHTreeBuffer >                 bvhTreeBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             bvhTreeBufferNodesGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bvhTreeBufferNodesGpuSRV;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             bvhTreeBufferNodesExtentsGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bvhTreeBufferNodesExtentsGpuSRV;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             bvhTreeBufferTrianglesGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bvhTreeBufferTrianglesGpuSRV;

        // Copying meshes in not allowed.
        BlockMesh( const BlockMesh& ) = delete;
        BlockMesh& operator=(const BlockMesh&) = delete;
    };
}