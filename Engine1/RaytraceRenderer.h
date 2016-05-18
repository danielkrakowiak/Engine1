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
    class GenerateReflectedRefractedRaysComputeShader;
    class RaytracingPrimaryRaysComputeShader;
    class RaytracingSecondaryRaysComputeShader;
    class BlockActor;

    class RaytraceRenderer
    {
        public:

        RaytraceRenderer( Direct3DRendererCore& rendererCore );
        ~RaytraceRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void generateAndTracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );
        void generateAndTraceSecondaryRays( const Camera& camera, 
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayOriginsTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayDirectionsTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayHitPositionTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >         getRayHitDistanceTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >        getRayHitAlbedoTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitMetalnessTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitRoughnessTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayHitNormalTexture();
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitIndexOfRefractionTexture();

        private:

        void generatePrimaryRays( const Camera& camera );
        void generateSecondaryRays( const Camera& camera, 
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture );

        void tracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );
        void traceSecondaryRays( const std::vector< std::shared_ptr< const BlockActor > >& actors );

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device>        device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        rayOriginsTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        rayDirectionsTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        rayHitPositionTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >         rayHitDistanceTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >        rayHitAlbedoTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > rayHitMetalnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > rayHitRoughnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        rayHitNormalTexture;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > rayHitIndexOfRefractionTexture;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< GenerateRaysComputeShader >                   generateRaysComputeShader;
        std::shared_ptr< GenerateReflectedRefractedRaysComputeShader > generateReflectedRefractedRaysComputeShader;
        std::shared_ptr< RaytracingPrimaryRaysComputeShader >          raytracingPrimaryRaysComputeShader;
        std::shared_ptr< RaytracingSecondaryRaysComputeShader >        raytracingSecondaryRaysComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Default textures.
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultMetalnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultRoughnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultIndexOfRefractionTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultAlbedoTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultNormalTexture;

        void createDefaultTextures( ID3D11Device& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

