#pragma once

#include <memory>
#include <wrl.h>

#include "GenerateRaysComputeShader.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class ComputeTargetTexture2D;

    class RaytraceRenderer
    {
        public:

        RaytraceRenderer( Direct3DRendererCore& rendererCore );
        ~RaytraceRenderer();

        void initialize( int imageWidth, int imageHeight, ID3D11Device& device, ID3D11DeviceContext& deviceContext );

        void clearComputeTargets( float4 color, float depth );

        void generateRays();

        // For test - only temporary.
        std::shared_ptr<ComputeTargetTexture2D> getComputeTarget();

        private:

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        std::shared_ptr<ComputeTargetTexture2D> computeTarget;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr<GenerateRaysComputeShader> generateRaysComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

