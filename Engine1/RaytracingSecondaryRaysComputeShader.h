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

    class RaytracingSecondaryRaysComputeShader : public ComputeShader
    {

        public:

        RaytracingSecondaryRaysComputeShader();
        virtual ~RaytracingSecondaryRaysComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const Texture2D< float4 >& rayOriginsTexture,
                            const Texture2D< float4 >& rayDirectionsTexture, 
                            const BlockMesh& mesh, 
                            const float43& worldMatrix, 
                            const float3 boundingBoxMin, 
                            const float3 boundingBoxMax,
                            const Texture2D< unsigned char >& alphaTexture, const float alphaMul,
                            const Texture2D< uchar4 >& emissiveTexture, const float3& emissiveMul,
                            const Texture2D< uchar4 >& albedoTexture, const float3& albedoMul,
                            const Texture2D< uchar4 >& normalTexture, const float3& normalMul,
                            const Texture2D< unsigned char >& metalnessTexture, const float metalnessMul,
                            const Texture2D< unsigned char >& roughnessTexture, const float roughnessMul,
                            const Texture2D< unsigned char >& indexOfRefractionTexture, const float indexOfRefractionMul,
                            const int outputTextureWidth, const int outputTextureHeight );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float44 localToWorldMatrix;
            float44 worldToLocalMatrix;
            float3  boundingBoxMin;
            float   pad1;
            float3  boundingBoxMax;
            float   pad2;
            float2  outputTextureSize;
            float2  pad3;
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
        RaytracingSecondaryRaysComputeShader( const RaytracingSecondaryRaysComputeShader& ) = delete;
        RaytracingSecondaryRaysComputeShader& operator=(const RaytracingSecondaryRaysComputeShader&) = delete;
    };
}

