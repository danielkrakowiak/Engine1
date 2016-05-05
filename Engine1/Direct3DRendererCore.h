#pragma once

#include <vector>
#include <memory>

#include "TTexture2D.h"

#include "uint3.h"
#include "float2.h"
#include "float4.h"
#include "uchar4.h"

struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace Engine1
{
    class VertexShader;
    class FragmentShader;
    class ComputeShader;
    class RectangleMesh;
    class BlockMesh;
    class SkeletonMesh;
    class FontCharacter;
    class RenderTarget2D;
    class RenderTargetDepth2D;
    class ComputeTargetTexture2D;

    class Direct3DRendererCore
    {

        public:

        Direct3DRendererCore();
        ~Direct3DRendererCore();

        void initialize( ID3D11DeviceContext& deviceContext );

        void enableRenderTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >& renderTargets, 
                                      const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget );

        // #TODO: Should be refactored to take any UAVs despite of their PixelType in one vector. Impossible until Texture2D class gets refactoring.
        void enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > > unorderedAccessTargetsF1,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4 );

        // Temporary. Until refactoring is done.
        void enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4 );

        void disableRenderTargetViews();
        void disableUnorderedAccessViews();

        void enableRenderingShaders( std::shared_ptr<const VertexShader> vertexShader, std::shared_ptr<const FragmentShader> fragmentShader );
        void enableComputeShader( std::shared_ptr<const ComputeShader> computeShader );
        void disableRenderingShaders();
        void disableComputeShaders();

        void enableRasterizerState( ID3D11RasterizerState& rasterizerState );
        void enableDepthStencilState( ID3D11DepthStencilState& depthStencilState );
        void enableBlendState( ID3D11BlendState& blendState );
        void enableDefaultRasterizerState();
        void enableDefaultDepthStencilState();
        void enableDefaultBlendState();

        void draw( const RectangleMesh& mesh );
        void draw( const BlockMesh& mesh );
        void draw( const SkeletonMesh& mesh );
        void draw( const FontCharacter& character );

        void compute( uint3 threadCount );

        void disableShaderInputs();

        private:

        ID3D11DeviceContext* deviceContext;

        bool vertexOrFragmentShaderEnabled;
        bool computeShaderEnabled;

        std::weak_ptr< const VertexShader >   currentVertexShader;
        std::weak_ptr< const FragmentShader > currentFragmentShader;
        std::weak_ptr< const ComputeShader >  currentComputeShader;

        std::vector< ID3D11RenderTargetView* >    currentRenderTargetViews;
        ID3D11DepthStencilView*                   currentDepthRenderTargetView;
        std::vector< ID3D11UnorderedAccessView* > currentUnorderedAccessTargetViews;

        ID3D11RasterizerState*   currentRasterizerState;
        ID3D11DepthStencilState* currentDepthStencilState;
        ID3D11BlendState*        currentBlendState;

        void createNullShaderInputs();

        std::vector<ID3D11Buffer*>             nullVertexBuffers;
        std::vector<unsigned int>              nullVertexBuffersStrideOffset;
        std::vector<ID3D11ShaderResourceView*> nullResources;
        std::vector<ID3D11SamplerState*>       nullSamplers;

        // Copying is not allowed.
        Direct3DRendererCore( const Direct3DRendererCore& ) = delete;
        Direct3DRendererCore& operator=(const Direct3DRendererCore&) = delete;
    };
}

