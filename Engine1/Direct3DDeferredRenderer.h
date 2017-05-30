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

        struct RenderTargets
        {
            RenderTargets() :
                position( nullptr ),
                emissive( nullptr ),
                albedo( nullptr ),
                metalness( nullptr ),
                roughness( nullptr ),
                normal( nullptr ),
                refractiveIndex( nullptr ),
                depth( nullptr )
            {}

            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        position;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        emissive;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > >        albedo;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > metalness;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > roughness;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > >        normal;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > refractiveIndex;
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, uchar4 > >        depth;
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

        Direct3DDeferredRenderer( Direct3DRendererCore& rendererCore );
        ~Direct3DDeferredRenderer();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device, 
                         Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void disableRenderTargets();

        void render( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix 
        );

        void renderEmissive( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix 
        );

        void render( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const SkeletonMesh& mesh, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const SkeletonPose& poseInSkeletonSpace 
        );

        void render( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const BlockModel& model, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const float4& extraEmissive = float4::ZERO 
        );

        void render( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const SkeletonModel& model, 
            const float43& worldMatrix, 
            const float44& viewMatrix, 
            const SkeletonPose& poseInSkeletonSpace, 
            const float4& extraEmissive = float4::ZERO 
        );

        // #TODO: Should be moved to a separate renderer.
        void render( 
            const RenderTargets& renderTargets, 
            const Settings& renderSettings, 
            const std::string& text, 
            Font& font, 
            float2 position, 
            float4 color 
        );

        private:

        Direct3DRendererCore& m_rendererCore;

        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

        bool m_initialized;

        // Rasterizer states.
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_wireframeRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForMeshRendering;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForTransparentMeshRendering;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendStateForTextRendering;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>   createWireframeRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthStencilState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForMeshRendering( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTransparentMeshRendering( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>        createBlendStateForTextRendering( ID3D11Device& device );

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

        void enableRenderTargets( const RenderTargets& renderTargets );

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
        Direct3DDeferredRenderer( const Direct3DDeferredRenderer& ) = delete;
        Direct3DDeferredRenderer& operator=(const Direct3DDeferredRenderer&) = delete;
    };
}

