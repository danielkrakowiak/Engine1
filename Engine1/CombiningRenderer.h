#pragma once

#include <wrl.h>
#include <memory>

#include "TTexture2D.h"
#include "RectangleMesh.h"

#include "CombiningVertexShader.h"
#include "CombiningFragmentShader.h"
#include "CombiningFragmentShader2.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class ShadingComputeShader;
    class Light;
    class Camera;

    class CombiningRenderer
    {
        public:

        CombiningRenderer( Direct3DRendererCore& rendererCore );
        ~CombiningRenderer();

        void initialize( const int screenWidth, const int screenHeight,
                         Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        // TODO: alpha should be replaced by "alpha texture".
        void combine( std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture, 
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
                      const float3 cameraPosition );

        void combine( std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > reflectionTermTexture, 
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitNormalTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousHitPositionTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  previousHitDistanceTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,
                      const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > previousRayOriginTexture );

        void  setNormalThreshold( float threshold );
        float getNormalThreshold() const;
        void  setPositionThreshold( float threshold );
        float getPositionThreshold() const;

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        bool m_initialized;

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > createRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr< ID3D11BlendState >      createBlendState( ID3D11Device& device );

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > m_rasterizerState;
        Microsoft::WRL::ComPtr< ID3D11BlendState >      m_blendState;

        // For debug.
        float m_normalThreshold;   // Normals with dot higher than this threshold are treated as part of the same surface.
        float m_positionThreshold; // Positions where each component is smaller than this threshold are treated as part of the same surface.

        // Default mesh.
        RectangleMesh m_rectangleMesh;

        // Shaders.
        std::shared_ptr< CombiningVertexShader >    m_combiningVertexShader;
        std::shared_ptr< CombiningFragmentShader >  m_combiningFragmentShader;
        std::shared_ptr< CombiningFragmentShader2 > m_combiningFragmentShader2;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        CombiningRenderer( const CombiningRenderer& )           = delete;
        CombiningRenderer& operator=(const CombiningRenderer& ) = delete;
    };
}

