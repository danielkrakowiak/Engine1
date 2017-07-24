#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class BlockMesh;

    class RaytracingPrimaryRaysComputeShader : public ComputeShader
    {

        public:

        RaytracingPrimaryRaysComputeShader();
        virtual ~RaytracingPrimaryRaysComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( ID3D11DeviceContext3& deviceContext, 
                            const float3 rayOrigin, 
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionsTexture, 
                            const BlockMesh& mesh, const float43& worldMatrix, 
                            const float3 boundingBoxMin, 
                            const float3 boundingBoxMax,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture, const float3& emissiveMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture, const float3& albedoMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture, const float3& normalMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture, const float metalnessMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture, const float roughnessMul,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture, const float indexOfRefractionMul );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3  rayOrigin;
            float   pad1;
            float44 localToWorldMatrix;
            float44 worldToLocalMatrix;
            float3  boundingBoxMin;
            float   pad2;
            float3  boundingBoxMax;
            float   pad3;
            float   alphaMul;
            float3  pad4;
            float3  emissiveMul;
            float   pad5;
            float3  albedoMul;
            float   pad6;
            float3  normalMul;
            float   pad7;
            float   metalnessMul;
            float3  pad8;
            float   roughnessMul;
            float3  pad9;
            float   indexOfRefractionMul;
            float3  pad10;
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        RaytracingPrimaryRaysComputeShader( const RaytracingPrimaryRaysComputeShader& ) = delete;
        RaytracingPrimaryRaysComputeShader& operator=(const RaytracingPrimaryRaysComputeShader&) = delete;
    };
}

