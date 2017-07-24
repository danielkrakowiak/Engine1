#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class ReplaceValueComputeShader : public ComputeShader
    {

        public:

        ReplaceValueComputeShader();
        virtual ~ReplaceValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const float replaceFromValue,
                            const float replaceToValue );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float  replaceFromValue;
            float3 pad1;
            float  replaceToValue;
            float3 pad2;
        };

        // Copying is not allowed.
        ReplaceValueComputeShader( const ReplaceValueComputeShader& ) = delete;
        ReplaceValueComputeShader& operator=( const ReplaceValueComputeShader& ) = delete;
    };
}



