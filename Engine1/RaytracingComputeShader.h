#pragma once

#include "ComputeShader.h"

#include <string>

#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Texture2D;
    class BlockMesh;

    class RaytracingComputeShader : public ComputeShader
    {

        public:

        RaytracingComputeShader();
        virtual ~RaytracingComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const float3 rayOrigin, const Texture2D& rayDirectionsTexture, const BlockMesh& mesh, const float43& worldMatrix, const float3 boundingBoxMin, const float3 boundingBoxMax );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3  rayOrigin;
            float   pad1;
            float44 worldMatrixInv;
            float3  boundingBoxMin;
            float   pad2;
            float3  boundingBoxMax;
            float   pad3;
        };

        // Copying is not allowed.
        RaytracingComputeShader( const RaytracingComputeShader& ) = delete;
        RaytracingComputeShader& operator=(const RaytracingComputeShader&) = delete;
    };
}

