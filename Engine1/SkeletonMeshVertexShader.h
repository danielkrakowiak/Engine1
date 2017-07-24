#pragma once

#include "VertexShader.h"

#include <string>
#include <vector>
#include <map>

#include "float44.h"

#include "SkeletonMesh.h"
#include "SkeletonPose.h"
#include "SkeletonMeshEnums.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class SkeletonMeshVertexShader : public VertexShader
    {
        public:
        static const int maxBoneCount = 255; // Note: should be less or equal to 255 (indexed from 1)
        static const int maxBonesPerVertexCount = 4;

        SkeletonMeshVertexShader();
        virtual ~SkeletonMeshVertexShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix, const SkeletonMesh& skeletonMesh, const SkeletonPose& bonesPoseInSkeletonSpace );

        ID3D11InputLayout& getInputLauout() const;

        private:

        BonesPerVertexCount::Type bonesPerVertexCurrentConfig;

        // Input layouts for each possible number of bones per vertex.
        std::map< BonesPerVertexCount::Type, Microsoft::WRL::ComPtr<ID3D11InputLayout> > inputLayouts;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float44       world;
            float44       view;
            float44       projection;
            float44       bonesBindPose[ maxBoneCount ];
            float44       bonesBindPoseInv[ maxBoneCount ];
            float44       bonesPose[ maxBoneCount ];
            unsigned char bonesPerVertex;
            char          pad1[15];
        };

        // Copying is not allowed.
        SkeletonMeshVertexShader( const SkeletonMeshVertexShader& ) = delete;
        SkeletonMeshVertexShader& operator=(const SkeletonMeshVertexShader&) = delete;
    };
}
