#pragma once

#include <vector>
#include <memory>
#include <wrl.h>

#include "BlockMeshVertexShader.h"
#include "BlockMeshFragmentShader.h"
#include "SkeletonMeshVertexShader.h"
#include "SkeletonMeshFragmentShader.h"
#include "BlockModelVertexShader.h"
#include "BlockModelFragmentShader.h"
#include "SkeletonModelVertexShader.h"
#include "SkeletonModelFragmentShader.h"
#include "TextVertexShader.h"
#include "TextFragmentShader.h"
#include "Texture2DTypes.h"

#include "uchar2.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class DX11RendererCore;
    class BlockMesh;
    class SkeletonMesh;
    class BlockModel;
    class SkeletonModel;
    class SkeletonPose;
    class Font;

    class DX11DeferredRenderer
    {

        public:

        struct DeferredRenderTargets
        {
            DeferredRenderTargets() :
                position( nullptr ),
                emissive( nullptr ),
                albedo( nullptr ),
                metalness( nullptr ),
                roughness( nullptr ),
                normal( nullptr ),
                refractiveIndex( nullptr ),
                depth( nullptr )
            {}

            std::shared_ptr< RenderTargetTexture2D< float4 > >        position;
            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        emissive;
            std::shared_ptr< RenderTargetTexture2D< uchar4 > >        albedo;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > metalness;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > roughness;
            std::shared_ptr< RenderTargetTexture2D< float4 > >        normal;
            std::shared_ptr< RenderTargetTexture2D< unsigned char > > refractiveIndex;
            std::shared_ptr< DepthTexture2D< uchar4 > >               depth;
        };

        struct Settings
        {
            Settings() :
                fieldOfView( 0.0f ),
                imageDimensions( 0.0f, 0.0f ),
                zNear( 0.0f ),
                zFar( 0.0f ),
                wireframeMode( false )
            {}

            float  fieldOfView;
            float2 imageDimensions;
            float  zNear;
            float  zFar;
            bool   wireframeMode;
        };

        DX11DeferredRenderer( DX11RendererCore& rendererCore );
        ~DX11DeferredRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void disableRenderTargets();

        void render( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix 
        );

        void renderEmissive( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix 
        );

        void render( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const SkeletonMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const SkeletonPose& poseInSkeletonSpace 
        );

        void render( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockModel& model, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const float4& extraEmissive = float4::ZERO 
        );

        void render( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const SkeletonModel& model, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const SkeletonPose& poseInSkeletonSpace, 
            const float4& extraEmissive = float4::ZERO 
        );

        // #TODO: Should be moved to a separate renderer.
        void render( 
            const DeferredRenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const std::string& text, 
            Font& font, 
            float2 position, 
            float4 color 
        );

        private:

        DX11RendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device3> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_deviceContext;

        bool m_initialized;

        // Rasterizer states.
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_wireframeRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForMeshRendering;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForTransparentMeshRendering;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForTextRendering;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createRasterizerState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createWireframeRasterizerState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthStencilState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForMeshRendering( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTransparentMeshRendering( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTextRendering( ID3D11Device3& device );

        // Shaders.
        BlockMeshVertexShader        m_blockMeshVertexShader;
        BlockMeshFragmentShader      m_blockMeshFragmentShader;
        BlockMeshFragmentShader      m_blockMeshEmissiveFragmentShader;
        SkeletonMeshVertexShader     m_skeletonMeshVertexShader;
        SkeletonMeshFragmentShader   m_skeletonMeshFragmentShader;
        BlockModelVertexShader	     m_blockModelVertexShader;
        BlockModelFragmentShader	 m_blockModelFragmentShader;
        SkeletonModelVertexShader    m_skeletonModelVertexShader;
        SkeletonModelFragmentShader  m_skeletonModelFragmentShader;
        TextVertexShader             m_textVertexShader;
        TextFragmentShader           m_textFragmentShader;

        void enableRenderTargets( const DeferredRenderTargets& renderTargets );

        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        // Copying is not allowed.
        DX11DeferredRenderer( const DX11DeferredRenderer& ) = delete;
        DX11DeferredRenderer& operator=(const DX11DeferredRenderer&) = delete;
    };
}

