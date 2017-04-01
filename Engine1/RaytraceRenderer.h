#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Direct3DRendererCore;
    class ComputeTargeTexture2D;
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
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceFirstRefractedRays( const Camera& camera, 
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > refractiveIndexTexture,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceReflectedRays( const int level,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void generateAndTraceRefractedRays( const int level, const int refractionLevel,
                                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture,
                                            const std::vector< std::shared_ptr< const BlockActor > >& actors );

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >             getRayOriginsTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >             getRayDirectionsTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >             getRayHitPositionTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > getRayHitDistanceTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >             getRayHitEmissiveTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > >             getRayHitAlbedoTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >      getRayHitMetalnessTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >      getRayHitRoughnessTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > >      getRayHitRefractiveIndexTexture( int level );
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > >             getRayHitNormalTexture( int level );
        
        const std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >& getCurrentRefractiveIndexTextures();

        private:

        void generatePrimaryRays( const Camera& camera );

        void generateFirstReflectedRays( const Camera& camera, 
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture );

        void generateFirstRefractedRays( const Camera& camera, 
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > refractiveIndexTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture );

        void generateReflectedRays( int level,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture );

        void generateRefractedRays( int level, const int refractionLevel,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayDirectionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitPositionTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayHitNormalTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > rayHitRoughnessTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > refractiveIndexTexture,
                                    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > contributionTermTexture );

        void tracePrimaryRays( const Camera& camera, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        void traceSecondaryRays( int level, const std::vector< std::shared_ptr< const BlockActor > >& actors );

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device>        m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        static const int maxRenderTargetCount;

        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >             m_rayOriginsTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >             m_rayDirectionsTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >             m_rayHitPositionTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > > m_rayHitDistanceTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > >             m_rayHitEmissiveTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, uchar4 > > >             m_rayHitAlbedoTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >      m_rayHitMetalnessTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >      m_rayHitRoughnessTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >      m_rayHitRefractiveIndexTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, float4 > > >             m_rayHitNormalTexture;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::UnorderedAccess_ShaderResource, unsigned char > > >      m_currentRefractiveIndexTextures;

        void createComputeTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Shaders.
        std::shared_ptr< GenerateRaysComputeShader >               m_generateRaysComputeShader;
        std::shared_ptr< GenerateFirstReflectedRaysComputeShader > m_generateFirstReflectedRaysComputeShader;
        std::shared_ptr< GenerateFirstRefractedRaysComputeShader > m_generateFirstRefractedRaysComputeShader;
        std::shared_ptr< GenerateReflectedRaysComputeShader >      m_generateReflectedRaysComputeShader;
        std::shared_ptr< GenerateRefractedRaysComputeShader >      m_generateRefractedRaysComputeShader;
        std::shared_ptr< RaytracingPrimaryRaysComputeShader >      m_raytracingPrimaryRaysComputeShader;
        std::shared_ptr< RaytracingSecondaryRaysComputeShader >    m_raytracingSecondaryRaysComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        // Default textures.
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > m_defaultAlphaTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > m_defaultMetalnessTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > m_defaultRoughnessTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > m_defaultIndexOfRefractionTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        m_defaultEmissiveTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        m_defaultAlbedoTexture;
        std::shared_ptr< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        m_defaultNormalTexture;

        void createDefaultTextures( ID3D11Device& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

