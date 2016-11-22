#pragma once

#include <vector>
#include <memory>

#include "Texture2D.h"
#include "StagingTexture2D.h"

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

    class Direct3DRendererCore
    {

        public:

        Direct3DRendererCore();
        ~Direct3DRendererCore();

        void initialize( ID3D11DeviceContext& deviceContext );

        void disableRenderingPipeline();
        void disableComputePipeline();

        void setViewport( float2 dimensions, float2 topLeft = float2::ZERO, float depthMin = 0.0f, float depthMax = 1.0f );

		void enableRenderTargets( const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget, const int mipmapLevel = 0 );

        void enableRenderTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float2 > > >& renderTargetsF2,
                                  const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, float4 > > >& renderTargetsF4,
                                  const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > >& renderTargetsU1, 
                                  const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, uchar4 > > >& renderTargetsU4, 
                                  const std::shared_ptr< Texture2DSpecBind< TexBind::DepthStencil, uchar4 > > depthRenderTarget,
                                  const int mipmapLevel = 0 );

        // #TODO: Should be refactored to take any UAVs despite of their PixelType in one vector. Impossible until Texture2D class gets refactoring.
        void enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float > > > unorderedAccessTargetsF1,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float2 > > > unorderedAccessTargetsF2,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, unsigned char > > > unorderedAccessTargetsU1,
                                           const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4,
                                           const int mipmapLevel = 0 );

        // Temporary. Until refactoring is done.
        void enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, float4 > > > unorderedAccessTargetsF4, 
                                           const int mipmapLevel = 0 );

        // Temporary. Until refactoring is done.
        void enableUnorderedAccessTargets( const std::vector< std::shared_ptr< Texture2DSpecBind< TexBind::UnorderedAccess, uchar4 > > > unorderedAccessTargetsU4, 
                                           const int mipmapLevel = 0 );

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

        void copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture );

        void copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > > srcTexture );

        void copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, unsigned char > > destTexture,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > srcTexture );

        void copyTexture( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > destTexture, const int destMipmap,
                          const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture, const int srcMipmap );

		template< typename T >
		void copyTexture( Texture2DSpecUsage< TexUsage::Default, T >& destTexture,
					      const Texture2DSpecBind< TexBind::ShaderResource, T >& srcTexture,
						  const int x = 0, const int y = 0, int width = 0, int height = 0 );

        template< typename T >
        void copyTexture( StagingTexture2D< T >& destTexture, const Texture2DGeneric< T >& srcTexture );

        template< typename T >
        void copyTexture( StagingTexture2D< T >& destTexture, const Texture2DGeneric< T >& srcTexture,
                          const int x, const int y, const int width, const int height );

        private:

        ID3D11DeviceContext* m_deviceContext;

        bool m_graphicsShaderEnabled;
        bool m_computeShaderEnabled;

        std::weak_ptr< const VertexShader >   m_currentVertexShader;
        std::weak_ptr< const FragmentShader > m_currentFragmentShader;
        std::weak_ptr< const ComputeShader >  m_currentComputeShader;

        float2 viewportDimensions;
        float2 viewportTopLeft;
        float  viewportDepthMin;
        float  viewportDepthMax;

        std::vector< ID3D11RenderTargetView* >    m_currentRenderTargetViews;
        ID3D11DepthStencilView*                   m_currentDepthRenderTargetView;
        std::vector< ID3D11UnorderedAccessView* > m_currentUnorderedAccessTargetViews;

        ID3D11RasterizerState*   m_currentRasterizerState;
        ID3D11DepthStencilState* m_currentDepthStencilState;
        ID3D11BlendState*        m_currentBlendState;

        void createNullShaderInputs();

        std::vector<ID3D11Buffer*>             m_nullVertexBuffers;
        std::vector<unsigned int>              m_nullVertexBuffersStrideOffset;
        std::vector<ID3D11ShaderResourceView*> m_nullResources;
        std::vector<ID3D11SamplerState*>       m_nullSamplers;

        // Copying is not allowed.
        Direct3DRendererCore( const Direct3DRendererCore& ) = delete;
        Direct3DRendererCore& operator=(const Direct3DRendererCore&) = delete;
    };

	template< typename T >
	void Direct3DRendererCore::copyTexture( Texture2DSpecUsage< TexUsage::Default, T >& destTexture,
											const Texture2DSpecBind< TexBind::ShaderResource, T >& srcTexture,
											const int x, const int y, int width, int height )
	{
		if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

        if ( width == 0 && height == 0 ) {
            width  = srcTexture.getWidth();
            height = srcTexture.getHeight();
        }

		if ( x < 0 || y < 0 || width < 0 || height < 0 || x + width > srcTexture.getWidth() || y + height > srcTexture.getHeight() )
			throw std::exception( "Direct3DRendererCore::copyTexture - given fragment exceeds boundaries of the source/destination texture." );

		D3D11_BOX sourceRregion;
		sourceRregion.left   = x;
		sourceRregion.right  = x + width;
		sourceRregion.top    = y;
		sourceRregion.bottom = y + height;
		sourceRregion.front  = 0;
		sourceRregion.back   = 1;

		m_deviceContext->CopySubresourceRegion( destTexture.getTextureResource().Get(), 0, x, y, 0, srcTexture.getTextureResource().Get(), 0, &sourceRregion );
	}

    template< typename T >
    void Direct3DRendererCore::copyTexture( StagingTexture2D< T >& destTexture, const Texture2DGeneric< T >& srcTexture )
    {
        if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

        m_deviceContext->CopyResource( destTexture.getTextureResource().Get(), srcTexture.getTextureResource().Get() );
    }

    template< typename T >
    void Direct3DRendererCore::copyTexture( StagingTexture2D< T >& destTexture, const Texture2DGeneric< T >& srcTexture,
                                            const int x, const int y, const int width, const int height )
    {
        if ( !m_deviceContext ) throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );

        if ( x < 0 || y < 0 || width < 0 || height < 0 || x + width > srcTexture.getWidth() || y + height > srcTexture.getHeight() )
            throw std::exception( "Direct3DRendererCore::copyTexture - given fragment exceeds boundaries of the source/destination texture." );

        D3D11_BOX sourceRregion;
        sourceRregion.left   = x;
        sourceRregion.right  = x + width;
        sourceRregion.top    = y;
        sourceRregion.bottom = y + height;
        sourceRregion.front  = 0;
        sourceRregion.back   = 1;

        m_deviceContext->CopySubresourceRegion( destTexture.getTextureResource().Get(), 0, x, y, 0, srcTexture.getTextureResource().Get(), 0, &sourceRregion );
    }
}

