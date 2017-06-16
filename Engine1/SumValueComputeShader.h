#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class SumValueComputeShader : public ComputeShader
    {

        public:

        SumValueComputeShader();
        virtual ~SumValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::ShaderResource, float >& texture1 );

        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::ShaderResource, float >& texture1,
                            Texture2DSpecBind< TexBind::ShaderResource, float >& texture2 );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        // Copying is not allowed.
        SumValueComputeShader( const SumValueComputeShader& ) = delete;
        SumValueComputeShader& operator=( const SumValueComputeShader& ) = delete;
    };
}







