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

#include "uchar2.h"

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

    class Direct3DDeferredRenderer
    {

        public:

        Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore );
        ~Direct3DDeferredRenderer();

        void initialize( int imageWidth, int imageHeight, Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void clearRenderTargets( float4 color, float depth );
        void disableRenderTargets();

        void render( const BlockMesh& mesh, const float43& worldMatrix, const float44& viewMatrix );
        void render( const SkeletonMesh& mesh, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
        void render( const BlockModel& model, const float43& worldMatrix, const float44& viewMatrix );
        void render( const SkeletonModel& model, const float43& worldMatrix, const float44& viewMatrix, const SkeletonPose& poseInSkeletonSpace );
        void render( const std::string& text, Font& font, float2 position, float4 color );

        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > >        getPositionRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, uchar4 > >        getAlbedoRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > getMetalnessRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > getRoughnessRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, float4 > >        getNormalRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget_ShaderResource, unsigned char > > getIndexOfRefractionRenderTarget();
        std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil_ShaderResource, uchar4 > >        getDepthRenderTarget();

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
        int imageWidth, imageHeight;

        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >        positionRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, uchar4 > >        albedoRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > > metalnessRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > > roughnessRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, float4 > >        normalRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::RenderTarget_ShaderResource, unsigned char > > indexOfRefractionRenderTarget;
        std::shared_ptr< TTexture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >        depthRenderTarget;

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

        // Default textures.
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultMetalnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultRoughnessTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, unsigned char > > defaultIndexOfRefractionTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultAlbedoTexture;
        std::shared_ptr< TTexture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >        defaultNormalTexture;

        void createDefaultTextures( ID3D11Device& device );

        // Copying is not allowed.
        Direct3DDeferredRenderer( const Direct3DDeferredRenderer& ) = delete;
        Direct3DDeferredRenderer& operator=(const Direct3DDeferredRenderer&) = delete;
    };
}

