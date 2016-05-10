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

    class ShadingComputeShader : public ComputeShader
    {

        public:

        ShadingComputeShader();
        virtual ~ShadingComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, 
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture, 
                            const std::vector< std::shared_ptr< Light > >& lights );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        /*__declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
        };*/

        // Copying is not allowed.
        ShadingComputeShader( const ShadingComputeShader& )          = delete;
        ShadingComputeShader& operator=(const ShadingComputeShader&) = delete;
    };
}

