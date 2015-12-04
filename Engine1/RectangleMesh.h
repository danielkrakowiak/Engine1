#pragma once

#include <vector>
#include <wrl.h>

#include "float2.h"
#include "float3.h"
#include "uint3.h"


struct ID3D11Device;
struct ID3D11Buffer;

namespace Engine1
{
    class RectangleMesh
    {

        public:

        RectangleMesh();
        ~RectangleMesh();

        void loadCpuToGpu( ID3D11Device& device );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        const std::vector<float3>& getVertices() const;
        const std::vector<float3>& getNormals() const;
        const std::vector<float2>& getTexcoords() const;
        const std::vector<uint3>& getTriangles() const;

        ID3D11Buffer* getVertexBuffer() const;
        ID3D11Buffer* getNormalBuffer() const;
        ID3D11Buffer* getTexcoordBuffer() const;
        ID3D11Buffer* getTriangleBuffer() const;

        private:

        std::vector<float3> vertices;
        std::vector<float3> normals;
        std::vector<float2> texcoords;
        std::vector<uint3>  triangles;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> normalBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> texcoordBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> triangleBuffer;

        // Copying is not allowed.
        RectangleMesh( const RectangleMesh& ) = delete;
        RectangleMesh& operator=(const RectangleMesh&) = delete;
    };
}

