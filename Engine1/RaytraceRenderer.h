#pragma once

#include <memory>
#include <wrl.h>

#include "uchar4.h"
#include "float2.h"
#include "float4.h"
#include "int2.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

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
    template < typename PixelType >
    class SumValuesComputeShader;

    class RaytraceRenderer
    {
        public:

        // For first reflected/refracted rays.
        struct InputTextures1
        {
            InputTextures1() :
                prevHitPosition( nullptr ),
                prevHitNormal( nullptr ),
                prevHitRoughness( nullptr ),
                prevHitRefractiveIndex( nullptr ),
                contribution( nullptr ),
                prevCurrentRefractiveIndex( nullptr ),
                prevPrevCurrentRefractiveIndex( nullptr )
            {}

            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >        prevHitPosition;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > >        prevHitNormal;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevHitRoughness;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevHitRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >        contribution;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevCurrentRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevPrevCurrentRefractiveIndex;
        };

        // For other layers of reflected/refracted rays.
        struct InputTextures2 : public InputTextures1
        {
            InputTextures2() :
                InputTextures1(),
                prevRayDirection( nullptr ),
                prevHitDistanceToCamera( nullptr )
            {}

            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > prevRayDirection;
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  prevHitDistanceToCamera;
        };

        struct RenderTargets
        {
            RenderTargets() :
                rayOrigin( nullptr ),
                rayDirection( nullptr ),
                hitPosition( nullptr ),
                hitEmissive( nullptr ),
                hitAlbedo( nullptr ),
                hitMetalness( nullptr ),
                hitRoughness( nullptr ),
                hitNormal( nullptr ),
                hitRefractiveIndex( nullptr ),
                currentRefractiveIndex( nullptr ),
                hitDistance( nullptr ),
                hitDistanceToCamera( nullptr )
            {}

            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        rayOrigin;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        rayDirection;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        hitPosition;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        hitEmissive;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        hitAlbedo;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitMetalness;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitRoughness;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        hitNormal;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > hitRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > currentRefractiveIndex;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         hitDistance;
            std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > >         hitDistanceToCamera;
        };

        RaytraceRenderer( Direct3DRendererCore& rendererCore );
        ~RaytraceRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void generateAndTracePrimaryRays( 
            const Camera& camera, 
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors );

        void generateAndTraceFirstReflectedRays( 
            const Camera& camera, 
            InputTextures1& inputTextures,
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors 
        );

        void generateAndTraceFirstRefractedRays( 
            const Camera& camera, 
            InputTextures1& inputTextures,
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors 
        );

        void generateAndTraceReflectedRays( 
            InputTextures2& inputTextures,
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors );

        void generateAndTraceRefractedRays( 
            const int refractionLevel,
            InputTextures2& inputTextures,
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors );

        private:

        void generatePrimaryRays( 
            const Camera& camera,
            RenderTargets& renderTargets
        );

        void generateFirstReflectedRays( 
            const Camera& camera, 
            InputTextures1& inputTextures,
            RenderTargets& renderTargets
        );

        void generateFirstRefractedRays( 
            const Camera& camera, 
            InputTextures1& inputTextures,
            RenderTargets& renderTargets 
        );

        void generateReflectedRays( 
            InputTextures2& inputTextures,
            RenderTargets& renderTargets 
        );

        void generateRefractedRays( 
            const int refractionLevel,
            InputTextures2& inputTextures,
            RenderTargets& renderTargets 
        );

        void tracePrimaryRays( 
            const Camera& camera, 
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors 
        );

        void traceSecondaryRays( 
            RenderTargets& renderTargets,
            const std::vector< std::shared_ptr< BlockActor > >& actors 
        );

        void calculateHitDistanceToCamera( InputTextures2& inputs, RenderTargets& renderTargets );

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device3>        m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_deviceContext;

        bool m_initialized;

        // Render targets.
        int m_imageWidth, m_imageHeight;

        // Shaders.
        std::shared_ptr< GenerateRaysComputeShader >               m_generateRaysComputeShader;
        std::shared_ptr< GenerateFirstReflectedRaysComputeShader > m_generateFirstReflectedRaysComputeShader;
        std::shared_ptr< GenerateFirstRefractedRaysComputeShader > m_generateFirstRefractedRaysComputeShader;
        std::shared_ptr< GenerateReflectedRaysComputeShader >      m_generateReflectedRaysComputeShader;
        std::shared_ptr< GenerateRefractedRaysComputeShader >      m_generateRefractedRaysComputeShader;
        std::shared_ptr< RaytracingPrimaryRaysComputeShader >      m_raytracingPrimaryRaysComputeShader;
        std::shared_ptr< RaytracingSecondaryRaysComputeShader >    m_raytracingSecondaryRaysComputeShader;
        std::shared_ptr< SumValuesComputeShader< float > >         m_sumValueComputeShader;

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        RaytraceRenderer( const RaytraceRenderer& ) = delete;
        RaytraceRenderer& operator=(const RaytraceRenderer&) = delete;
    };
}

