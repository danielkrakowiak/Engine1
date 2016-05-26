#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "TTexture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class EdgeDistanceComputeShader : public ComputeShader
    {

        public:

        EdgeDistanceComputeShader();
        virtual ~EdgeDistanceComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& distToEdgeTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        // Copying is not allowed.
        EdgeDistanceComputeShader( const EdgeDistanceComputeShader& )          = delete;
        EdgeDistanceComputeShader& operator=(const EdgeDistanceComputeShader&) = delete;
    };
}

