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
        friend class MeshFileParser;

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

        BlockMeshFileInfo m_fileInfo;

        std::vector<float3> m_vertices;
        std::vector<float3> m_normals;
        std::vector<float3> m_tangents;
        std::list< std::vector<float2> > m_texcoords;
        std::vector<uint3> m_triangles;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_normalBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_tangentBuffer;
        std::list< Microsoft::WRL::ComPtr<ID3D11Buffer> > m_texcoordBuffers;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_triangleBuffer;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_vertexBufferResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalBufferResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tangentBufferResource;
        std::list< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > m_texcoordBufferResources;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_triangleBufferResource;

        float3 m_boundingBoxMin;
        float3 m_boundingBoxMax;

        std::shared_ptr< BVHTree >                       m_bvhTree;
        std::shared_ptr< BVHTreeBuffer >                 m_bvhTreeBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             m_bvhTreeBufferNodesGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bvhTreeBufferNodesGpuSRV;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             m_bvhTreeBufferNodesExtentsGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bvhTreeBufferNodesExtentsGpuSRV;
        Microsoft::WRL::ComPtr<ID3D11Buffer>             m_bvhTreeBufferTrianglesGpu;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bvhTreeBufferTrianglesGpuSRV;

        // Copying meshes in not allowed.
        BlockMesh( const BlockMesh& ) = delete;
        BlockMesh& operator=(const BlockMesh&) = delete;
    };
}