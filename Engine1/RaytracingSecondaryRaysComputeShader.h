#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class BlockMesh;

    class RaytracingSecondaryRaysComputeShader : public ComputeShader
    {

        public:

        RaytracingSecondaryRaysComputeShader();
        virtual ~RaytracingSecondaryRaysComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );

        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 >& rayOriginsTexture,
                            const Texture2DSpecBind< TexBind::UnorderedAccess_ShaderResource, float4 >& rayDirectionsTexture, 
                            const BlockMesh& mesh, 
                            const float43& worldMatrix, 
                            const float3 boundingBoxMin, 
                            const float3 boundingBoxMax,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& alphaTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& emissiveTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& albedoTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& normalTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& metalnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& roughnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& indexOfRefractionTexture,
                            const int outputTextureWidth, const int outputTextureHeight );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

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
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        RaytracingSecondaryRaysComputeShader( const RaytracingSecondaryRaysComputeShader& ) = delete;
        RaytracingSecondaryRaysComputeShader& operator=(const RaytracingSecondaryRaysComputeShader&) = delete;
    };
}

