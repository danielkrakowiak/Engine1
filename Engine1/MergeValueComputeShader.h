#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class MergeValueComputeShader : public ComputeShader
    {

        public:

        MergeValueComputeShader();
        virtual ~MergeValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            Texture2D< float >& texture );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

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



