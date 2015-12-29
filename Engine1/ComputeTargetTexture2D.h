#pragma once

#include "Texture2D.h"

#include "float4.h"

// #TODO: support for various pixel formats - uint, float etc.

struct ID3D11UnorderedAccessView;

namespace Engine1
{
    class ComputeTargetTexture2D : public Texture2D
    {
        public:

        ComputeTargetTexture2D( int width, int height, ID3D11Device& device );
        ~ComputeTargetTexture2D();

        void clearOnGpu( float4 color, ID3D11DeviceContext& deviceContext );

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void unloadFromGpu();

        bool isInGpuMemory() const;

        ID3D11UnorderedAccessView* getComputeTarget();

        protected:

        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> unorderedAccess;

        private:

        // Copying textures in not allowed.
        ComputeTargetTexture2D( const ComputeTargetTexture2D& ) = delete;
        ComputeTargetTexture2D& operator=(const ComputeTargetTexture2D&) = delete;
    };
}

