#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class SpreadValueComputeShader : public ComputeShader
    {

        public:

        SpreadValueComputeShader();
        virtual ~SpreadValueComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const float skipPixelIfBelowValue,
                            const float minAcceptableValue,
                            const int spreadDistance,
                            const int offset );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            float  skipPixelIfBelowValue;
            float3 pad1;
            float  minAcceptableValue;
            float3 pad2;
            int    spreadDistance;
            float3 pad3;
            int    offset;
            float3 pad4;
        };

        // Copying is not allowed.
        SpreadValueComputeShader( const SpreadValueComputeShader& ) = delete;
        SpreadValueComputeShader& operator=( const SpreadValueComputeShader& ) = delete;
    };
}



