#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"

#include "TTexture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class ComputeTargetTexture2D;
    class Camera;
    class GenerateRaysComputeShader;
    class RaytracingComputeShader;
    class BlockActor;

    class RaytraceRenderer
    {
        public:

        RaytraceRenderer( Direct3DRendererCore& rendererCore );
        ~RaytraceRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void generateAndTraceRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        // For test - only temporary.
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > getRayDirectionsTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >  getRayHitDistanceTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > getRayHitAlbedoTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > > getRayHitNormalTexture();

        private:

        void disableRenderingPipeline();
        void disableComputePipeline();

        void generateRays( const Camera& camera );
        void traceRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device>        device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > rayDirectionsTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >  rayHitDistanceTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > rayHitAlbedoTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float2 > > rayHitNormalTexture;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr<GenerateRaysComputeShader> generateRaysComputeShader;
        std::shared_ptr<RaytracingComputeShader>   raytracingComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

