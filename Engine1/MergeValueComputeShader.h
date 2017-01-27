#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class MergeValueComputeShader : public ComputeShader
    {

        public:

        MergeValueComputeShader();
        virtual ~MergeValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            Texture2DSpecBind< TexBind::ShaderResource, float > texture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        /*__declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
            struct ConstantBuffer
        {
        };*/

        // Copying is not allowed.
        MergeValueComputeShader( const MergeValueComputeShader& ) = delete;
        MergeValueComputeShader& operator=( const MergeValueComputeShader& ) = delete;
    };
}



