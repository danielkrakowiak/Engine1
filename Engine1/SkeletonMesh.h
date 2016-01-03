#pragma once

#include <array>
#include <vector>
#include <list>
#include <memory>
#include <wrl.h>

#include "float3.h"
#include "float44.h"
#include "float2.h"
#include "uint3.h"

#include "Asset.h"
#include "SkeletonMeshEnums.h"
#include "SkeletonMeshFileInfo.h"

struct ID3D11Buffer;
struct ID3D11Device;

namespace Engine1
{
    class SkeletonMesh : public Asset
    {
        friend class MyOBJFileParser;
        friend class MyDAEFileParser;

        public:

        static std::shared_ptr<SkeletonMesh>                createFromFile( const SkeletonMeshFileInfo& fileInfo );
        static std::shared_ptr<SkeletonMesh>                createFromFile( const std::string& path, const SkeletonMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::vector< std::shared_ptr<SkeletonMesh> > createFromFile( const std::string& path, const SkeletonMeshFileInfo::Format format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::shared_ptr<SkeletonMesh>                createFromMemory( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const SkeletonMeshFileInfo::Format format, const int indexInFile, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );
        static std::vector< std::shared_ptr<SkeletonMesh> > createFromMemory( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt, const SkeletonMeshFileInfo::Format format, const bool invertZCoordinate = false, const bool invertVertexWindingOrder = false, const bool flipUVs = false );

        SkeletonMesh();
        ~SkeletonMesh();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                        setFileInfo( const SkeletonMeshFileInfo& fileInfo );
        const SkeletonMeshFileInfo& getFileInfo() const;
        SkeletonMeshFileInfo&       getFileInfo();

        void loadCpuToGpu( ID3D11Device& device, bool reload = false );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        const std::vector<float3>&        getVertices() const;
        std::vector<float3>&              getVertices();
        const std::vector<float>&         getVertexWeights() const;
        std::vector<float>&               getVertexWeights();
        const std::vector<unsigned char>& getVertexBones() const;
        std::vector<unsigned char>&       getVertexBones();
        const std::vector<float3>&        getNormals() const;
        std::vector<float3>&              getNormals();
        int                               getTexcoordsCount() const;
        const std::vector<float2>&        getTexcoords( int setIndex = 0 ) const;
        std::vector<float2>&              getTexcoords( int setIndex = 0 );
        const std::vector<uint3>&         getTriangles() const;
        std::vector<uint3>&               getTriangles();

        ID3D11Buffer*              getVertexBuffer() const;
        ID3D11Buffer*              getVertexWeightsBuffer() const;
        ID3D11Buffer*              getVertexBonesBuffer() const;
        ID3D11Buffer*              getNormalBuffer() const;
        std::list< ID3D11Buffer* > getTexcoordBuffers() const;
        ID3D11Buffer*              getTriangleBuffer() const;

        // Attaches vertex to a bone. In each vertex data, bones are ordered in weight descending order. 
        // Throws exception if the vertex cannot be attached, because it is attached to the maximum number of bones already.
        void attachVertexToBone( int vertexIndex, unsigned char boneIndex, float weight );

        // Attaches vertex to a bone. In each vertex data, bones are ordered in weight descending order. 
        // If there is more bones than slots than the bone with the smallest weight is ignored. Returns true if vertex has been attached.
        bool attachVertexToBoneIfPossible( int vertexIndex, unsigned char boneIndex, float weight );

        void normalizeVertexWeights();

        class Bone
        {
            friend class SkeletonMesh;
            //friend class std::vector< Bone >; //so it could create objects of this class

            public:
            Bone();
            Bone( const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose );
            Bone( const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose, const float43& bindPoseInv );

            std::string  getName() const { return name; }
            unsigned char getParentBoneIndex() const { return parentBoneIndex; }
            float43  getBindPose() const { return bindPose; }
            float43  getBindPoseInv() const { return bindPoseInv; }

            private:
            std::string name;
            unsigned char parentBoneIndex;
            float43 bindPose;
            float43 bindPoseInv;
        };

        unsigned char             getBoneCount() const;
        BonesPerVertexCount::Type getBonesPerVertexCount() const;

        void addOrModifyBone( const unsigned char boneIndex, const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose );
        void addOrModifyBone( const unsigned char boneIndex, const std::string& name, const unsigned char parentBoneIndex, const float43& bindPose, const float43& bindPoseInv );

        // Returns bone index for a given bone name. Throws exception if there is no such bone.
        unsigned char getBoneIndex( const std::string& name ) const;

        // If bone is found - returns ( true, bone index ), otherwise returns ( false, 0 ).
        std::tuple< bool, unsigned char > findBoneIndex( const std::string& name ) const;

        const Bone& getBone( const std::string& name ) const;
        const Bone& getBone( unsigned char index ) const;

        void recalculateBoundingBox();
        // Returns <min, max> of the bounding box.
        std::tuple<float3, float3> SkeletonMesh::getBoundingBox() const;

        private:

        SkeletonMeshFileInfo fileInfo;

        std::vector<float3>              vertices;
        std::vector< float >             vertexWeights;
        std::vector< unsigned char >     vertexBones; // Index starts from 1, 0 means no bone assigned.
        std::vector<float3>              normals;
        std::list< std::vector<float2> > texcoords;
        std::vector<uint3>               triangles;

        // To how many bones at maximum can one vertex be attached in this mesh.
        BonesPerVertexCount::Type bonesPerVertexCount;

        Microsoft::WRL::ComPtr<ID3D11Buffer>              vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>              vertexWeightsBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>              vertexBonesBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>              normalBuffer;
        std::list< Microsoft::WRL::ComPtr<ID3D11Buffer> > texcoordBuffers;
        Microsoft::WRL::ComPtr<ID3D11Buffer>              triangleBuffer;

        std::vector< Bone > bones;

        float3 boundingBoxMin;
        float3 boundingBoxMax;

        // Copying mesh in not allowed.
        SkeletonMesh( const SkeletonMesh& ) = delete;
        SkeletonMesh& operator=(const SkeletonMesh&) = delete;

    };
}

