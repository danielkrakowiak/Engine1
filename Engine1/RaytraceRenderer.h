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
    class GenerateFirstReflectedRaysComputeShader;
    class GenerateFirstRefractedRaysComputeShader;
    class GenerateReflectedRaysComputeShader;
    class GenerateRefractedRaysComputeShader;
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

        void generateAndTraceFirstReflectedRays( const Camera& camera, 
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceFirstRefractedRays( const Camera& camera, 
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceReflectedRays( const int level,
                                            /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,*/
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceRefractedRays( const int level,
                                            /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,*/
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayOriginsTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayDirectionsTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayHitPositionTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > >         getRayHitDistanceTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >        getRayHitEmissiveTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >        getRayHitAlbedoTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitMetalnessTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitRoughnessTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >        getRayHitNormalTexture( int level );
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > getRayHitIndexOfRefractionTexture( int level );

        private:

        void generatePrimaryRays( const Camera& camera );

        void generateFirstReflectedRays( const Camera& camera, 
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture );

        void generateFirstRefractedRays( const Camera& camera, 
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture );

        void generateReflectedRays( int level,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture );

        void generateRefractedRays( int level,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > refractionTermTexture );

        void tracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void traceSecondaryRays( int level, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device>        device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

        bool initialized;

        // Render targets.
        int imageWidth, imageHeight;

        static const int maxRenderTargetCount;

        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >        rayOriginsTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >        rayDirectionsTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >        rayHitPositionTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float > > >         rayHitDistanceTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > >        rayHitEmissiveTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > >        rayHitAlbedoTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > > rayHitMetalnessTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > > rayHitRoughnessTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >        rayHitNormalTexture;
        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > > rayHitIndexOfRefractionTexture;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< GenerateRaysComputeShader >               generateRaysComputeShader;
        std::shared_ptr< GenerateFirstReflectedRaysComputeShader > generateFirstReflectedRaysComputeShader;
        std::shared_ptr< GenerateFirstRefractedRaysComputeShader > generateFirstRefractedRaysComputeShader;
        std::shared_ptr< GenerateReflectedRaysComputeShader >      generateReflectedRaysComputeShader;
        std::shared_ptr< GenerateRefractedRaysComputeShader >      generateRefractedRaysComputeShader;
        std::shared_ptr< RaytracingPrimaryRaysComputeShader >      raytracingPrimaryRaysComputeShader;
        std::shared_ptr< RaytracingSecondaryRaysComputeShader >    raytracingSecondaryRaysComputeShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Default textures.
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultAlphaTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultMetalnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultRoughnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultIndexOfRefractionTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultEmissiveTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultAlbedoTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultNormalTexture;

        void createDefaultTextures( ID3D11Device& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

