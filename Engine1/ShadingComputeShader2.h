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
    class Light;

    class ShadingComputeShader2 : public ComputeShader
    {

        public:

        ShadingComputeShader2();
        virtual ~ShadingComputeShader2();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitEmissiveTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > rayHitAlbedoTexture, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitMetalnessTexture, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                            const std::vector< std::shared_ptr< Light > >& lights );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        static const unsigned int maxPointLightCount = 50;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            unsigned int pointLightCount;
            float3       pad1;
            float4       pointLightPositions[ maxPointLightCount ];
            float4       pointLightColors[ maxPointLightCount ];
        };

        // Copying is not allowed.
        ShadingComputeShader2( const ShadingComputeShader2& )          = delete;
        ShadingComputeShader2& operator=(const ShadingComputeShader2&) = delete;
    };
}

