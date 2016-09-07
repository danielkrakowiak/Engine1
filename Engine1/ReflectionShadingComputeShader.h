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
    class Light;

    class ReflectionShadingComputeShader : public ComputeShader
    {

        public:

        ReflectionShadingComputeShader();
        virtual ~ReflectionShadingComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );

        void setParameters( ID3D11DeviceContext& deviceContext, const float3& cameraPos,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
        };

        // Copying is not allowed.
        ReflectionShadingComputeShader( const ReflectionShadingComputeShader& )          = delete;
        ReflectionShadingComputeShader& operator=(const ReflectionShadingComputeShader&) = delete;
    };
}

