#pragma once

#include <wrl.h>
#include <memory>

#include "Texture2DTypes.h"
#include "RectangleMesh.h"

#include "CombiningVertexShader.h"
#include "CombiningFragmentShader.h"
#include "CombiningFragmentShader2.h"

#include "uchar4.h"
#include "float2.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
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

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        // TODO: alpha should be replaced by "alpha texture".
        void combine( 
            std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture,
            const std::shared_ptr< Texture2D< float4 > > srcTexture,
            const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture,
            const std::shared_ptr< Texture2D< float4 > > normalTexture,
            const std::shared_ptr< Texture2D< float4 > > positionTexture,
            const std::shared_ptr< Texture2D< uchar4 > > depthTexture,
            const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,
            const float3 cameraPosition,
            const int contributionTextureFilledWidth, 
            const int contributionTextureFilledHeight,
            const int srcTextureFilledWidth, 
            const int srcTextureFilledHeight 
        );

        void combine( 
            std::shared_ptr< RenderTargetTexture2D< float4 > > destTexture,
            const std::shared_ptr< Texture2D< float4 > > srcTexture,
            const std::shared_ptr< Texture2D< uchar4 > > contributionTermTexture,
            const std::shared_ptr< Texture2D< float4 > > previousHitNormalTexture,
            const std::shared_ptr< Texture2D< float4 > > previousHitPositionTexture,
            const std::shared_ptr< Texture2D< float > >  previousHitDistanceTexture,
            const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,
            const std::shared_ptr< Texture2D< float4 > > previousRayOriginTexture,
            const int contributionTextureFilledWidth, 
            const int contributionTextureFilledHeight,
            const int srcTextureFilledWidth, 
            const int srcTextureFilledHeight 
        );

        void  setNormalThreshold( float threshold );
        float getNormalThreshold() const;
        void  setPositionThreshold( float threshold );
        float getPositionThreshold() const;

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_initialized;

        Microsoft::WRL::ComPtr< ID3D11RasterizerState > createRasterizerState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr< ID3D11BlendState >      createBlendState( ID3D11Device3& device );

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

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        CombiningRenderer( const CombiningRenderer& )           = delete;
        CombiningRenderer& operator=(const CombiningRenderer& ) = delete;
    };
}

