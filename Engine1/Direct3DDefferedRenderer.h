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

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class BlockMesh;
    class SkeletonMesh;
    class BlockModel;
    class SkeletonModel;
    class SkeletonPose;
    class Font;

    class Direct3DDefferedRenderer
    {

        public:

        enum class RenderTargetType : char
        {
            ALBEDO = 0,
            NORMAL = 1,
            DEBUG_VERTEX_INDEX = 2
        };

        Direct3DDefferedRenderer( Direct3DRendererCore& rendererCore );
        ~Direct3DDefferedRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void clearRenderTargets( float4 color, float depth );

        void render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix );
        void render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
        void render( const BlockModel& model, const float43& worldMatrix, const float44& viewMatrix );
        void render( const SkeletonModel& model, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
        void render( const std::string& text, Font& font, float2 position, float4 color );

        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > > getRenderTarget( RenderTargetType type );
        std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, uchar4 > > getDepthRenderTarget();

        private:

        Direct3DRendererCore& rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

        bool initialized;

        // Rasterizer states.
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStateForMeshRendering;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStateForTextRendering;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthStencilState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForMeshRendering( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTextRendering( ID3D11Device& device );

        // Render targets.
        const int RENDER_TARGETS_COUNT = 3;

        int imageWidth, imageHeight;

        std::vector< std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > > > renderTargets;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >                depthRenderTarget;

        void createRenderTargets( int imageWidth, int imageHeight, ID3D11Device& device );

        // Projection matrices.
        float44 perspectiveProjectionMatrix;
        float44 orthographicProjectionMatrix;

        // Shaders.
        std::shared_ptr<BlockMeshVertexShader>        blockMeshVertexShader;
        std::shared_ptr<BlockMeshFragmentShader>      blockMeshFragmentShader;
        std::shared_ptr<SkeletonMeshVertexShader>     skeletonMeshVertexShader;
        std::shared_ptr<SkeletonMeshFragmentShader>   skeletonMeshFragmentShader;
        std::shared_ptr<BlockModelVertexShader>	      blockModelVertexShader;
        std::shared_ptr<BlockModelFragmentShader>	  blockModelFragmentShader;
        std::shared_ptr<SkeletonModelVertexShader>    skeletonModelVertexShader;
        std::shared_ptr<SkeletonModelFragmentShader>  skeletonModelFragmentShader;
        std::shared_ptr<TextVertexShader>             textVertexShader;
        std::shared_ptr<TextFragmentShader>           textFragmentShader;

        void loadAndCompileShaders( ID3D11Device& device );

        // Copying is not allowed.
        Direct3DDefferedRenderer( const Direct3DDefferedRenderer& ) = delete;
        Direct3DDefferedRenderer& operator=(const Direct3DDefferedRenderer&) = delete;
    };
}

