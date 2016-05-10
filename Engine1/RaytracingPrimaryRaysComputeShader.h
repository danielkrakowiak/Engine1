#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "TTexture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class BlockMesh;

    class RaytracingPrimaryRaysComputeShader : public ComputeShader
    {

        public:

        RaytracingPrimaryRaysComputeShader();
        virtual ~RaytracingPrimaryRaysComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const float3 rayOrigin, 
                            const Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 >& rayDirectionsTexture, 
                            const BlockMesh& mesh, const float43& worldMatrix, const float3 boundingBoxMin, const float3 boundingBoxMax,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture );
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

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

        // Copying is not allowed.
        RaytracingPrimaryRaysComputeShader( const RaytracingPrimaryRaysComputeShader& ) = delete;
        RaytracingPrimaryRaysComputeShader& operator=(const RaytracingPrimaryRaysComputeShader&) = delete;
    };
}

